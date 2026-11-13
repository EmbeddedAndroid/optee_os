// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025, Linaro Limited.
 */

#include <atomic.h>
#include <ffa.h>
#include <initcall.h>
#include <kernel/delay.h>
#include <kernel/mutex.h>
#include <kernel/pseudo_ta.h>
#include <kernel/tee_time.h>
#include <kernel/thread.h>
#include <kernel/thread_spmc.h>
#include <sm/optee_smc.h>

/*
 * The in-thread test LSP answers as the test LSP when the request runs in
 * a thread that cannot RPC, and with x3 = 0 otherwise. With x3 =
 * TEST_LSP_CMD_HOLD_MUTEX it locks test_mutex, waits for the test PTA to
 * start waiting for it, holds it TEST_HOLD_MS longer and unlocks it; x3 =
 * 1 then means the PTA did contend. With x3 = TEST_LSP_CMD_TRY_MUTEX it
 * answers x3 = 1 (busy) if a suspended thread holds a mutex, as a caller
 * that must not hang does; otherwise 2 if it could take and release
 * test_mutex, 3 if test_mutex is held (never waiting for it).
 */
#define TEST_LSP_CMD_HOLD_MUTEX	U(0x484f4c44)
#define TEST_LSP_CMD_TRY_MUTEX	U(0x54525920)
#define TEST_HOLD_MS		U(20)
#define TEST_CONTEND_TIMEOUT_US	U(1000000)
#define TEST_HELD_TIMEOUT_US	U(5000000)

/*
 * Test PTA command: wait for the in-thread test LSP to hold test_mutex,
 * then lock it. value[0].a returns the milliseconds mutex_lock() took.
 * The wait polls without RPC: the LSP holds the mutex with foreign
 * interrupts masked, which can stall a normal world sleep meanwhile.
 */
#define TEST_PTA_CMD_CONTEND	U(0)
/*
 * Test PTA command: lock test_mutex, then sleep in the normal world on a
 * condvar, holding it, until TEST_PTA_CMD_RELEASE or TEST_PARK_TIMEOUT_MS.
 */
#define TEST_PTA_CMD_HOLD	U(1)
#define TEST_PARK_TIMEOUT_MS	U(30000)
/* Test PTA command: let the TEST_PTA_CMD_HOLD caller unlock and return */
#define TEST_PTA_CMD_RELEASE	U(2)
#define TEST_PTA_UUID { 0xcaad2041, 0x7801, 0x4e34, \
		{ 0x8e, 0xb8, 0x6a, 0x19, 0x12, 0xe2, 0x5b, 0xb1 } }

static struct mutex test_mutex __nex_data = MUTEX_INITIALIZER;
static struct mutex park_mutex __nex_data = MUTEX_INITIALIZER;
static struct condvar park_cv __nex_data = CONDVAR_INITIALIZER;
static bool park_release __nex_bss;
static unsigned int test_mutex_held __nex_bss;
static unsigned int test_mutex_contended __nex_bss;

static void test_direct_req(struct thread_smc_1_2_regs *args,
			    struct sp_session *caller_sp __unused)
{
	uint16_t src = args->a1 >> 16;
	uint16_t dst = args->a1;

	if (OPTEE_SMC_IS_64(args->a0))
		args->a0 = FFA_MSG_SEND_DIRECT_RESP_64;
	else
		args->a0 = FFA_MSG_SEND_DIRECT_RESP_32;
	args->a1 = SHIFT_U32(dst, 16) | src;
	args->a2 = 0;
	args->a3 = args->a3 + args->a4 + args->a5 + args->a6 + args->a7;
}

static uint64_t hold_test_mutex(void)
{
	uint64_t timeout = 0;
	uint64_t ret = 1;

	mutex_lock(&test_mutex);
	atomic_store_uint(&test_mutex_held, 1);

	timeout = timeout_init_us(TEST_CONTEND_TIMEOUT_US);
	while (!atomic_load_uint(&test_mutex_contended)) {
		if (timeout_elapsed(timeout)) {
			ret = 0;
			break;
		}
	}
	/* Let the PTA get into mutex_lock() */
	if (ret)
		mdelay(TEST_HOLD_MS);

	atomic_store_uint(&test_mutex_held, 0);
	mutex_unlock(&test_mutex);
	atomic_store_uint(&test_mutex_contended, 0);

	return ret;
}

static uint64_t try_test_mutex(void)
{
	if (thread_suspended_holds_mutex())
		return 1;
	if (!mutex_trylock(&test_mutex))
		return 3;
	mutex_unlock(&test_mutex);

	return 2;
}

static void test_thread_direct_req(struct thread_smc_1_2_regs *args,
				   struct sp_session *caller_sp)
{
	bool no_rpc = thread_get_id_may_fail() != THREAD_ID_INVALID &&
		      !thread_can_rpc();
	uint64_t cmd = args->a3;

