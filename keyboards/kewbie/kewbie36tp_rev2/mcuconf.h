#pragma once

#include_next "mcuconf.h"

/* 本番基板は GP18/GP19 = I2C1 を使う */
#undef RP_I2C_USE_I2C0
#define RP_I2C_USE_I2C0 FALSE
#undef RP_I2C_USE_I2C1
#define RP_I2C_USE_I2C1 TRUE
