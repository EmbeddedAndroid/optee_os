# TOPICS: OP-TEE qcom/integration (public platforms)

Base: master 3b0528e17cc6 (2026-11-05).
Every qcom/<platform> branch below is the base plus the LATEST revision
of every in-flight upstream submission for that platform (GitHub PR head)
and our not yet submitted patches. This branch merges every platform
branch onto the base; each merge commit message records its conflict
resolutions and adaptations.

Rebuilt 2026-11-05 on master after PR 8073 (Monaco DSP PAS, 01ae9da6c)
and PR 8131 (Agatti ADSP, b3d60194d) merged: qcom/monaco is the base
itself; Talos follows its PR 8213. The state before the rebuild is tag
qcom/integration-20261101.

## Platform branches

| platform | branch | on | carries | upstream status |
|---|---|---|---|---|
| monaco | qcom/monaco | base | nothing (PR 8073 merged) | merged |
| talos | qcom/talos | base | PR 8213 head e3322c0c1 (8) | open |
| kodiak | qcom/kodiak | base | PR 8124 head 3cbae7245 (10) + 3 Kodiak PAS | 8124 open; Kodiak PAS commits not submitted |
| lemans | qcom/lemans | kodiak | PR 8042 revision d816e4897 without its last commit (4) + a build warning fix + PR 8136 head b2247fdf1 (14) | open; PR 8042 head is now ad8d852fd (FIFO depth masks, QUP-core bandwidth vote over RPMh), not carried yet |
| agatti | qcom/agatti | base | PR 8124 commits 1..5 + GPU zap + venus + venus seed clean + PR 8129 head 56eece4b4 | 8131 merged; GPU/venus not submitted; 8129 open |
| shikra | qcom/shikra | base | RPM client and QUP clocks of PR 8122 head bd53e42c3 | 8107 and 8134 merged; PR 8122 head d596d0c3e sits on an RPM GLink stack that is not in master and does not apply |
| lanai | qcom/lanai | base | SM8650 | not submitted |
| kailua | qcom/kailua | lanai | SM8550 | not submitted |
| eliza | qcom/eliza | base | SM7750 on the Wildcat layer: platform, PAS clocks, CDSP, ADSP and iris PAS, CI build | not submitted |

## Per-patch submission map

Each commit of a platform branch (from its parent) and the upstream
submission it is the latest revision of. "adapted": the subject matches a
commit of that PR head but the patch differs (base or another series
changed under it).

### qcom/monaco (on the base)

Nothing carried: qcom/monaco is the base.

### qcom/talos (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| a8a2bfc89350 | plat-qcom: hoya: move the pIMEM TEE layout to the targets | Tyler Baker | PR 8213 head e3322c0c1fae |  |
| 900fe1bfdb26 | plat-qcom: hoya: move the RNG base to the targets | Tyler Baker | PR 8213 head e3322c0c1fae |  |
| 76ba422255f0 | plat-qcom: hoya: add talos support | Tyler Baker | PR 8213 head e3322c0c1fae |  |
| 1a2366b5de55 | ci: add qcom-talos build | Tyler Baker | PR 8213 head e3322c0c1fae |  |
| a5e554cbfcc7 | clk: qcom: share the QDSP6 halt and MMIO lookup helpers | Tyler Baker | PR 8213 head e3322c0c1fae |  |
| 3d99cea79320 | clk: qcom: talos: add PAS clock support | Tyler Baker | PR 8213 head e3322c0c1fae |  |
| c21060c463bc | pta: qcom: pas: talos: add ADSP and CDSP bring-up and shutdown | Tyler Baker | PR 8213 head e3322c0c1fae |  |
| 7f1c6f5bf0aa | plat-qcom: hoya: talos: enable the PAS pseudo-TA | Tyler Baker | PR 8213 head e3322c0c1fae |  |

### qcom/kodiak (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 5ddd5c53f791 | pta: qcom: pas: share the iris start and shutdown | Tyler Baker | PR 8124 head 3cbae7245472 |  |
| 2135c254f688 | core: pta: qcom: pas: Move elf related helpers to pas_elf.c | Harshal Dev | PR 8124 head 3cbae7245472 |  |
| df848fe3d32f | core: pta: qcom: pas: Set fw_base for non-relocatable images | Harshal Dev | PR 8124 head 3cbae7245472 |  |
| 970790ca29ac | pta: qcom: pas: bound the program header table in pas_elf_parse() | Tyler Baker | PR 8124 head 3cbae7245472 |  |
| d809109845e4 | pta: qcom: pas: pass image metadata to the subsystem | Tyler Baker | PR 8124 head 3cbae7245472 |  |
| db502f641a21 | ta: qcom_pas: keep one image generation across INIT_IMAGE | Tyler Baker | PR 8124 head 3cbae7245472 |  |
| 3f6cc2f695e1 | pta: qcom: pas: add video firmware seed helpers | Tyler Baker | PR 8124 head 3cbae7245472 |  |
| 84367c1f01b2 | pta: qcom: pas: iris: program the CP layout and seed | Tyler Baker | PR 8124 head 3cbae7245472 |  |
| a0bb427e7c2e | pta: qcom: pas: kodiak: program iris secure session settings | Tyler Baker | PR 8124 head 3cbae7245472 |  |
| 65656d84ef41 | pta: qcom: pas: lemans: program iris secure session settings | Tyler Baker | PR 8124 head 3cbae7245472 |  |
| 1a7cb4bfecd9 | pta: qcom: pas: kodiak: add the ADSP shutdown | Tyler Baker | not submitted |  |
| 41d36dbdae36 | pta: qcom: pas: kodiak: add the CDSP shutdown | Tyler Baker | not submitted |  |
| b994c22c17cc | pta: qcom: pas: kodiak: map the USB pages the ADSP reads | Tyler Baker | not submitted |  |

