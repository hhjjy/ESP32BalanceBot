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
#define I2C_MASTER_NUM              I2C_NUM_0      // I2C port number
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
        mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        temp = mpu.getTemperature();

        // 轉換數據
        float temp_c = temp / 340.0f + 36.53f;
        float acc_x = ax / 16384.0f;  // ±2g
        float acc_y = ay / 16384.0f;
        float acc_z = az / 16384.0f;
        float gyro_x = gx / 131.0f;   // ±250°/s
        float gyro_y = gy / 131.0f;
        float gyro_z = gz / 131.0f;

        // 輸出數據
        ESP_LOGI(TAG, "=====================");
        ESP_LOGI(TAG, "加速度 (g):");
        ESP_LOGI(TAG, "  X: %.2f", acc_x);
        ESP_LOGI(TAG, "  Y: %.2f", acc_y);
        ESP_LOGI(TAG, "  Z: %.2f", acc_z);
        ESP_LOGI(TAG, "角速度 (deg/s):");
        ESP_LOGI(TAG, "  X: %.2f", gyro_x);
        ESP_LOGI(TAG, "  Y: %.2f", gyro_y);
        ESP_LOGI(TAG, "  Z: %.2f", gyro_z);
        ESP_LOGI(TAG, "溫度: %.2f °C", temp_c);

        // 延遲
        vTaskDelay(pdMS_TO_TICKS(1000));  // 每秒更新一次
    }
}
