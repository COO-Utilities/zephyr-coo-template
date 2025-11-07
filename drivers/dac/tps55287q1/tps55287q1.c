#define DT_DRV_COMPAT ti_tps55287q1

#include <zephyr/kernel.h>
#include <zephyr/drivers/dac.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

#include "tps55287q1.h"

LOG_MODULE_REGISTER(TPS55287Q1, LOG_LEVEL_INF);

static int tps55287q1_channel_setup() {
    int ret = 0;
    return ret;
}

static int tps55287q1_init(const struct device *dev) {
    const struct tps55287q1_config *config = dev->config;
    int ret = 0;

    if(!device_is_ready(config->bus.bus)) {
        LOG_ERR("%s device not found", config->bus.bus->name);
        return -ENODEV;
    }

    LOG_INF("%s device found", config->bus.bus->name);

    return ret;
}

static int tps55287q1_write_value(const struct device *dev, uint8_t reg, uint8_t val) {
    const struct tps55287q1_config *config = (struct tps55287q1_config *)dev->config;
    int ret;

    uint8_t buf[2] = {reg, val};
    ret = i2c_write_dt(&config->bus, buf, sizeof(buf));

    return ret;
}

static DEVICE_API(dac, tps55287q1_driver_api) = {
    .channel_setup = tps55287q1_channel_setup,
    .write_value = tps55287q1_write_value,
};

#define INST_DT_TPS55287Q1(index)                                               \
    static const struct tps55287q1_config tps55287q1_config_##index = {         \
        .bus = I2C_DT_SPEC_INST_GET(index),                                     \
    };                                                                          \
                                                                                \
    DEVICE_DT_INST_DEFINE(                                                      \
        index,                                                                  \
        tps55287q1_init,                                                        \
        NULL,                                                                   \
        NULL,                                                                   \
        &tps55287q1_config_##index,                                             \
        POST_KERNEL,                                                            \
        CONFIG_TPS55287Q1_INIT_PRIORITY,                                        \
        &tps55287q1_driver_api                                                  \
    );                                                                          \

DT_INST_FOREACH_STATUS_OKAY(INST_DT_TPS55287Q1);