#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "MPU6050_6Axis_MotionApps20.h"

static const char *TAG = "MPU6050_DMP";
// I2C配置
#define I2C_MASTER_SCL_IO           0      // SCL腳位
#define I2C_MASTER_SDA_IO           1      // SDA腳位
#define I2C_MASTER_NUM              I2C_NUM_0       // I2C port number
#define I2C_MASTER_FREQ_HZ          400000  // I2C frequency
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0
// MPU6050相關變量
MPU6050 mpu;
bool dmpReady = false;
uint8_t mpuIntStatus;
uint8_t devStatus;
uint16_t packetSize;
uint16_t fifoCount;
uint8_t fifoBuffer[64];

// 姿態和運動變量
Quaternion q;           // [w, x, y, z]
VectorInt16 aa;        // [x, y, z]            加速度感測器測量值
VectorInt16 aaReal;    // [x, y, z]            重力去除後的加速度
VectorFloat gravity;    // [x, y, z]            重力向量
float ypr[3];          // [yaw, pitch, roll]   偏航角、俯仰角、翻滾角

// I2C初始化函數
static esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {
            .clk_speed = I2C_MASTER_FREQ_HZ,
        },
    };
    
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) {
        return err;
    }

    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 
                            I2C_MASTER_RX_BUF_DISABLE,
                            I2C_MASTER_TX_BUF_DISABLE, 0);
}
// MPU6050初始化和DMP配置
static void mpu6050_init(void)
{
    ESP_LOGI(TAG, "Initializing MPU6050...");
    mpu.initialize();

    // 驗證設備連接
    ESP_LOGI(TAG, "Testing device connections...");
    if (mpu.testConnection()) {
        ESP_LOGI(TAG, "MPU6050 connection successful");
    } else {
        ESP_LOGE(TAG, "MPU6050 connection failed");
        return;
    }

    // 初始化DMP
    ESP_LOGI(TAG, "Initializing DMP...");
    devStatus = mpu.dmpInitialize();

    // 設置陀螺儀偏移量（這些值需要通過校準獲得）
    mpu.setXGyroOffset(220);
    mpu.setYGyroOffset(76);
    mpu.setZGyroOffset(-85);
    mpu.setZAccelOffset(1788);

    if (devStatus == 0) {
        // 開啟DMP
        ESP_LOGI(TAG, "Enabling DMP...");
        mpu.setDMPEnabled(true);
        dmpReady = true;
        packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
        ESP_LOGE(TAG, "DMP Initialization failed (code %d)", devStatus);
    }
}

// MPU6050數據讀取任務
static void mpu6050_task(void *arg)
{
    while (1) {
        if (!dmpReady) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        // 等待新數據
        while (fifoCount < packetSize) {
            fifoCount = mpu.getFIFOCount();
        }

        // 讀取FIFO數據
        mpu.getFIFOBytes(fifoBuffer, packetSize);
        fifoCount -= packetSize;

        // 獲取姿態數據
        mpu.dmpGetQuaternion(&q, fifoBuffer);
        mpu.dmpGetGravity(&gravity, &q);
        mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

        // 獲取加速度數據
        mpu.dmpGetAccel(&aa, fifoBuffer);
        mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);

        // 輸出數據
        ESP_LOGI(TAG, "Yaw: %.2f, Pitch: %.2f, Roll: %.2f", 
                ypr[0] * 180/M_PI, 
                ypr[1] * 180/M_PI, 
                ypr[2] * 180/M_PI);
        
        ESP_LOGI(TAG, "Accel (Raw) - X: %d, Y: %d, Z: %d", 
                aa.x, aa.y, aa.z);
        
        ESP_LOGI(TAG, "Accel (Real) - X: %d, Y: %d, Z: %d", 
                aaReal.x, aaReal.y, aaReal.z);

        vTaskDelay(pdMS_TO_TICKS(50)); // 20Hz更新率
    }
}

extern "C" void app_main(void)
{
    // 初始化I2C
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");

    // 初始化MPU6050和DMP
    mpu6050_init();

    // 創建MPU6050數據讀取任務
    xTaskCreate(mpu6050_task, "mpu6050_task", 2048, NULL, 5, NULL);
}