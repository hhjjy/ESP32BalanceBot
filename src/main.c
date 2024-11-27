#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "imu/mpu6050.h"
// #include "ssd1306.h"
#include "pid/pid.h"
#include <string.h>  // 添加 string.h
#include "motor/motor.h"
#define EXPECTED_I2C_DEVICES 2  // 預期的 I2C 設備數量 (MPU6050 + OLED)
#define OLED_RESET_GPIO -1          // 重置腳位 (-1 表示不使用)
static const char *TAG = "MAIN";
#include <math.h>
#include "debug/debug.h"
// 定義數學常數
#define RAD_TO_DEG 57.295779513082320876798154814105  // 180/PI
#define DEG_TO_RAD 0.017453292519943295769236907684886  // PI/180
#define PI 3.14159265359

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




// 在全域宣告 Queue handle
static QueueHandle_t pitch_queue = NULL;

// 定義資料結構（如果需要的話）
typedef struct {
    float pitch;
    uint32_t timestamp;  // 可選，如果需要時間戳記
} pitch_data_t;

// MPU6050 任務 100HZ
static void mpu6050_task(void *pvParameters) {
    mpu6050_data_t sensor_data;
    float pitch_angle = 0;
    float prev_pitch_angle = 0;
    #define PICHOFFSET -9.50
    // 時間追蹤變數
    TickType_t last_time = xTaskGetTickCount();
    float dt;

    // 檢查 Queue 是否已創建
    if (pitch_queue == NULL) {
        ESP_LOGE(TAG, "Queue not created!");
        vTaskDelete(NULL);
        return;
    }

    while (1) {
        // 計算時間差
        TickType_t now = xTaskGetTickCount();
        dt = (float)(now - last_time) / configTICK_RATE_HZ;
        last_time = now;

        // 讀取感測器數據
        esp_err_t ret = mpu6050_read_data(&sensor_data);
        
        if (ret == ESP_OK) {
            // 計算加速度向量的大小
            float accel_magnitude = sqrtf(
                powf(sensor_data.ax, 2) + 
                powf(sensor_data.ay, 2) + 
                powf(sensor_data.az, 2)
            );

            // 檢查數據有效性
            if (accel_magnitude > 0.85f && accel_magnitude < 1.15f) {
                // 計算 pitch 角度
                float accel_pitch = atan2f(-sensor_data.ax, 
                                         sqrtf(powf(sensor_data.ay, 2) + 
                                              powf(sensor_data.az, 2))) * RAD_TO_DEG;

                // 使用 gy 來計算 pitch
                float gyro_rate = sensor_data.gy;

                // 互補濾波
                pitch_angle = 0.96f * (prev_pitch_angle + gyro_rate * dt) + 0.04 * accel_pitch ;
                prev_pitch_angle = pitch_angle;

                // 準備資料並發送到 Queue
                pitch_data_t pitch_data = {
                    .pitch = pitch_angle,
                    .timestamp = now  // 如果需要時間戳記
                };

                // 發送到 Queue，等待 10 ticks
                if (xQueueSend(pitch_queue, &pitch_data, pdMS_TO_TICKS(10)) != pdTRUE) {
                    // ESP_LOGW(TAG, "Queue full!");
                }

                // Debug 輸出
                printf(">Pitch:%f\n",pitch_angle) ; 
                // ESP_LOGI(TAG, "Pitch: %.2f°", pitch_angle);
            }
            
        } else {
            ESP_LOGE(TAG, "Failed to read sensor data");
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void apply_motor_control(float control_output) {
    uint32_t pwm_value = (uint32_t)(fabs(control_output));
    float motor_speed = (float)pwm_value;
    
    if (control_output > 0) {
        motor_set_direction_speed(MOTOR_LEFT, MOTOR_DIRECTION_FORWARD, 130);
        motor_set_direction_speed(MOTOR_RIGHT, MOTOR_DIRECTION_FORWARD, 130);
        debug_print_motor_data(motor_speed, motor_speed);
    } else {
        motor_set_direction_speed(MOTOR_LEFT, MOTOR_DIRECTION_BACKWARD, 130);
        motor_set_direction_speed(MOTOR_RIGHT, MOTOR_DIRECTION_BACKWARD, 130);
        debug_print_motor_data(-motor_speed, -motor_speed);
    }
}

static void control_task(void *pvParameters) {
    pid_controller_t pitch_controller;
    pid_init(&pitch_controller, 5.0f, 0.1f, 0.05f, -6.8f, -255.0f, 255.0f);
    
    debug_data_t debug_data = {0};
    pitch_data_t received_pitch;
    TickType_t last_wake_time = xTaskGetTickCount();
    
    while (1) {
        if (xQueueReceive(pitch_queue, &received_pitch, pdMS_TO_TICKS(10)) == pdTRUE) {
            // 計算 PID 輸出
            float control_output = pid_compute(&pitch_controller, received_pitch.pitch, 0.02f);
            
            // 更新除錯數據
            debug_data.pitch = received_pitch.pitch;
            debug_data.setpoint = pitch_controller.setpoint;
            debug_data.pid_output = control_output;
            debug_data.p_term = pitch_controller.p_term;
            debug_data.i_term = pitch_controller.i_term;
            debug_data.d_term = pitch_controller.d_term;
            
            // 應用馬達控制
            apply_motor_control(control_output);
            
            // 使用 Teleplot 格式打印除錯資訊
            debug_print_all(&debug_data);
        }
        
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(20));
    }
}



void app_main(void) {
    // 1. 初始化馬達 
    motor_init() ; 
    motor_pwm_init() ;
    motor_forward_backward_test(); 
    // 2. 初始化 MPU6050
    esp_err_t ret = mpu6050_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize MPU6050");
        return;
    }
    ESP_LOGI(TAG, "MPU6050 initialized successfully");

    // 3. 創建Quene
    pitch_queue = xQueueCreate(10, sizeof(pitch_data_t));
    if (pitch_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create queue!");
        return;
    }


    xTaskCreate(
        mpu6050_task,          // 任務函數
        "mpu6050_task",        // 任務名稱
        4096,                  // 堆疊大小
        NULL,                  // 任務參數
        5,                     // 任務優先級
        NULL                   // 任務句柄
    );
    ESP_LOGI(TAG, "MPU6050 task created");

    xTaskCreate(
        control_task,          // 任務函數
        "control_task",        // 任務名稱
        4096,                  // 堆疊大小
        NULL,                  // 任務參數
        5,                     // 任務優先級
        NULL                   // 任務句柄
    );
    ESP_LOGI(TAG, "Control task created");

    
}
