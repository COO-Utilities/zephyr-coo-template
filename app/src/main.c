#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/dac.h>
#include <stdio.h>

LOG_MODULE_REGISTER(tps55287q1, LOG_LEVEL_INF);

int main(void) {
    LOG_INF("TPS55287-Q1 DAC");
    const struct device *const tps = DEVICE_DT_GET(DT_ALIAS(tps55287q1_rspec));

    if (!device_is_ready(tps)) {
        LOG_ERR("DAC device %s is not ready\n", tps->name);
        return 0;
    }

    int ret = dac_write_value(tps, 0x00, 1);
    if (ret != 0) {
        LOG_ERR("dac_write_value() failed with code %d\n", ret);
        return 0;
    }

    LOG_INF("End of main function");

    return 0;
}
