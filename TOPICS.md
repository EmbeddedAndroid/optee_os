# TOPICS: OP-TEE qcom/integration (public platforms)

Base: master 2cabb6d06351. This branch merges every qcom/<platform>
branch below onto the base (a branch that another one builds on comes in
through that branch's merge); each merge commit message records its
conflict resolutions and adaptations.

| platform | branch | depends on | series | upstream status |
|---|---|---|---|---|
| monaco | qcom/monaco | base | Monaco PAS (ADSP, CDSP, GP-DSP0) + Lemans PAS fixes + reset after a failed start | PR 8073 (open) |
| kodiak | qcom/kodiak | base | iris CP (PR 8124) + Kodiak ADSP/CDSP shutdown and ADSP mappings + reset after a failed start (from PR 8073) | PR 8124 (open); Kodiak commits local |
| lemans | qcom/lemans | monaco | PR 8073 + a copy of the kodiak series | as above |
| talos | qcom/talos | base | Talos support + PAS; map type and Turing restart fixes + reset after a failed start (from PR 8073) | local |
| agatti | qcom/agatti | base | Agatti PAS (ADSP, GPU zap shader, venus) | local |
| lanai | qcom/lanai | base | Lanai (SM8650) | local |

The LAVA builds used OP-TEE bd6f523e58b4, TF-A qcom/integration
f2b3d43e0 and U-Boot qcom/integration 82fbe4d77385.

## LAVA results

Build: one fresh firmware fingerprint per build (QCDFP-<date>-<n>), read
back from BL2, BL31, OP-TEE and U-Boot on the cold boot and, where the
board comes back, the second boot (Lemans: SPL instead of BL2). Suite:
the OSS suite minus ACS, compared case by case with the platform
baseline. Every board ends on stock firmware; Monaco through a separate
restore job, because its warm reboot hangs. Jobs:
https://lava.infra.foundries.io/scheduler/job/<id>.

| platform | build | job | result vs baseline |
|---|---|---|---|
| kodiak | QCDFP-261024-69 | 511916 | equal to 502713 except the big-core cpufreq runtime-ratio check, a board flake also seen with other firmware |
| agatti | QCDFP-261024-68 | 511900 | equal to 490883 |
| monaco | QCDFP-261024-67 | 511893 | equal to 502190 (gpu-tflite x2 now pass) up to the warm reboot, which hangs (SAIL acknowledge, open) |
| talos | QCDFP-261024-62 | 511901 | as 492393 except: no OSM cpufreq (not carried); kvm-unit-tests pmu-overflow-interrupt, a KVM PMU timing race that fails as often with the previous firmware (per run 2.2 % vs 3.1 %) |
| lemans | QCDFP-261024-61 | 511912 | no regression vs 509672 (cpufreq and KVM PMU now pass); U-Boot SPL not gated |
| lanai | | | untested: no LAVA board |
