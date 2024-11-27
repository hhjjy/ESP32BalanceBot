#ifndef _PID_H_
#define _PID_H_

#include <stdint.h>
#include <stdbool.h>

// PID 控制器結構體
typedef struct {
    // PID 參數
    float kp;                  // 比例增益
    float ki;                  // 積分增益
    float kd;                  // 微分增益
    
    // 控制目標和限制
    float setpoint;            // 目標值
    float output_min;          // 輸出下限
    float output_max;          // 輸出上限
    
    // 內部狀態
    float integral;            // 積分項
    float prev_error;          // 上一次誤差
    float prev_measurement;    // 上一次測量值
    
    // 控制標誌
    bool anti_windup;         // 積分限幅標誌
    bool derivative_on_measurement; // 是否在測量值上微分
} pid_controller_t;

// PID 初始化函數
void pid_init(pid_controller_t *pid, 
             float kp, 
             float ki, 
             float kd, 
             float setpoint,
             float output_min, 
             float output_max);

// PID 計算函數
float pid_compute(pid_controller_t *pid, float input, float dt);

// PID 參數設置函數
void pid_set_gains(pid_controller_t *pid, float kp, float ki, float kd);
void pid_set_setpoint(pid_controller_t *pid, float setpoint);
void pid_set_output_limits(pid_controller_t *pid, float min, float max);

// PID 狀態重置函數
void pid_reset(pid_controller_t *pid);

// PID 配置函數
void pid_set_anti_windup(pid_controller_t *pid, bool enable);
void pid_set_derivative_on_measurement(pid_controller_t *pid, bool on_measurement);

// PID 狀態獲取函數
float pid_get_proportional(pid_controller_t *pid);
float pid_get_integral(pid_controller_t *pid);
float pid_get_derivative(pid_controller_t *pid);

#endif // _PID_H_
