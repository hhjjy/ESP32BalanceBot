#include "motor.h"
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "MOTOR";

void motor_pwm_init(void) {
    // 設定 LEDC Timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz = LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    // 設定 PWMA Channel
    ledc_channel_config_t ledc_channel_a = {
        .speed_mode = LEDC_MODE,
        .channel = MOTOR_PWM_CHANNEL_A,
        .timer_sel = LEDC_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = MOTOR_PWMA,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&ledc_channel_a);

    // 設定 PWMB Channel
    ledc_channel_config_t ledc_channel_b = {
        .speed_mode = LEDC_MODE,
        .channel = MOTOR_PWM_CHANNEL_B,
        .timer_sel = LEDC_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = MOTOR_PWMB,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&ledc_channel_b);
}
void motor_set_pin(gpio_num_t pin, int level) {
    gpio_set_level(pin, level);
    ESP_LOGI(TAG, "Pin %d set to %d (實際輸出: %s)", 
             pin, 
             level, 
             level == OUTPUT_HIGH ? "LOW (0V)" : "HIGH (5V)");
}

int motor_read_pin(gpio_num_t pin) {
    int level = gpio_get_level(pin);
    ESP_LOGI(TAG, "Pin %d read: %d (實際電平: %s)", 
             pin, 
             level, 
             level == OUTPUT_HIGH ? "LOW (0V)" : "HIGH (5V)");
    return level;
}

void motor_init(void) {
    // 設定一般 GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL<<MOTOR_AIN1) | 
                        (1ULL<<MOTOR_AIN2) |
                        (1ULL<<MOTOR_BIN1) | 
                        (1ULL<<MOTOR_BIN2) |
                        (1ULL<<MOTOR_STBY),
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // 初始化 PWM
    motor_pwm_init();

    // 初始化所有控制腳位
    motor_set_pin(MOTOR_AIN1, OUTPUT_HIGH);
    motor_set_pin(MOTOR_AIN2, OUTPUT_HIGH);
    motor_set_pin(MOTOR_BIN1, OUTPUT_HIGH);
    motor_set_pin(MOTOR_BIN2, OUTPUT_HIGH);
    motor_set_pin(MOTOR_STBY, OUTPUT_HIGH);

    // 設定初始 PWM 為 0
    motor_set_speed_A(0);
    motor_set_speed_B(0);
}

void motor_set_speed_A(uint8_t speed) {
    // 設定 PWMA 的 duty cycle (0-255)
    uint32_t duty = (speed * ((1 << LEDC_DUTY_RES) - 1)) / 255;
    ledc_set_duty(LEDC_MODE, MOTOR_PWM_CHANNEL_A, duty);
    ledc_update_duty(LEDC_MODE, MOTOR_PWM_CHANNEL_A);
    ESP_LOGI(TAG, "Motor A speed set to %d", speed);
}

void motor_set_speed_B(uint8_t speed) {
    // 設定 PWMB 的 duty cycle (0-255)
    uint32_t duty = (speed * ((1 << LEDC_DUTY_RES) - 1)) / 255;
    ledc_set_duty(LEDC_MODE, MOTOR_PWM_CHANNEL_B, duty);
    ledc_update_duty(LEDC_MODE, MOTOR_PWM_CHANNEL_B);
    ESP_LOGI(TAG, "Motor B speed set to %d", speed);
}

// 修改 motor_forward_backward_test 函數
void motor_forward_backward_test(void) {
    ESP_LOGI(TAG, "開始馬達正反轉測試");
    
    // 啟用 STBY
    motor_set_pin(MOTOR_STBY, OUTPUT_LOW);
    vTaskDelay(pdMS_TO_TICKS(1000));

    // 測試馬達 A
    ESP_LOGI(TAG, "測試馬達 A");
    
    // 馬達 A 正轉
    ESP_LOGI(TAG, "馬達 A 正轉");
    motor_set_speed_A(200);  // 設定速度約 80%
    motor_set_pin(MOTOR_AIN1, OUTPUT_LOW);
    motor_set_pin(MOTOR_AIN2, OUTPUT_HIGH);
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // 馬達 A 停止
    ESP_LOGI(TAG, "馬達 A 停止");
    motor_set_speed_A(0);
    motor_set_pin(MOTOR_AIN1, OUTPUT_HIGH);
    motor_set_pin(MOTOR_AIN2, OUTPUT_HIGH);
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 馬達 A 反轉
    ESP_LOGI(TAG, "馬達 A 反轉");
    motor_set_speed_A(200);
    motor_set_pin(MOTOR_AIN1, OUTPUT_HIGH);
    motor_set_pin(MOTOR_AIN2, OUTPUT_LOW);
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // 馬達 A 停止
    motor_set_speed_A(0);
    motor_set_pin(MOTOR_AIN1, OUTPUT_HIGH);
    motor_set_pin(MOTOR_AIN2, OUTPUT_HIGH);
    vTaskDelay(pdMS_TO_TICKS(2000));

    // 測試馬達 B (類似的測試程序)
    ESP_LOGI(TAG, "測試馬達 B");
    
    // 馬達 B 正轉
    ESP_LOGI(TAG, "馬達 B 正轉");
    motor_set_speed_B(200);
    motor_set_pin(MOTOR_BIN1, OUTPUT_LOW);
    motor_set_pin(MOTOR_BIN2, OUTPUT_HIGH);
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // 以此類推...

    // 最後停用 STBY
    motor_set_pin(MOTOR_STBY, OUTPUT_HIGH);
    ESP_LOGI(TAG, "馬達測試完成");
}
