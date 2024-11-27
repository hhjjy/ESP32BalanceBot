#ifndef MOTOR_H
#define MOTOR_H

#include <driver/gpio.h>
#include <driver/ledc.h>
// | IO3     | GPIO    | RTC_GPIO3 | TOUCH3/ADC1_CH2 | ✅ 已使用 | TB6612 E1A |
// | IO46    | GPIO    | -       | SPIWP    | ✅ 已使用   | TB6612 E1B |
// | IO9     | GPIO    | RTC_GPIO9 | TOUCH9/ADC1_CH8 | ✅ 已使用 | TB6612 E2A |
// | IO10    | GPIO    | RTC_GPIO10 | TOUCH10/ADC1_CH9 | ✅ 已使用 | TB6612 E2B |
// 輸入Pin 定義
// 左邊馬達:1
// 右邊馬達:2
#define MOTOR_E1A GPIO_NUM_3
#define MOTOR_E1B GPIO_NUM_46
#define MOTOR_E2A GPIO_NUM_9
#define MOTOR_E2B GPIO_NUM_10
// 輸出Pin 定義
#define MOTOR_PWMB GPIO_NUM_41
#define MOTOR_BIN2 GPIO_NUM_40
#define MOTOR_BIN1 GPIO_NUM_39

#define MOTOR_PWMA GPIO_NUM_38
#define MOTOR_AIN1 GPIO_NUM_37
#define MOTOR_AIN2 GPIO_NUM_36
#define MOTOR_STBY GPIO_NUM_35

// PWM 設定
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES          LEDC_TIMER_8_BIT // 8 位元解析度
#define LEDC_FREQUENCY         (20000) // 5KHz
#define MOTOR_PWM_CHANNEL_A     LEDC_CHANNEL_0
#define MOTOR_PWM_CHANNEL_B     LEDC_CHANNEL_1

#define OUTPUT_HIGH 1
#define OUTPUT_LOW 0

// 修改原本的馬達方向定義
typedef enum {
    MOTOR_LEFT = 1,    // 左馬達
    MOTOR_RIGHT = 2    // 右馬達
} MotorSelect;

typedef enum {
    MOTOR_DIRECTION_FORWARD,    // 正轉
    MOTOR_DIRECTION_BACKWARD,   // 反轉
    MOTOR_DIRECTION_BRAKE,      // 剎車
    MOTOR_DIRECTION_STOP        // 停止
} MotorDirection;

// 修改函數宣告
void motor_set_direction_speed(MotorSelect motor, MotorDirection direction, uint8_t speed);
void motor_stop_all(void);

// 函數宣告
void motor_init(void);
void motor_pwm_init(void);
void motor_set_pin(gpio_num_t pin, int level);
int motor_read_pin(gpio_num_t pin);
void motor_set_speed_A(uint8_t speed);
void motor_set_speed_B(uint8_t speed);
void motor_forward_backward_test(void);


#endif
