#pragma once

#include <stdio.h>
#include "esp_err.h"
#include "driver/i2c.h"
#include "esp_log.h"

#ifdef __cplusplus
extern "C" {
#endif

// I2C 配置
#define I2C_MASTER_SCL_IO           0      // SCL GPIO
#define I2C_MASTER_SDA_IO           1      // SDA GPIO
#define I2C_MASTER_NUM              0       // I2C port number
#define I2C_MASTER_FREQ_HZ          400000  // I2C master clock frequency
#define I2C_MASTER_TIMEOUT_MS       1000
#define I2C_READ                    0 
#define I2C_Write                    0 
// MPU6050 配置
#define MPU6050_ADDR               0x68    // MPU6050 device address
#define MPU6050_WHO_AM_I           0x75    // Who Am I register address
#define MPU6050_PWR_MGMT_1         0x6B    // Power Management 1 register
#define MPU6050_ACCEL_XOUT_H       0x3B    // First register of accelerometer data
#define MPU6050_GYRO_XOUT_H        0x43    // First register of gyroscope data

// 結構體宣告
typedef struct {
    float ax ;
    float ay ;
    float az ;
    float temp;
    float gx ;
    float gy ;
    float gz ; 
} mpu6050_data_t;
// 函數宣告
esp_err_t mpu6050_init(void);
esp_err_t mpu6050_read_data(mpu6050_data_t*);
esp_err_t mpu6050_who_am_i(void); 


#ifdef __cplusplus
}
#endif

