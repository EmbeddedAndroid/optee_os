# TOPICS: OP-TEE qcom/integration (public platforms)

Base: master 2cabb6d06351.
Every qcom/<platform> branch below is the pinned base plus
the LATEST revision of every in-flight upstream submission for that
platform (Gerrit patch set, GitHub PR head, lore series version) and our
not yet submitted patches unchanged (refresh 2026-10-25). This branch
merges every platform branch onto the base; each merge commit message
records its conflict resolutions, the ones redone in the refresh under
"Conflicts (refresh 2026-10-25)". Snapshot of the gated state before the
refresh: tag qcom/integration-20261024.

Added 2026-10-26: qcom/kailua (SM8550 HDK), merged on top of the gated
refresh; the state before it is tag qcom/integration-20261025.

Added 2026-10-31: qcom/eliza (SM7750 MTP), merged on top of the
2026-10-26 state (tag qcom/integration-20261026).

## Platform branches

| platform | branch | on | carries | upstream status |
|---|---|---|---|---|
| monaco | qcom/monaco | base | PR 8073 head 68b1903b0 (10) | open |
| kodiak | qcom/kodiak | base | PR 8124 head f145e2f59 (11, incl. PR 8117) + 3 Kodiak PAS + the 8073 failed-start fix | open; Kodiak PAS commits not submitted |
| lemans | qcom/lemans | monaco | PR 8073 + PR 8124 + Kodiak PAS copy + PR 8042 (GENI SPI, Lemans config) + PR 8136 (optional embedded DTB) | open; 8093 content already in the base |
| talos | qcom/talos | base | Talos support, PAS, map type, Turing restart, failed-start fix | not submitted (failed-start fix from PR 8073) |
| agatti | qcom/agatti | base | PR 8131 head 85cf46ea5 (Igor) + GPU zap + PR 8124 commits 1..4 + venus + venus seed clean | 8131 open; GPU/venus not submitted |
| lanai | qcom/lanai | base | SM8650 | not submitted |
| kailua | qcom/kailua | lanai | SM8550 | not submitted |
| shikra | qcom/shikra | base | PR 8107 head 64501d96e, PR 8122 head bd53e42c3, PR 8134 head 21fffcf6d | open (7926/8105 merged in the base) |
| eliza | qcom/eliza | base | SM7750 on the Wildcat layer: platform, PAS clocks, CDSP, ADSP and iris PAS, CI build | not submitted |

## Per-patch submission map

Each commit of a platform branch (from its parent: the base or the branch it builds on) and the upstream submission it is the latest revision of. "adapted": the patch differs from the posted revision because the pinned base or another series changed under it; the merge commit message or the commit itself says how. "not submitted": local work, carried unchanged.

### qcom/monaco (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| a2d7bc346b1c | clk: qcom: Get Lucid-EVO PLL offsets from platform | Taniya Das | PR 8073 head 68b1903b064d |  |
| e5328180e783 | clk: qcom: lemans: return TEE_ERROR_TIMEOUT from LPASS collapse polls | Tyler Baker | PR 8073 head 68b1903b064d |  |
| 26d2df5f8e95 | clk: qcom: lemans: reset a GP-DSP while its PDC holds it in low power | Tyler Baker | PR 8073 head 68b1903b064d |  |
| 841cb20ec6f6 | clk: qcom: hoya: share the Lemans Q6 PAS sequences | Tyler Baker | PR 8073 head 68b1903b064d |  |
| ec624733966f | clk: qcom: lemans: return the NSP clock enable error as is | Tyler Baker | PR 8073 head 68b1903b064d |  |
| 7146d510a17d | pta: qcom: pas: reset the subsystem when a start fails | Tyler Baker | PR 8073 head 68b1903b064d |  |
| 566e9371dd9d | clk: qcom: hoya: enable the QDSP6 boot branches with the NSP clocks | Tyler Baker | PR 8073 head 68b1903b064d |  |
| 4d80359d92f4 | clk: qcom: monaco: add PAS clock support | Tyler Baker | PR 8073 head 68b1903b064d |  |
| b343917cd3a7 | pta: qcom: pas: monaco: add ADSP, CDSP and GP-DSP0 bring-up | Tyler Baker | PR 8073 head 68b1903b064d |  |
| 78057e5c9fed | plat-qcom: hoya: monaco: enable the PAS pseudo-TA | Tyler Baker | PR 8073 head 68b1903b064d |  |

