#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "MPU6050.h"

static const char *TAG = "Main";

// I2C 配置
#define I2C_MASTER_SCL_IO           0      // SCL GPIO
#define I2C_MASTER_SDA_IO           1      // SDA GPIO
#define I2C_MASTER_NUM              0      // I2C port number
#define I2C_MASTER_FREQ_HZ          400000 // I2C master clock frequency
#define I2C_MASTER_TIMEOUT_MS       1000

// MPU6050 實例
MPU6050 mpu;

// I2C 初始化
static esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {
            .clk_speed = I2C_MASTER_FREQ_HZ
        },
        .clk_flags = 0
    };
    
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) return err;
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}
    
extern "C" void app_main(void)
{
    // 初始化 I2C
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C 初始化成功");
    
    // 初始化 MPU6050
    mpu.initialize();
    ESP_LOGI(TAG, "MPU6050 初始化成功");

    // 測試連接是否成功
    if (mpu.testConnection()) {
        ESP_LOGI(TAG, "MPU6050 連接測試成功");
    } else {
        ESP_LOGE(TAG, "MPU6050 連接測試失敗");
        return;
    }
    
    int16_t ax, ay, az;
    int16_t gx, gy, gz;
    int16_t temp;

    // 主循環
    while (1)
    {
        // 讀取原始數據

}
