#include "pid.h"
#include <string.h>

void pid_init(pid_controller_t *pid, 
             float kp, 
             float ki, 
             float kd, 
             float setpoint,
             float output_min, 
             float output_max) {
    // 初始化 PID 參數
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->setpoint = setpoint;
    pid->output_min = output_min;
    pid->output_max = output_max;
    
    // 重置內部狀態
    pid_reset(pid);
    
    // 設置默認配置
    pid->anti_windup = true;
    pid->derivative_on_measurement = false;
}

float pid_compute(pid_controller_t *pid, float input, float dt) {
    if (dt <= 0.0f) {
        return 0.0f;
    }

    // 計算誤差
    float error = pid->setpoint - input;
    
    // 比例項
    pid->p_term = pid->kp * error;
    
    // 積分項
    pid->integral += error * dt;

    // 積分限幅
    if (pid->anti_windup) {
        if (pid->integral > pid->output_max) {
            pid->integral = pid->output_max;
        } else if (pid->integral < pid->output_min) {
            pid->integral = pid->output_min;
        }
    }
    pid->i_term = pid->ki * pid->integral;
    
    // 微分項
    float d_term;
    if (pid->derivative_on_measurement) {
        // 在測量值上微分
        d_term = -pid->kd * (input - pid->prev_measurement) / dt;
        pid->prev_measurement = input;
    } else {
        // 在誤差上微分
        d_term = pid->kd * (error - pid->prev_error) / dt;
        pid->prev_error = error;
    }
    pid->d_term = d_term;
    // 計算總輸出
    float output = pid->p_term + pid->i_term + d_term;
    
    // 輸出限幅
    if (output > pid->output_max) {
        output = pid->output_max;
    } else if (output < pid->output_min) {
        output = pid->output_min;
    }
    
    return output;
}

void pid_reset(pid_controller_t *pid) {
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->prev_measurement = 0.0f;
}

void pid_set_gains(pid_controller_t *pid, float kp, float ki, float kd) {
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
}

void pid_set_setpoint(pid_controller_t *pid, float setpoint) {
    pid->setpoint = setpoint;
}

void pid_set_output_limits(pid_controller_t *pid, float min, float max) {
    if (min < max) {
        pid->output_min = min;
        pid->output_max = max;
    }
}

void pid_set_anti_windup(pid_controller_t *pid, bool enable) {
    pid->anti_windup = enable;
}

void pid_set_derivative_on_measurement(pid_controller_t *pid, bool on_measurement) {
    pid->derivative_on_measurement = on_measurement;
}

float pid_get_proportional(pid_controller_t *pid) {
    return pid->kp * (pid->setpoint - pid->prev_measurement);
}

float pid_get_integral(pid_controller_t *pid) {
    return pid->ki * pid->integral;
}

float pid_get_derivative(pid_controller_t *pid) {
    if (pid->derivative_on_measurement) {
        return -pid->kd * pid->prev_measurement;
    } else {
        return pid->kd * pid->prev_error;
    }
}
