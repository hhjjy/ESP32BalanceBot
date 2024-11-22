#pragma once

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c.h"

// 前向聲明
class MPU6050;
class Quaternion;
class VectorInt16;
class VectorFloat;

class MPUWrapper {
private:
    static const char *TAG;
    MPU6050* mpu;
    bool dmpReady = false;
    uint16_t packetSize;
    uint16_t fifoCount;
    uint8_t fifoBuffer[64];
    uint8_t mpuIntStatus;

    // 姿態數據
    Quaternion* q;
    VectorFloat* gravity;
    VectorInt16* accel;
    float ypr[3];

    static const int I2C_MASTER_SCL_IO = 0;
    static const int I2C_MASTER_SDA_IO = 1;
    static const i2c_port_t I2C_MASTER_NUM = I2C_NUM_0;
    static const uint32_t I2C_MASTER_FREQ_HZ = 400000;

public:
    MPUWrapper();
    ~MPUWrapper();
    esp_err_t i2c_master_init();
    esp_err_t initialize();
    bool readAngles(float &pitch, float &roll, float &yaw);
    bool readWorldAccel(float &x, float &y, float &z);
    bool readSensorAccel(float &x, float &y, float &z);
    bool isDMPReady() { return dmpReady; }
};
