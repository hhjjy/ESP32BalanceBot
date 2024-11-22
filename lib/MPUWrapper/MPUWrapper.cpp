// MPUWrapper.cpp
#include "MPUWrapper.h"
#include "MPU6050.h"
#include "MPU6050_6Axis_MotionApps20.h"

const char *MPUWrapper::TAG = "MPUWrapper";

MPUWrapper::MPUWrapper() {
    mpu = new MPU6050();
    q = new Quaternion();
    gravity = new VectorFloat();
    accel = new VectorInt16();
}

MPUWrapper::~MPUWrapper() {
    delete mpu;
    delete q;
    delete gravity;
    delete accel;
}

// ... 其餘方法保持不變 ...

esp_err_t MPUWrapper::i2c_master_init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {
            .clk_speed = I2C_MASTER_FREQ_HZ
        },
        .clk_flags = 0
    };
    
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) return err;
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

esp_err_t MPUWrapper::initialize() {
    mpu.initialize();
    ESP_LOGI(TAG, "MPU6050 初始化成功");

    if (!mpu.testConnection()) {
        ESP_LOGE(TAG, "MPU6050 連接測試失敗");
        return ESP_ERR_INVALID_RESPONSE;
    }
    ESP_LOGI(TAG, "MPU6050 連接測試成功");

    ESP_LOGI(TAG, "初始化 DMP...");
    uint8_t devStatus = mpu.dmpInitialize();

    if (devStatus == 0) {
        mpu.CalibrateGyro(6);
        mpu.CalibrateAccel(6);
        mpu.setDMPEnabled(true);
        dmpReady = true;
        packetSize = mpu.dmpGetFIFOPacketSize();
        ESP_LOGI(TAG, "DMP 初始化成功!");
        return ESP_OK;
    }
    
    ESP_LOGE(TAG, "DMP 初始化失敗 (代碼 %d)", devStatus);
    return ESP_ERR_INVALID_RESPONSE;
}

bool MPUWrapper::readAngles(float &pitch, float &roll, float &yaw) {
    if (!dmpReady) return false;

    fifoCount = mpu.getFIFOCount();
    
    if (fifoCount >= 1024) {
        mpu.resetFIFO();
        ESP_LOGE(TAG, "FIFO 溢出!");
        return false;
    }
    
    if (fifoCount < packetSize) return false;

    mpu.getFIFOBytes(fifoBuffer, packetSize);
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

    yaw = ypr[0] * 180/M_PI;
    pitch = ypr[1] * 180/M_PI;
    roll = ypr[2] * 180/M_PI;
    
    return true;
}

bool MPUWrapper::readWorldAccel(float &x, float &y, float &z) {
    if (!dmpReady) return false;

    fifoCount = mpu.getFIFOCount();
    
    if (fifoCount >= 1024) {
        mpu.resetFIFO();
        ESP_LOGE(TAG, "FIFO 溢出!");
        return false;
    }
    
    if (fifoCount < packetSize) return false;

    mpu.getFIFOBytes(fifoBuffer, packetSize);
    
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetAccel(&accel, fifoBuffer);
    mpu.dmpGetLinearAccel(&accel, &accel, &gravity);
    mpu.dmpGetLinearAccelInWorld(&accel, &accel, &q);

    x = accel.x / 16384.0f;
    y = accel.y / 16384.0f;
    z = accel.z / 16384.0f;

    return true;
}

bool MPUWrapper::readSensorAccel(float &x, float &y, float &z) {
    if (!dmpReady) return false;

    fifoCount = mpu.getFIFOCount();
    
    if (fifoCount >= 1024) {
        mpu.resetFIFO();
        ESP_LOGE(TAG, "FIFO 溢出!");
        return false;
    }
    
    if (fifoCount < packetSize) return false;

    mpu.getFIFOBytes(fifoBuffer, packetSize);
    
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetAccel(&accel, fifoBuffer);
    mpu.dmpGetLinearAccel(&accel, &accel, &gravity);

    x = accel.x / 16384.0f;
    y = accel.y / 16384.0f;
    z = accel.z / 16384.0f;

    return true;
}
