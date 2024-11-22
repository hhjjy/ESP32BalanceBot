我來幫您完整整理包含 MPU6050 實例的 I²C 通訊教學：

# I²C 通訊協議完整教學

## 一、基礎理論

### 1. 基本概念
- **SCL（Serial Clock）**：時脈線，由主設備控制通訊節奏
- **SDA（Serial Data）**：數據線，用於雙向數據傳輸
- **通訊速率**：標準模式 100kHz，快速模式 400kHz

### 2. 信號條件
- **起始條件（Start）**：SCL 高電平時，SDA 由高轉低
- **結束條件（Stop）**：SCL 高電平時，SDA 由低轉高
- **重複起始（Repeated Start）**：無需發送停止條件即可重新開始

## 二、通訊流程

### 1. 單字節寫入
```
Master: S → AD + W → REG → DATA → P
Slave:     ↓ACK    ↓ACK  ↓ACK
```

### 2. 連續寫入
```
Master: S → AD + W → REG → DATA1 → DATA2 → ... → P
Slave:     ↓ACK    ↓ACK  ↓ACK    ↓ACK
```

### 3. 單字節讀取
```
Master: S → AD + W → REG → S → AD + R → [DATA] → NACK → P
Slave:     ↓ACK    ↓ACK       ↓ACK    ↑DATA
```

### 4. 連續讀取
```
Master: S → AD + W → REG → S → AD + R → [DATA1] → ACK → [DATA2] → NACK → P
Slave:     ↓ACK    ↓ACK       ↓ACK    ↑DATA1         ↑DATA2
```

## 三、ESP-IDF 實現

### 1. 基礎配置
```c
#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/task.h"

#define I2C_MASTER_SCL_IO           22      // SCL 腳位
#define I2C_MASTER_SDA_IO           21      // SDA 腳位
#define I2C_MASTER_NUM              0       // I2C 端口號
#define I2C_MASTER_FREQ_HZ          400000  // 頻率
#define I2C_MASTER_TIMEOUT_MS       1000    // 超時時間

// MPU6050 相關定義
#define MPU6050_ADDR    0x68
#define MPU6050_WHO_AM_I     0x75
#define MPU6050_PWR_MGMT_1   0x6B
#define MPU6050_ACCEL_XOUT_H 0x3B

// I2C 初始化
static esp_err_t i2c_master_init(void)
{

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}
```

### 2. 四種通訊模式實現

```c
// 1. 單字節寫入
static esp_err_t i2c_single_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};
    return i2c_master_write_to_device(I2C_MASTER_NUM, dev_addr, write_buf, 
                                    sizeof(write_buf), 
                                    pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
}

// 2. 連續寫入
static esp_err_t i2c_burst_write(uint8_t dev_addr, uint8_t reg_addr, 
                                uint8_t *data, size_t len)
{
    uint8_t *write_buf = malloc(len + 1);
    if (!write_buf) return ESP_ERR_NO_MEM;
    
    write_buf[0] = reg_addr;
    memcpy(write_buf + 1, data, len);
    
    esp_err_t ret = i2c_master_write_to_device(I2C_MASTER_NUM, dev_addr, 
                                              write_buf, len + 1, 
                                              pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    free(write_buf);
    return ret;
}

// 3. 單字節讀取
static esp_err_t i2c_single_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data)
{
    return i2c_master_write_read_device(I2C_MASTER_NUM, dev_addr, &reg_addr, 1,
                                      data, 1, 
                                      pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
}

// 4. 連續讀取
static esp_err_t i2c_burst_read(uint8_t dev_addr, uint8_t reg_addr, 
                               uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(I2C_MASTER_NUM, dev_addr, &reg_addr, 1,
                                      data, len, 
                                      pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
}
```

### 3. MPU6050 應用示例

```c
// MPU6050 數據結構
typedef struct {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t temp;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
} mpu6050_data_t;

// 初始化 MPU6050
static esp_err_t mpu6050_init(void)
{
    uint8_t check;
    
    // 檢查設備 ID
    ESP_ERROR_CHECK(i2c_single_read(MPU6050_ADDR, MPU6050_WHO_AM_I, &check));
    if (check != MPU6050_ADDR) {
        ESP_LOGE("MPU6050", "Device not found!");
        return ESP_FAIL;
    }
    
    // 喚醒設備
    ESP_ERROR_CHECK(i2c_single_write(MPU6050_ADDR, MPU6050_PWR_MGMT_1, 0x00));
    
    return ESP_OK;
}

// 讀取 MPU6050 所有數據
static esp_err_t mpu6050_read_all(mpu6050_data_t *data)
{
    uint8_t buf[14];
    
    // 連續讀取 14 個字節的數據
    ESP_ERROR_CHECK(i2c_burst_read(MPU6050_ADDR, MPU6050_ACCEL_XOUT_H, buf, 14));
    
    // 組合數據
    data->accel_x = (int16_t)((buf[0] << 8) | buf[1]);
    data->accel_y = (int16_t)((buf[2] << 8) | buf[3]);
    data->accel_z = (int16_t)((buf[4] << 8) | buf[5]);
    data->temp = (int16_t)((buf[6] << 8) | buf[7]);
    data->gyro_x = (int16_t)((buf[8] << 8) | buf[9]);
    data->gyro_y = (int16_t)((buf[10] << 8) | buf[11]);
    data->gyro_z = (int16_t)((buf[12] << 8) | buf[13]);
    
    return ESP_OK;
}

// 主程序
void app_main(void)
{
    // 初始化 I2C
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI("I2C", "I2C initialized successfully");
    
    // 初始化 MPU6050
    ESP_ERROR_CHECK(mpu6050_init());
    ESP_LOGI("MPU6050", "MPU6050 initialized successfully");
    
    mpu6050_data_t sensor_data;
    
    // 主循環
    while (1) {
        ESP_ERROR_CHECK(mpu6050_read_all(&sensor_data));
        
        // 打印數據
        ESP_LOGI("MPU6050", "Accel: X=%d, Y=%d, Z=%d",
                 sensor_data.accel_x,
                 sensor_data.accel_y,
                 sensor_data.accel_z);
                 
        ESP_LOGI("MPU6050", "Gyro: X=%d, Y=%d, Z=%d",
                 sensor_data.gyro_x,
                 sensor_data.gyro_y,
                 sensor_data.gyro_z);
                 
        float temp = sensor_data.temp / 340.0 + 36.53;
        ESP_LOGI("MPU6050", "Temp: %.2f°C", temp);
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

## 四、注意事項

1. **時序要求**：I²C 通訊必須嚴格遵循時序要求
2. **上拉電阻**：SDA 和 SCL 需要外部上拉電阻（通常 4.7kΩ）
3. **位址衝突**：確保總線上的設備位址不重複
4. **錯誤處理**：要有完善的錯誤處理機制
5. **超時處理**：設置合適的超時時間，避免程序卡死

以上就是完整的 I²C 通訊教學，包含了理論基礎、通訊流程、ESP-IDF 實現以及 MPU6050 的實際應用示例。