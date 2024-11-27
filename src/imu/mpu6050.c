#include "mpu6050.h"

static const char *TAG = "MPU6050";
// I2C 配置
static i2c_master_bus_handle_t i2c_bus_handle;   // I2C bus handle
static i2c_master_dev_handle_t i2c_dev_handle;   // MPU6050 device handle

static esp_err_t i2c_master_init(void)
{
    ESP_LOGI(TAG, "Initializing I2C master with new driver (SCL: GPIO%d, SDA: GPIO%d)", 
             I2C_MASTER_SCL_IO, I2C_MASTER_SDA_IO);
    
    // 1. 配置 I2C 主機匯流排
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_NUM,
        .scl_io_num = I2C_MASTER_SCL_IO,    // GPIO0
        .sda_io_num = I2C_MASTER_SDA_IO,    // GPIO1
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    
    // 2. 安裝 I2C 主機驅動程序
    esp_err_t ret = i2c_new_master_bus(&i2c_mst_config, &i2c_bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C master bus: %s", esp_err_to_name(ret));
        return ret;
    }

    // 3. 配置 MPU6050 設備參數
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,    // 7-bit address
        .device_address = MPU6050_ADDR,           // MPU6050 address
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,      // Clock speed
    };

    // 4. 將 MPU6050 設備添加到 I2C 匯流排
    ret = i2c_master_bus_add_device(i2c_bus_handle, &dev_cfg, &i2c_dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add MPU6050 device: %s", esp_err_to_name(ret));
        // 清理已分配的資源
        i2c_master_bus_rm_device(i2c_dev_handle);
        i2c_del_master_bus(i2c_bus_handle);
        return ret;
    }

    ESP_LOGI(TAG, "I2C master initialization completed successfully");
    return ESP_OK;
}

esp_err_t mpu6050_init(void)
{
    esp_err_t ret = i2c_master_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C init failed");
        return ret;
    }

    // 檢查設備 ID (WHO_AM_I)
    uint8_t who_am_i;
    ret = mpu6050_read_reg(MPU6050_WHO_AM_I, &who_am_i, 1);
    if (ret != ESP_OK || who_am_i != 0x68) {
        ESP_LOGE(TAG, "MPU6050 Not found (WHO_AM_I = 0x%02X)", who_am_i);
        return ESP_ERR_NOT_FOUND;
    }

    // 喚醒 MPU6050
    ret = mpu6050_write_reg(MPU6050_PWR_MGMT_1, 0x00);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "MPU6050 Wake up failed");
        return ret;
    }

    ESP_LOGI(TAG, "MPU6050 initialized successfully");
    return ESP_OK;
}

esp_err_t mpu6050_read_data(mpu6050_data_t *data)
{
    uint8_t buffer[14];
    uint8_t reg_addr = 0x3B;  // 起始寄存器地址
    
    // 使用新版 API 讀取數據
    esp_err_t ret = i2c_master_transmit_receive(i2c_dev_handle, 
                                              &reg_addr, 1,    // 寫入寄存器地址
                                              buffer, 14,      // 讀取 14 bytes 數據
                                              -1);             // 無超時

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read sensor data: %s", esp_err_to_name(ret));
        return ret;
    }

    // 數據轉換保持不變
    data->ax = (int16_t)((buffer[0] << 8) | buffer[1]) / 16384.0f;
    data->ay = (int16_t)((buffer[2] << 8) | buffer[3]) / 16384.0f;
    data->az = (int16_t)((buffer[4] << 8) | buffer[5]) / 16384.0f;
    data->temp = (int16_t)((buffer[6] << 8) | buffer[7]) / 340.0f + 36.53f;
    data->gx = (int16_t)((buffer[8] << 8) | buffer[9]) / 131.0f;
    data->gy = (int16_t)((buffer[10] << 8) | buffer[11]) / 131.0f;
    data->gz = (int16_t)((buffer[12] << 8) | buffer[13]) / 131.0f;

    return ESP_OK;
}

// 輔助函數：讀取寄存器
esp_err_t mpu6050_read_reg(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_transmit_receive(i2c_dev_handle, 
                                     &reg_addr, 1,  // 寫入寄存器地址
                                     data, len,     // 讀取數據
                                     -1);           // 無超時
}

// 輔助函數：寫入寄存器
esp_err_t mpu6050_write_reg(uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};
    return i2c_master_transmit(i2c_dev_handle, write_buf, sizeof(write_buf), -1);
}





int mpu6050_i2c_scan(int scl_io, int sda_io, i2c_port_num_t i2c_num)
{
    int devices_found = 0;
    esp_err_t ret;

    // 創建臨時的 I2C 總線配置
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_NUM,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    // 創建臨時的 I2C 總線句柄
    i2c_master_bus_handle_t bus_handle;
    ret = i2c_new_master_bus(&i2c_bus_config, &bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C 主機匯流排初始化失敗: %s", esp_err_to_name(ret));
        return -1;
    }

    printf("\n     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    printf("00:");

    // 掃描地址範圍 0x00-0x7F
    for (uint8_t addr = 0; addr < 128; addr++) {
        if (addr % 16 == 0 && addr != 0) {
            printf("\n%02x:", addr);
        }

        // 配置設備
        i2c_device_config_t dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = addr,
            .scl_speed_hz = 400000,  // 400 KHz
        };

        // 創建臨時設備句柄
        i2c_master_dev_handle_t dev_handle;
        ret = i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle);
        
        if (ret != ESP_OK) {
            printf(" --");
            continue;
        }

        // 嘗試與設備通訊
        uint8_t dummy;
        ret = i2c_master_transmit_receive(dev_handle, 
                                        NULL, 0,     // 不發送數據
                                        &dummy, 1,   // 嘗試讀取一個字節
                                        -1);        // 默認超時

        // 移除設備
        i2c_master_bus_rm_device(dev_handle);

        if (ret == ESP_OK) {
            printf(" %02x", addr);
            devices_found++;
        } else {
            printf(" --");
        }
    }
    printf("\n");

    // 清理：刪除 I2C 總線
    i2c_del_master_bus(bus_handle);

    return devices_found;
}
