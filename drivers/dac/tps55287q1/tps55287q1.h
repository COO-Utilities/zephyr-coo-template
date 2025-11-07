#ifndef ZEPHYR_DRIVERS_DAC_TI_TPS55287Q1_H_
#define ZEPHYR_DRIVERS_DAC_TI_TPS55287Q1_H_

#include <zephyr/drivers/dac.h>

#define TPS55287Q1_VREF_LSB         0x00
#define TPS55287Q1_VREF_MSB         0x01
#define TPS55287Q1_IOUT_LIMIT       0x02
#define TPS55287Q1_VOUT_SR          0x03
#define TPS55287Q1_VOUT_FS          0x04
#define TPS55287Q1_CDC              0x05
#define TPS55287Q1_MODE             0x06
#define TPS55287Q1_REG_STATUS       0x07

struct tps55287q1_config {
    struct i2c_dt_spec bus;
};

#endif  // ZEPHYR_DRIVERS_DAC_TI_TPS55287Q1_H_