#include "debug.h"
#include <stdio.h>

void debug_print_value(const char* name, float value) {
    printf(">%s:%f\n", name, value);
}

void debug_print_pid_data(const pid_controller_t* pid, float current_angle, float output) {
    printf(">Pitch:%f;Setpoint:%f;Output:%f;P_Term:%f;I_Term:%f;D_Term:%f\n",
           current_angle,
           pid->setpoint,
           output,
           pid->p_term,
           pid->i_term,
           pid->d_term);
}

void debug_print_motor_data(float left_speed, float right_speed) {
    printf(">Left_Motor:%f;Right_Motor:%f\n", left_speed, right_speed);
}

void debug_print_all(const debug_data_t* debug_data) {
    printf(">Time:%f;Pitch:%f;Setpoint:%f;PID_Out:%f;P_Term:%f;I_Term:%f;D_Term:%f;L_Motor:%f;R_Motor:%f\n",
           debug_data->timestamp,
           debug_data->pitch,
           debug_data->setpoint,
           debug_data->pid_output,
           debug_data->p_term,
           debug_data->i_term,
           debug_data->d_term,
           debug_data->left_motor,
           debug_data->right_motor);
}
