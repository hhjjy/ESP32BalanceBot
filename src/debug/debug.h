#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "pid/pid.h"

// 除錯數據結構
typedef struct {
    float timestamp;      // 時間戳
    float pitch;         // 當前角度
    float setpoint;      // 目標角度
    float pid_output;    // PID 輸出
    float p_term;        // P 項
    float i_term;        // I 項
    float d_term;        // D 項
    float left_motor;    // 左馬達輸出
    float right_motor;   // 右馬達輸出
} debug_data_t;

// Teleplot 打印函數
void debug_print_value(const char* name, float value);
void debug_print_pid_data(const pid_controller_t* pid, float current_angle, float output);
void debug_print_motor_data(float left_speed, float right_speed);
void debug_print_all(const debug_data_t* debug_data);

#endif // _DEBUG_H_