	test_direct_req(args, caller_sp);
	if (!no_rpc)
		args->a3 = 0;
	else if (cmd == TEST_LSP_CMD_HOLD_MUTEX)
		args->a3 = hold_test_mutex();
	else if (cmd == TEST_LSP_CMD_TRY_MUTEX)
		args->a3 = try_test_mutex();
}

static struct spmc_lsp_desc desc __nex_data = {
	.name = "Test LSP",
	.direct_req = test_direct_req,
	.properties = FFA_PART_PROP_DIRECT_REQ_RECV,
	/* UUID 54b5440e-a3d2-48d1-872a-7b6cbfc34855 */
	.uuid_words = { 0x0e44b554, 0xd148d2a3, 0x6c7b2a87, 0x5548c3bf, },
};

static struct spmc_lsp_desc thread_desc __nex_data = {
	.name = "Test LSP in thread",
	.direct_req = test_thread_direct_req,
	.in_thread = true,
	.properties = FFA_PART_PROP_DIRECT_REQ_RECV,
	/* UUID 248fc2a0-0917-4f17-b6df-1f2896f45e80 */
	.uuid_words = { 0xa0c28f24, 0x174f1709, 0x281fdfb6, 0x805ef496, },
};

static TEE_Result lsp_init(void)
{
	TEE_Result res = spmc_register_lsp(&desc);

	if (res)
		return res;

	return spmc_register_lsp(&thread_desc);
}

nex_service_init_late(lsp_init);

static TEE_Result test_pta_contend(uint32_t param_types,
				   TEE_Param params[TEE_NUM_PARAMS])
{
	uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_OUTPUT,
					  TEE_PARAM_TYPE_NONE,
					  TEE_PARAM_TYPE_NONE,
					  TEE_PARAM_TYPE_NONE);
	uint64_t timeout = timeout_init_us(TEST_HELD_TIMEOUT_US);
	TEE_Time start = { };
	TEE_Time end = { };

	if (param_types != exp_pt)
		return TEE_ERROR_BAD_PARAMETERS;

	while (!atomic_load_uint(&test_mutex_held))
		if (timeout_elapsed(timeout))
			return TEE_ERROR_TIMEOUT;

	if (tee_time_get_sys_time(&start))
		return TEE_ERROR_GENERIC;
	atomic_store_uint(&test_mutex_contended, 1);
	mutex_lock(&test_mutex);
	mutex_unlock(&test_mutex);
	if (tee_time_get_sys_time(&end))
		return TEE_ERROR_GENERIC;

	params[0].value.a = (end.seconds - start.seconds) * 1000 +
			    end.millis - start.millis;
	params[0].value.b = 0;

	return TEE_SUCCESS;
}

static TEE_Result test_pta_hold(uint32_t param_types,
				TEE_Param params[TEE_NUM_PARAMS] __unused)
{
	TEE_Result res = TEE_SUCCESS;

	if (param_types != TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
					   TEE_PARAM_TYPE_NONE,
					   TEE_PARAM_TYPE_NONE,
					   TEE_PARAM_TYPE_NONE))
		return TEE_ERROR_BAD_PARAMETERS;

	mutex_lock(&test_mutex);
	mutex_lock(&park_mutex);
	while (!park_release) {
		res = condvar_wait_timeout(&park_cv, &park_mutex,
					   TEST_PARK_TIMEOUT_MS);
		if (res)
			break;
	}
	park_release = false;
	mutex_unlock(&park_mutex);
	mutex_unlock(&test_mutex);

	return res;
}

static TEE_Result test_pta_release(uint32_t param_types,
				   TEE_Param params[TEE_NUM_PARAMS] __unused)
{
	if (param_types != TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
					   TEE_PARAM_TYPE_NONE,
					   TEE_PARAM_TYPE_NONE,
					   TEE_PARAM_TYPE_NONE))
		return TEE_ERROR_BAD_PARAMETERS;

	mutex_lock(&park_mutex);
	park_release = true;
	condvar_signal(&park_cv);
	mutex_unlock(&park_mutex);

	return TEE_SUCCESS;
}

static TEE_Result test_pta_invoke(void *session __unused, uint32_t cmd,
				  uint32_t param_types,
				  TEE_Param params[TEE_NUM_PARAMS])
{
	if (cmd == TEST_PTA_CMD_CONTEND)
		return test_pta_contend(param_types, params);
	if (cmd == TEST_PTA_CMD_HOLD)
		return test_pta_hold(param_types, params);
	if (cmd == TEST_PTA_CMD_RELEASE)
		return test_pta_release(param_types, params);

	return TEE_ERROR_NOT_SUPPORTED;
}

pseudo_ta_register(.uuid = TEST_PTA_UUID, .name = "ffa_lsp_test.pta",
		   .flags = PTA_DEFAULT_FLAGS | TA_FLAG_CONCURRENT,
		   .invoke_command_entry_point = test_pta_invoke);