### qcom/lemans (on qcom/kodiak)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 95f0bc7dd7fe | drivers: qcom: add GENI SPI driver | Anil Veshala Veshala | PR 8042 head d816e4897567 |  |
| 29e86779b049 | drivers: qcom: spi: add lemans SPI config | Anil Veshala Veshala | PR 8042 head d816e4897567 |  |
| 4db22dce39ad | drivers: qcom: spi: add loopback SPI test | Anil Veshala Veshala | PR 8042 head d816e4897567 |  |
| 49608dc89e27 | MAINTAINERS: add Qualcomm GENI SPI driver | Anil Veshala Veshala | PR 8042 head d816e4897567 |  |
| ed8294a7b536 | core: arm: crypto: build the CE GCM code only with CFG_CRYPTO | Selvam Sathappan Periakaruppan | PR 8136 head b2247fdf14b5 |  |
| 1438b1d9d482 | plat-qcom: ipq52xx: add a low memory profile | Selvam Sathappan Periakaruppan | PR 8136 head b2247fdf14b5 |  |
| 7def3b19f2c5 | plat-qcom: support an embedded DTB | Selvam Sathappan Periakaruppan | PR 8136 head b2247fdf14b5 |  |
| 4e3d312535a5 | plat-qcom: take the DRAM from an embedded DT | Selvam Sathappan Periakaruppan | PR 8136 head b2247fdf14b5 |  |
| 9808dec3db39 | drivers: qcom: xpuv4: let an instance guard several ranges | Selvam Sathappan Periakaruppan | PR 8136 head b2247fdf14b5 |  |
| 67236c4e42e2 | plat-qcom: xpu: split out the source of the DDR guards | Selvam Sathappan Periakaruppan | PR 8136 head b2247fdf14b5 |  |
| 6f9224e3d6fe | plat-qcom: xpu: take the DDR guards from an embedded DT | Selvam Sathappan Periakaruppan | PR 8136 head b2247fdf14b5 |  |
| 0bd0d2fcb71e | plat-qcom: ipq96xx: describe the DRAM in a DT | Selvam Sathappan Periakaruppan | PR 8136 head b2247fdf14b5 |  |
| 0f51c5df0d4e | plat-qcom: lemans: describe the DRAM in a DT | Selvam Sathappan Periakaruppan | PR 8136 head b2247fdf14b5 |  |
| c919b414bedf | plat-qcom: ipq52xx: describe the DRAM in a DT | Selvam Sathappan Periakaruppan | PR 8136 head b2247fdf14b5 |  |
| e8a3fd4c88be | drivers: qcom: ramblur: split out the pIMEM configuration | Selvam Sathappan Periakaruppan | PR 8136 head b2247fdf14b5 |  |
| 0a216ba1d099 | plat-qcom: dts: add a DT binding for the Ramblur pIMEM controller | Selvam Sathappan Periakaruppan | PR 8136 head b2247fdf14b5 |  |
| 8f2f48193722 | drivers: qcom: ramblur: take pIMEM regs and vault from DT | Selvam Sathappan Periakaruppan | PR 8136 head b2247fdf14b5 |  |
| 6e2e787a1723 | MAINTAINERS: add the Qualcomm device tree files | Selvam Sathappan Periakaruppan | PR 8136 head b2247fdf14b5 | adapted |
| d6683ecdb091 | drivers: qcom: spi: fix const and declaration warnings | Tyler Baker | not submitted |  |

