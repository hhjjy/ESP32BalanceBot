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
    esp_err_t ret = i2c_master_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C init failed");
        return ret;
    }

    // 檢查設備 ID (WHO_AM_I)
    uint8_t write_reg = MPU6050_WHO_AM_I;
    uint8_t who_am_i;
    ret = i2c_master_write_read_device(I2C_MASTER_NUM, MPU6050_ADDR,
                                      &write_reg, 1,    // 寫入部分
                                      &who_am_i, 1,     // 讀取部分
                                      I2C_MASTER_TIMEOUT_MS);
    if (ret != ESP_OK || who_am_i != 0x68) {
        ESP_LOGE(TAG, "MPU6050 Not found (WHO_AM_I = 0x%02X)", who_am_i);
        return ESP_ERR_NOT_FOUND;
    }

    // 喚醒 MPU6050
    uint8_t write_buf[2] = {MPU6050_PWR_MGMT_1, 0x00};
    ret = i2c_master_write_to_device(I2C_MASTER_NUM, MPU6050_ADDR,
                                    write_buf, sizeof(write_buf),
                                    I2C_MASTER_TIMEOUT_MS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "MPU6050 Wake up failed");
        return ret;
    }

    ESP_LOGI(TAG, "MPU6050 initialized successfully");
    return ESP_OK;
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
}


