#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "imu/mpu6050.h"

#define EXPECTED_I2C_DEVICES 2  // 預期的 I2C 設備數量 (MPU6050 + OLED)
static const char *TAG = "MPU6050_TEST";
// I2C 掃描函數
int i2c_scan(void) {
    ESP_LOGI(TAG, "開始掃描 I2C 設備...");
    uint8_t devices_found = 0;

    printf("\n     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    printf("00:");

    // 掃描所有可能的地址 (0x03-0x77)
    for (uint8_t i = 0; i < 128; i++) {
        if (i % 16 == 0 && i > 0) {
            printf("\n%02x:", i);
        }

        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);

        esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(50));
        i2c_cmd_link_delete(cmd);

        if (ret == ESP_OK) {
            printf(" %02x", i);
            devices_found++;
            
            // 輸出常見設備的說明
            if (i == 0x3C || i == 0x3D) {
                ESP_LOGI(TAG, "在地址 0x%02X 發現 SSD1306 OLED 顯示器", i);
            }
            if (i == 0x68 || i == 0x69) {
                ESP_LOGI(TAG, "在地址 0x%02X 發現 MPU6050 感測器", i);
            }
        } else {
            printf(" --");
        }
    }

    ESP_LOGI(TAG, "\n掃描完成! 找到 %d 個 I2C 設備\n", devices_found);
    return devices_found ; 
}
// 格式化輸出函數
static void print_sensor_data(mpu6050_data_t* data) {
    printf("\033[2J\033[H");  // 清屏並將游標移到開頭
    printf("╔══════════════════════════════════════╗\n");
    printf("║         MPU6050 Sensor Data          ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║ 加速度計 (g):                        ║\n");
    printf("║   X: %7.3f                          ║\n", data->ax);
    printf("║   Y: %7.3f                          ║\n", data->ay);
    printf("║   Z: %7.3f                          ║\n", data->az);
    printf("╠══════════════════════════════════════╣\n");
    printf("║ 陀螺儀 (deg/s):                      ║\n");
    printf("║   X: %7.3f                          ║\n", data->gx);
    printf("║   Y: %7.3f                          ║\n", data->gy);
    printf("║   Z: %7.3f                          ║\n", data->gz);
    printf("╠══════════════════════════════════════╣\n");
    printf("║ 溫度: %6.2f °C                      ║\n", data->temp);
    printf("╚══════════════════════════════════════╝\n");
}

// MPU6050 任務
static void mpu6050_task(void *pvParameters) {
    mpu6050_data_t sensor_data;
    
    while (1) {
        // 讀取感測器數據
        esp_err_t ret = mpu6050_read_data(&sensor_data);
        
        if (ret == ESP_OK) {
            // 列印數據
            print_sensor_data(&sensor_data);
        } else {
            ESP_LOGE(TAG, "Failed to read sensor data");
        }
        
        // 延遲100ms
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void app_main(void) {
    // 初始化 MPU6050
    esp_err_t ret = mpu6050_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize MPU6050");
        return;
    }
    
    ESP_LOGI(TAG, "MPU6050 initialized successfully");
    // 檢查 I2C 設備
    int devices_found = i2c_scan();
    
    if (devices_found != EXPECTED_I2C_DEVICES) {
        ESP_LOGE(TAG, "I2C 設備數量不符! 預期 %d 個，實際找到 %d 個", 
                 EXPECTED_I2C_DEVICES, devices_found);
        return;  // 如果設備數量不符，直接結束程式
    }

    ESP_LOGI(TAG, "所有 I2C 設備就緒，開始執行主程式");

    while (1)
    {
        /* code */
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    // // 創建任務
    // xTaskCreate(
    //     mpu6050_task,          // 任務函數
    //     "mpu6050_task",        // 任務名稱
    //     4096,                  // 堆疊大小
    //     NULL,                  // 任務參數
    //     5,                     // 任務優先級
    //     NULL                   // 任務句柄
    // );

    // ESP_LOGI(TAG, "MPU6050 task created");
}