### qcom/kodiak (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 5fa3fa072c4b | core: pta: qcom: pas: Move elf related helpers to pas_elf.c | Harshal Dev | PR 8117 head 63630e7ecf4d |  |
| 50e42d0b43d9 | core: pta: qcom: pas: Set fw_base for non-relocatable images | Harshal Dev | PR 8117 head 63630e7ecf4d |  |
| ec088f71bc8c | pta: qcom: pas: bound the program header table in pas_elf_parse() | Tyler Baker | PR 8124 head f145e2f59118 |  |
| 83f98cc0168b | pta: qcom: pas: pass image metadata to the subsystem | Tyler Baker | PR 8124 head f145e2f59118 |  |
| 471cd2597fe2 | ta: qcom_pas: keep one image generation across INIT_IMAGE | Tyler Baker | PR 8124 head f145e2f59118 |  |
| 744141f35405 | pta: qcom: pas: add video firmware seed helpers | Tyler Baker | PR 8124 head f145e2f59118 |  |
| 9c09dc296a8f | pta: qcom: pas: define iris registers from IRIS_BASE | Tyler Baker | PR 8124 head f145e2f59118 |  |
| 2aa24b325510 | pta: qcom: pas: kodiak: program iris CP layout and seed | Tyler Baker | PR 8124 head f145e2f59118 |  |
| 9696466e016d | pta: qcom: pas: kodiak: program iris secure session settings | Tyler Baker | PR 8124 head f145e2f59118 |  |
| 8c3f3fbbe85f | pta: qcom: pas: lemans: program iris CP layout and seed | Tyler Baker | PR 8124 head f145e2f59118 |  |
| ac4bd89e8d5b | pta: qcom: pas: lemans: program iris secure session settings | Tyler Baker | PR 8124 head f145e2f59118 |  |
| 36a2a8feba61 | pta: qcom: pas: kodiak: add the ADSP shutdown | Tyler Baker | not submitted |  |
| b255a52ce496 | pta: qcom: pas: kodiak: add the CDSP shutdown | Tyler Baker | not submitted |  |
| 8d5134bbd43c | pta: qcom: pas: kodiak: map the USB pages the ADSP reads | Tyler Baker | not submitted |  |
| 1dae84adceae | pta: qcom: pas: reset the subsystem when a start fails | Tyler Baker | PR 8073 head 68b1903b064d |  |

### qcom/lemans (on qcom/monaco)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 8ea7d9e0837f | core: pta: qcom: pas: Move elf related helpers to pas_elf.c | Harshal Dev | PR 8117 head 63630e7ecf4d |  |
| ae40b9e1dded | core: pta: qcom: pas: Set fw_base for non-relocatable images | Harshal Dev | PR 8117 head 63630e7ecf4d |  |
| cc80a5100e3b | pta: qcom: pas: bound the program header table in pas_elf_parse() | Tyler Baker | PR 8124 head f145e2f59118 |  |
| 115fde748c94 | pta: qcom: pas: pass image metadata to the subsystem | Tyler Baker | PR 8124 head f145e2f59118 |  |
| 6330e076805e | ta: qcom_pas: keep one image generation across INIT_IMAGE | Tyler Baker | PR 8124 head f145e2f59118 |  |
| 8cf35a2d3adb | pta: qcom: pas: add video firmware seed helpers | Tyler Baker | PR 8124 head f145e2f59118 |  |
| d47639a97e15 | pta: qcom: pas: define iris registers from IRIS_BASE | Tyler Baker | PR 8124 head f145e2f59118 | adapted |
| 065b11dec6c6 | pta: qcom: pas: kodiak: program iris CP layout and seed | Tyler Baker | PR 8124 head f145e2f59118 |  |
| df57c2272786 | pta: qcom: pas: kodiak: program iris secure session settings | Tyler Baker | PR 8124 head f145e2f59118 |  |
| 5beb2b5cb2fb | pta: qcom: pas: lemans: program iris CP layout and seed | Tyler Baker | PR 8124 head f145e2f59118 |  |
| ac2381866f31 | pta: qcom: pas: lemans: program iris secure session settings | Tyler Baker | PR 8124 head f145e2f59118 |  |
| 824811d5f10c | pta: qcom: pas: kodiak: add the ADSP shutdown | Tyler Baker | not submitted |  |
| 9a0398cbffd4 | pta: qcom: pas: kodiak: add the CDSP shutdown | Tyler Baker | not submitted |  |
| 979d1a895f55 | pta: qcom: pas: kodiak: map the USB pages the ADSP reads | Tyler Baker | not submitted |  |
| f7e957eb0e8d | drivers: qcom: add GENI SPI driver | Anil Veshala Veshala | PR 8042 head e9622a6d1b46 |  |
| 005cb957e74c | drivers: qcom: spi: add lemans SPI config | Anil Veshala Veshala | PR 8042 head e9622a6d1b46 | adapted |
| 4f958e24ed14 | drivers: qcom: spi: add loopback SPI test | Anil Veshala Veshala | PR 8042 head e9622a6d1b46 |  |
| 9dbfbec118be | MAINTAINERS: add Qualcomm GENI SPI driver | Anil Veshala Veshala | PR 8042 head e9622a6d1b46 |  |
| 61e0f31162ef | plat-qcom: add optional embedded DTB support | Selvam Sathappan Periakaruppan | PR 8136 head 29d91ac2aead |  |
| 947657f8d58a | drivers: qcom_geni_uart: add DT console driver | Selvam Sathappan Periakaruppan | PR 8136 head 29d91ac2aead |  |
| 0a7009681f43 | plat-qcom: dts: add embedded DTBs for Lemans, IPQ52xx and IPQ96xx | Selvam Sathappan Periakaruppan | PR 8136 head 29d91ac2aead | adapted |
| 0beba847fb39 | plat-qcom: take GIC bases from DT when available | Selvam Sathappan Periakaruppan | PR 8136 head 29d91ac2aead | adapted |
| ef9007b31b29 | drivers: qcom: csrng: take register base from DT when available | Selvam Sathappan Periakaruppan | PR 8136 head 29d91ac2aead |  |
| 67b4531d230b | drivers: qcom: ramblur: take pIMEM registers and vault from DT when available | Selvam Sathappan Periakaruppan | PR 8136 head 29d91ac2aead |  |

