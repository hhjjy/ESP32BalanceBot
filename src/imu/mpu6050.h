#pragma once

#include <stdio.h>
#include "esp_err.h"
#include "driver/i2c_master.h"  // 使用新版 API
#include "esp_log.h"

#ifdef __cplusplus
extern "C" {
#endif


#define I2C_MASTER_NUM              I2C_NUM_0    // I2C port number
#define I2C_MASTER_SCL_IO           GPIO_NUM_6            // GPIO number for I2C SCL (改為 GPIO0)
#define I2C_MASTER_SDA_IO           GPIO_NUM_5            // GPIO number for I2C SDA (改為 GPIO1)
#define I2C_MASTER_FREQ_HZ          400000       // I2C master clock frequency
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
esp_err_t mpu6050_read_reg(uint8_t reg_addr, uint8_t *data, size_t len); 
esp_err_t mpu6050_write_reg(uint8_t reg_addr, uint8_t data);

int mpu6050_i2c_scan();



#ifdef __cplusplus
}
#endif

