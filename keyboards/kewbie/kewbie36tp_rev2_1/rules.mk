# This file intentionally left blank

LTO_ENABLE = yes

POINTING_DEVICE_ENABLE = yes
# 円周スクロールのため、TPS43 のドライバは自前（tps43_driver.c）。
# QMK の azoteq_iqs5xx.c は使わない（バージョン差に左右されないように）。
POINTING_DEVICE_DRIVER = custom
I2C_DRIVER_REQUIRED = yes

SRC += kewbie36tp.c
SRC += tps43_driver.c
SRC += tp_circular.c
