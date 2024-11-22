#include "mpu6050.h"

static const char *TAG = "MPU6050";

// I2C 初始化
static esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) return err;

    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

esp_err_t mpu6050_init(void)
{
    // 初始化 I2C
    esp_err_t ret = i2c_master_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C init failed");
        return ret;
    }

    uint8_t cmd = 0;
    i2c_cmd_handle_t cmd_handle;

    // 檢查設備 ID
    mpu6050_who_am_i(); 

    // 喚醒 MPU6050 單比特寫入 S -> AD/W -> Function -> [data] -> P 
    cmd_handle = i2c_cmd_link_create(); // 創建一個CMD儲存命令
    i2c_master_start(cmd_handle); // S
    i2c_master_write_byte(cmd_handle, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true); // A/D + W 
    i2c_master_write_byte(cmd_handle, MPU6050_PWR_MGMT_1, true); // Function : Reg 
    i2c_master_write_byte(cmd_handle, 0x00, true); // Data 
    i2c_master_stop(cmd_handle);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd_handle, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd_handle);

    ESP_LOGI(TAG, "MPU6050 initialized successfully");
    return ret;
}
// ACCEL 0x3B - 0x40
// TEMP 0x41 - 0x42
// GYRO 0x43 -0x48 
// => 連續讀取 14byte 
esp_err_t mpu6050_read_data(mpu6050_data_t *data)
{
    uint8_t buffer[14];
    i2c_cmd_handle_t cmd_handle;
    esp_err_t ret ; 
    cmd_handle = i2c_cmd_link_create() ; 
    // 連續讀取多字節 S ->  AD/W (MPU6050_ADDR << 1) | I2C_MASTER_WRITE -> function (0x38) 
    //              ->S -> AD/R (MPU6050<<1) | I2C_Master_READ -> Data 1 -> Data2 -> .... Data 14(NACK) -> P
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd_handle,0x3B,true) ; 
    i2c_master_start(cmd_handle) ; 
    i2c_master_write_byte(cmd_handle, (MPU6050_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd_handle,&buffer,14,I2C_MASTER_LAST_NACK) ;
    i2c_master_stop(cmd_handle) ; 
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM,cmd_handle,pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS)) ; 

    if (ret != ESP_OK){
        ESP_LOGE(TAG, "讀取感應器數據失敗! 錯誤碼:%2d",ret);
        return ret; 
    }
    // 賦值 2^16 = 65536
    // data 範圍 -32683 - 32683  (2g )
    data->ax = (int16_t)((buffer[0]<<8)| buffer[1])/ 16384.0f ; 
    data->ay = (int16_t)((buffer[2]<<8)| buffer[3])/ 16384.0f ; 
    data->az = (int16_t)((buffer[4]<<8)| buffer[5])/ 16384.0f ; 
    data->temp = (int16_t)((buffer[6]<<8)| buffer[7])/ 340.0f + 36.53f; 
    data->gx = (int16_t)((buffer[8]<<8)| buffer[9])/131.0f ; // degree
    data->gy = (int16_t)((buffer[10]<<8)| buffer[11])/131.0f ; 
    data->gz = (int16_t)((buffer[12]<<8)| buffer[13])/131.0f ; 

    return ESP_OK;
    // uint8_t data[14];
    // i2c_cmd_handle_t cmd_handle;

    // // 讀取所有數據
    // cmd_handle = i2c_cmd_link_create();
    // i2c_master_start(cmd_handle);
    // i2c_master_write_byte(cmd_handle, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
    // i2c_master_write_byte(cmd_handle, MPU6050_ACCEL_XOUT_H, true);
    // i2c_master_start(cmd_handle);
    // i2c_master_write_byte(cmd_handle, (MPU6050_ADDR << 1) | I2C_MASTER_READ, true);
    // i2c_master_read(cmd_handle, data, 14, I2C_MASTER_LAST_NACK);
    // i2c_master_stop(cmd_handle);
    // esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd_handle, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    // i2c_cmd_link_delete(cmd_handle);
    // if (ret != ESP_OK) return ret;

    // // 轉換數據
    // *ax = (int16_t)((data[0] << 8) | data[1]) / 16384.0f;
    // *ay = (int16_t)((data[2] << 8) | data[3]) / 16384.0f;
    // *az = (int16_t)((data[4] << 8) | data[5]) / 16384.0f;
    // *gx = (int16_t)((data[8] << 8) | data[9]) / 131.0f;
    // *gy = (int16_t)((data[10] << 8) | data[11]) / 131.0f;
    // *gz = (int16_t)((data[12] << 8) | data[13]) / 131.0f;

    // return ESP_OK;
}


esp_err_t mpu6050_who_am_i(void)
{
    esp_err_t ret;
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    uint8_t data;

    // Debug 輸出：開始讀取
    ESP_LOGI(TAG, "開始讀取 WHO_AM_I 寄存器");

    // 第一階段：寫入寄存器地址
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd_handle, MPU6050_WHO_AM_I, true);
    
    // 第二階段：讀取數據
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (MPU6050_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd_handle, &data, I2C_MASTER_NACK);
    i2c_master_stop(cmd_handle);

    // 執行命令
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd_handle, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd_handle);

    // Debug 輸出：I2C 通訊結果
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C 通訊錯誤: %d", ret);
        ESP_LOGE(TAG, "錯誤描述: %s", esp_err_to_name(ret));
        return ret;
    }

    // Debug 輸出：原始數據
    ESP_LOGI(TAG, "收到的原始數據: 0x%02X", data);

    // 驗證設備 ID
    if (data != 0x68) {
        ESP_LOGE(TAG, "設備 ID 不符: 預期 0x68, 實際 0x%02X", data);
        return ESP_ERR_NOT_FOUND;
    }

    ESP_LOGI(TAG, "成功確認 MPU6050 (WHO_AM_I = 0x%02X)", data);
    return ESP_OK;
}
