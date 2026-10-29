srcs-y += subsys.c q6ss.c socinfo.c
srcs-$(CFG_QCOM_PAS_ADSP) += lpass.c
srcs-$(CFG_QCOM_PAS_CDSP) += cdsp.c
incdirs-y += .
incdirs-y += ../
