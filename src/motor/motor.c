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
}

int motor_read_pin(gpio_num_t pin) {
    int level = gpio_get_level(pin);
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
    motor_set_pin(MOTOR_AIN1, OUTPUT_LOW);
    motor_set_pin(MOTOR_AIN2, OUTPUT_LOW);
    motor_set_pin(MOTOR_BIN1, OUTPUT_LOW);
    motor_set_pin(MOTOR_BIN2, OUTPUT_LOW);
    motor_set_pin(MOTOR_STBY, OUTPUT_LOW);


}

void motor_set_speed_A(uint8_t speed) {
    // 設定 PWMA 的 duty cycle (0-128)
    uint32_t duty = (speed * ((1 << LEDC_DUTY_RES) - 1)) / 255;
    ledc_set_duty(LEDC_MODE, MOTOR_PWM_CHANNEL_A, duty);
    ledc_update_duty(LEDC_MODE, MOTOR_PWM_CHANNEL_A);
}

void motor_set_speed_B(uint8_t speed) {
    // 設定 PWMB 的 duty cycle (0-128)
    uint32_t duty = (speed * ((1 << LEDC_DUTY_RES) - 1)) / 255;
    ledc_set_duty(LEDC_MODE, MOTOR_PWM_CHANNEL_B, duty);
    ledc_update_duty(LEDC_MODE, MOTOR_PWM_CHANNEL_B);
}

void motor_set_direction_speed(MotorSelect motor, MotorDirection direction, uint8_t speed) {
    motor_set_pin(MOTOR_STBY, OUTPUT_HIGH);  // 啟用馬達

    if (motor == MOTOR_LEFT) {  // 左馬達 (Motor A)
        switch (direction) {
            case MOTOR_DIRECTION_FORWARD:
                motor_set_pin(MOTOR_AIN1, OUTPUT_HIGH);
                motor_set_pin(MOTOR_AIN2, OUTPUT_LOW);
                motor_set_speed_A(speed);
                break;
            case MOTOR_DIRECTION_BACKWARD:
                motor_set_pin(MOTOR_AIN1, OUTPUT_LOW);
                motor_set_pin(MOTOR_AIN2, OUTPUT_HIGH);
                motor_set_speed_A(speed);
                break;
            case MOTOR_DIRECTION_BRAKE:
                motor_set_pin(MOTOR_AIN1, OUTPUT_HIGH);
                motor_set_pin(MOTOR_AIN2, OUTPUT_HIGH);
                motor_set_speed_A(speed);
                break;
            case MOTOR_DIRECTION_STOP:
                motor_set_pin(MOTOR_AIN1, OUTPUT_LOW);
                motor_set_pin(MOTOR_AIN2, OUTPUT_LOW);
                motor_set_speed_A(0);
                break;
        }
    } 
    else if (motor == MOTOR_RIGHT) {  // 右馬達 (Motor B)
        switch (direction) {
            case MOTOR_DIRECTION_FORWARD:
                motor_set_pin(MOTOR_BIN1, OUTPUT_HIGH);
                motor_set_pin(MOTOR_BIN2, OUTPUT_LOW);
                motor_set_speed_B(speed);
                break;
            case MOTOR_DIRECTION_BACKWARD:
                motor_set_pin(MOTOR_BIN1, OUTPUT_LOW);
                motor_set_pin(MOTOR_BIN2, OUTPUT_HIGH);
                motor_set_speed_B(speed);
                break;
            case MOTOR_DIRECTION_BRAKE:
                motor_set_pin(MOTOR_BIN1, OUTPUT_HIGH);
                motor_set_pin(MOTOR_BIN2, OUTPUT_HIGH);
                motor_set_speed_B(speed);
                break;
            case MOTOR_DIRECTION_STOP:
                motor_set_pin(MOTOR_BIN1, OUTPUT_LOW);
                motor_set_pin(MOTOR_BIN2, OUTPUT_LOW);
                motor_set_speed_B(0);
                break;
        }
    }
}

void motor_stop_all(void) {
    // 停止所有馬達
    motor_set_direction_speed(MOTOR_LEFT, MOTOR_DIRECTION_STOP, 0);
    motor_set_direction_speed(MOTOR_RIGHT, MOTOR_DIRECTION_STOP, 0);
    motor_set_pin(MOTOR_STBY, OUTPUT_LOW);  // 進入待機模式
}


void motor_forward_backward_test(void) {
    ESP_LOGI(TAG, "開始馬達正反轉測試");
    
    // 左馬達測試
    ESP_LOGI(TAG, "左馬達正轉");
    motor_set_direction_speed(MOTOR_LEFT, MOTOR_DIRECTION_FORWARD, 150);
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    ESP_LOGI(TAG, "左馬達反轉");
    motor_set_direction_speed(MOTOR_LEFT, MOTOR_DIRECTION_BACKWARD, 150);
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 右馬達測試
    ESP_LOGI(TAG, "右馬達正轉");
    motor_set_direction_speed(MOTOR_RIGHT, MOTOR_DIRECTION_FORWARD, 150);
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    ESP_LOGI(TAG, "右馬達反轉");
    motor_set_direction_speed(MOTOR_RIGHT, MOTOR_DIRECTION_BACKWARD, 150);
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 停止所有馬達
    motor_stop_all();
    
    ESP_LOGI(TAG, "馬達測試完成");
}