### qcom/agatti (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 5ddd5c53f791 | pta: qcom: pas: share the iris start and shutdown | Tyler Baker | PR 8124 head 3cbae7245472 |  |
| 2135c254f688 | core: pta: qcom: pas: Move elf related helpers to pas_elf.c | Harshal Dev | PR 8124 head 3cbae7245472 |  |
| df848fe3d32f | core: pta: qcom: pas: Set fw_base for non-relocatable images | Harshal Dev | PR 8124 head 3cbae7245472 |  |
| 970790ca29ac | pta: qcom: pas: bound the program header table in pas_elf_parse() | Tyler Baker | PR 8124 head 3cbae7245472 |  |
| d809109845e4 | pta: qcom: pas: pass image metadata to the subsystem | Tyler Baker | PR 8124 head 3cbae7245472 |  |
| a5bad9e40048 | pta: qcom: pas: agatti: add GPU zap shader support | Tyler Baker | not submitted |  |
| 9b36a57e3949 | pta: qcom: pas: agatti: add venus bring-up | Tyler Baker | not submitted |  |
| 1fd222294d92 | pta: qcom: pas: agatti: clean the venus seed to memory | Tyler Baker | not submitted |  |
| f4c9fde62d11 | drivers: gic: do not log an error for unused redistributor frames | Igor Opaniuk | PR 8129 head 56eece4b4f13 |  |

### qcom/shikra (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 7d4bded06781 | drivers: qcom: add RPM client for Shikra | Dinesh Choudhary | not submitted |  |
| 4c3630d5b2e3 | plat-qcom: shikra: add QUP clock support | Naresh Nunna | PR 8122 head d596d0c3ea53 | adapted |

### qcom/lanai (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| d66f1432808c | plat-qcom: wildcat: add Lanai (SM8650) | Tyler Baker | not submitted |  |
| 3524bd885e57 | ci: qcom: add lanai platform build | Tyler Baker | not submitted |  |

### qcom/kailua (on qcom/lanai)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| a7152cb8ef07 | plat-qcom: wildcat: add Kailua (SM8550) | Tyler Baker | not submitted |  |
| afda7bf387c5 | ci: qcom: add kailua platform build | Tyler Baker | not submitted |  |

### qcom/eliza (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 5eff446368a0 | plat-qcom: wildcat: add Eliza (SM7750) | Tyler Baker | not submitted |  |
| 4ad50cce1b13 | clk: qcom: eliza: add PAS clock support | Tyler Baker | not submitted |  |
| 6b157a62e151 | pta: qcom: pas: add the ADSP DTB PAS ID | Tyler Baker | not submitted |  |
| 1387bb55a9ce | pta: qcom: pas: eliza: add CDSP bring-up and shutdown | Tyler Baker | not submitted |  |
| 487b63834d0a | pta: qcom: pas: eliza: add ADSP bring-up and shutdown | Tyler Baker | not submitted |  |
| 9b2025c451bb | plat-qcom: wildcat: eliza: enable the PAS pseudo-TA | Tyler Baker | not submitted |  |
| 927ad70083a4 | ci: qcom: add eliza platform build | Tyler Baker | not submitted |  |
| 189d8423f3ed | pta: qcom: pas: pass image metadata to the subsystem | Tyler Baker | PR 8124 head 3cbae7245472 | adapted |
| 0b343881b41b | pta: qcom: pas: move ELF segment selection to a header | Tyler Baker | not submitted |  |
| f765934bda8e | pta: qcom: pas: add video firmware seed helpers | Tyler Baker | PR 8124 head 3cbae7245472 | adapted |
| 11a71a633d50 | pta: qcom: pas: eliza: add iris bring-up and shutdown | Tyler Baker | not submitted |  |
| 20e87e34ce38 | plat-qcom: wildcat: eliza: enable the iris PAS support | Tyler Baker | not submitted |  |
| 4e9e0d684d83 | pta: qcom: pas: eliza: map the PMIC arbiter and ADSP QUP engines | Tyler Baker | not submitted |  |

## In-flight submissions not carried

- PR 8117 (qcom_pas non-relocatable images) merged after the base
  (master 5420fe40e): PR 8124 carries its first three commits in an earlier
  revision; the two TA metadata commits are not carried. The next refresh
  takes master with PR 8124 rebased onto it.
- PR 8042 revision ad8d852fd: its FIFO depth mask fix and its QUP-core
  bandwidth vote, which needs RPMh API from another open series.
- PR 8122 head d596d0c3e and the Shikra RPM GLink/ICB/PAS stack on EA
  qcom/shikra e05bca799 (PRs 8152, 8181 and unposted work): not merged here.
- PR 8142 (plat-qcom families under arch/): tree-wide move.

## LAVA results

Gated 2026-11-05 on LAVA (public stack; only OP-TEE replaced in the
2026-11-02 gate images; low priority, stock restore verified), case for
case against the last gate baselines:

| board | job | baseline | result |
|---|---|---|---|
| VENTUNO Q (monaco) | 539915 | 530465 | equal (377 pass) |
| RB3 Gen 2 (kodiak) | 540899 | 530506 | equal (407 pass) |
| QCS615 Ride (talos) | 539892 | 512759 | equal (386 pass) |
| UNO Q (agatti) | 539893 | 530590 | equal (313 pass) |
| Lemans EVK | 539896 | control 539895 (old integration, same day) | equal except fastrpc unsigned PD, which flips run to run on the old integration as well |