### qcom/talos (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 3b0e16c12764 | plat-qcom: hoya: move the pIMEM TEE layout to the targets | Tyler Baker | not submitted |  |
| 4cbdf679a007 | plat-qcom: hoya: move the RNG base to the targets | Tyler Baker | not submitted |  |
| ea8d8510ec88 | plat-qcom: hoya: add talos support | Tyler Baker | not submitted |  |
| 2bb9d29ace7d | ci: add qcom-talos build | Tyler Baker | not submitted |  |
| ef551e4e447d | clk: qcom: talos: add PAS clock support | Tyler Baker | not submitted |  |
| c8cf31840b6b | pta: qcom: pas: talos: add ADSP and CDSP bring-up and shutdown | Tyler Baker | not submitted |  |
| 0b1f7848d963 | plat-qcom: hoya: talos: enable the PAS pseudo-TA | Tyler Baker | not submitted |  |
| a330e5c3e675 | pta: qcom: pas: talos: set the controller map type | Tyler Baker | not submitted |  |
| c5970650bbd7 | clk: qcom: talos: restart Turing whatever its power-on flag says | Tyler Baker | not submitted |  |
| 120a891f1f42 | pta: qcom: pas: reset the subsystem when a start fails | Tyler Baker | PR 8073 head 68b1903b064d |  |

### qcom/agatti (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| ec96e639edda | clk: qcom: agatti: add PAS clock support | Tyler Baker | PR 8131 head 85cf46ea5487 |  |
| f6b11715a620 | pta: qcom: pas: agatti: add ADSP bring-up | Tyler Baker | PR 8131 head 85cf46ea5487 | adapted |
| eac15ccbca0e | plat-qcom: bruin: agatti: enable the PAS pseudo-TA | Tyler Baker | PR 8131 head 85cf46ea5487 |  |
| 3c593694ac8a | pta: qcom: pas: agatti: add GPU zap shader support | Tyler Baker | not submitted |  |
| 78f2a47cebf6 | core: pta: qcom: pas: Move elf related helpers to pas_elf.c | Harshal Dev | PR 8117 head 63630e7ecf4d |  |
| 52ab21abdc7c | core: pta: qcom: pas: Set fw_base for non-relocatable images | Harshal Dev | PR 8117 head 63630e7ecf4d |  |
| d80ba1cab66c | pta: qcom: pas: bound the program header table in pas_elf_parse() | Tyler Baker | PR 8124 head f145e2f59118 |  |
| 798f97199a81 | pta: qcom: pas: pass image metadata to the subsystem | Tyler Baker | PR 8124 head f145e2f59118 |  |
| 2160daf2359e | pta: qcom: pas: agatti: add venus bring-up | Tyler Baker | not submitted |  |
| edaeed5f11bb | pta: qcom: pas: agatti: clean the venus seed to memory | Tyler Baker | not submitted |  |

### qcom/lanai (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 964364719862 | plat-qcom: wildcat: add Lanai (SM8650) | Tyler Baker | not submitted |  |
| 033d0c629c51 | ci: qcom: add lanai platform build | Tyler Baker | not submitted |  |

### qcom/shikra (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 9e49556cea23 | drivers: qcom: qfprom: isolate RPMH power control | Kishore Batta | PR 8107 head 64501d96ecaf |  |
| fe4f2432d6e4 | drivers: qcom: qfprom: add Shikra platform support | Kishore Batta | PR 8107 head 64501d96ecaf |  |
| cb93a72ec890 | drivers: qcom: add RPM client for Shikra | Dinesh Choudhary | PR 8122 head bd53e42c36b2 |  |
| ebbbd4a79f2e | plat-qcom: shikra: add QUP clock support | Naresh Nunna | PR 8122 head bd53e42c36b2 | adapted |
| 62a6594de998 | drivers: qcom: tlmm: enable driver for Shikra | Siva Rama Krishna Potu | PR 8134 head 21fffcf6de4a | adapted |


### qcom/kailua (on qcom/lanai)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| e34bccb9bec9 | plat-qcom: wildcat: add Kailua (SM8550) | Tyler Baker | not submitted |  |
| b39dc9544e5a | ci: qcom: add kailua platform build | Tyler Baker | not submitted |  |

### qcom/eliza (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 0e7ea3887fa6 | plat-qcom: wildcat: add Eliza (SM7750) | Tyler Baker | not submitted |  |
| 1661765f72af | clk: qcom: eliza: add PAS clock support | Tyler Baker | not submitted |  |
| f19c344f1bc0 | pta: qcom: pas: add the ADSP DTB PAS ID | Tyler Baker | not submitted |  |
| e680483289db | pta: qcom: pas: eliza: add CDSP bring-up and shutdown | Tyler Baker | not submitted |  |
| 0d16a9bc4ae0 | pta: qcom: pas: eliza: add ADSP bring-up and shutdown | Tyler Baker | not submitted |  |
| 384220270a5c | plat-qcom: wildcat: eliza: enable the PAS pseudo-TA | Tyler Baker | not submitted |  |
| f34d0b3fb15c | ci: qcom: add eliza platform build | Tyler Baker | not submitted |  |
| b34c1c3e6b56 | pta: qcom: pas: pass image metadata to the subsystem | Tyler Baker | not submitted | superseded in the merge by the integration PAS core |
| ec305ed735d6 | pta: qcom: pas: move ELF segment selection to a header | Tyler Baker | not submitted | superseded in the merge by the integration pas_elf.c |
| ed30368d6e47 | pta: qcom: pas: add video firmware seed helpers | Tyler Baker | not submitted | superseded in the merge by the integration pas_video.c |
| f777efc56c4f | pta: qcom: pas: eliza: add iris bring-up and shutdown | Tyler Baker | not submitted | adapted in the merge to the integration op signature |
| 974b70e7d27e | plat-qcom: wildcat: eliza: enable the iris PAS support | Tyler Baker | not submitted |  |

## In-flight submissions not carried

- PR 8142 (plat-qcom families under arch/): tree-wide move, not a platform submission.
- PR 8093 (draft): its change is already in the base as f51bd2b32.
- PR 7971 (draft, generic PAS anti-rollback), PR 7956 and PR 7906 (bobcat IPQ): not platforms of these branches.

## LAVA results

Build: one fresh firmware fingerprint per build (QCDFP-<date>-<n>), read
back from BL2, BL31, OP-TEE and U-Boot on the cold boot and on the second
boot where the board comes back (Lemans: SPL instead of BL2, Shikra: U-Boot
only). Suite: the OSS suite minus ACS, compared case by case with the
earlier integration gate and the platform baseline. Every job ends on the
lab stock firmware (in-job restore block, or a pinned restore job where the
job cannot reach it). Jobs: https://lava.infra.foundries.io/scheduler/job/<id>.

| platform | stack | build | job | result vs baseline |
|---|---|---|---|---|
| kodiak | public | QCDFP-261025-83 | 512685 | equal to 502713 except policy7-min fail -> pass |
| agatti | public | QCDFP-261025-86 | 512743 | equal to 511900 and 490883 |
| monaco | public | QCDFP-261025-89 | 512751 | equal to 511893; same warm reboot gap; backstop 512754 |
| lemans | public | QCDFP-261025-84 | 512687 | equal to 511912 except the same flake |
| talos | public | QCDFP-261025-92 | 512759 | equal to 511901 except the known KVM PMU timing race (now passing) and the OSM cpufreq checks (not carried, skip) |
| shikra | public | QCDFP-261025-91 | 512770 | U-Boot only (Shikra v4) on the stock XBL/QSEE/Gunyah health image: FP, Linux, login and the health smoke tests pass as health job 511579; no upstream TF-A/OP-TEE boot path yet |
| lanai | public |  |  | untested: no LAVA board |
| kailua | public |  |  | untested: no board yet (lab access pending) |
| eliza | public | QCDFP-261031-408 | MTP, no LAVA board | BOARD-REQUEST-3: TF-A/OP-TEE/U-Boot FP, U-Boot UFS clock lines gone, UFS, fastboot, GPU, NPU, CPU hotplug pass (BOARD-REQUEST-1 on 401 passed the same steps) |
