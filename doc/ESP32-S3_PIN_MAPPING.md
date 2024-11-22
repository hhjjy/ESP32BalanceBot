# ESP32-S3 開發板腳位功能對照表

## 硬體資訊
- 開發板型號：ESP32-S3-WROOM-1
- 最後更新日期：2024-11-23

## 當前專案使用設備
1. MPU6050 傳感器 (I2C模式，地址：0x68)
   - SCL: GPIO0
   - SDA: GPIO1
   - VCC: 3.3V
   - GND: GND

2. OLED 顯示器 (I2C模式，地址：0x3C)
   - SCL: GPIO0 (與 MPU6050 共用)
   - SDA: GPIO1 (與 MPU6050 共用)
   - VCC: 3.3V
   - GND: GND

## 左側腳位 (由上至下)

| GPIO編號 | 基本功能 | RTC功能 | 特殊功能 | 使用狀態 | 備註 |
|---------|---------|---------|----------|---------|------|
| GND     | 電源    | -       | 接地      | 🔒 已使用   | 必要連接 |
| 3.3V    | 電源    | -       | 3.3V電源  | 🔒 已使用   | 必要連接 |
| RST     | 系統    | -       | 重置      | 🔒 已使用   | 系統重置 |
| IO4     | GPIO    | RTC_GPIO4 | TOUCH4/ADC1_CH3 | ⚪ 未使用 | 支援觸控/ADC |
| IO5     | GPIO    | RTC_GPIO5 | TOUCH5/ADC1_CH4 | ⚪ 未使用 | 支援觸控/ADC |
| IO6     | GPIO    | RTC_GPIO6 | TOUCH6/ADC1_CH5 | ⚪ 未使用 | 支援觸控/ADC |
| IO7     | GPIO    | RTC_GPIO7 | TOUCH7/ADC1_CH6 | ⚪ 未使用 | 支援觸控/ADC |
| IO15    | GPIO    | RTC_GPIO15 | U0RTS | ⚪ 未使用   | UART控制 |
| IO16    | GPIO    | RTC_GPIO16 | U0CTS | ⚪ 未使用   | UART控制 |
| IO17    | GPIO    | RTC_GPIO17 | U1TXD | ⚪ 未使用   | UART發送 |
| IO18    | GPIO    | RTC_GPIO18 | U1RXD | ⚪ 未使用   | UART接收 |
| IO8     | GPIO    | RTC_GPIO8 | TOUCH8/ADC1_CH7 | ⚪ 未使用 | 支援觸控/ADC |
| IO19    | GPIO    | RTC_GPIO19 | USB_D- | ⚠️ 保留 | USB差分信號 |
| IO20    | GPIO    | RTC_GPIO20 | USB_D+ | ⚠️ 保留 | USB差分信號 |
| IO3     | GPIO    | RTC_GPIO3 | TOUCH3/ADC1_CH2 | ⚪ 未使用 | 支援觸控/ADC |
| IO46    | GPIO    | -       | SPIWP    | ⚪ 未使用   | SPI通訊 |
| IO9     | GPIO    | RTC_GPIO9 | TOUCH9/ADC1_CH8 | ⚪ 未使用 | 支援觸控/ADC |
| IO10    | GPIO    | RTC_GPIO10 | TOUCH10/ADC1_CH9 | ⚪ 未使用 | 支援觸控/ADC |
| IO11    | GPIO    | RTC_GPIO11 | TOUCH11/ADC2_CH0 | ⚪ 未使用 | 支援觸控/ADC |
| IO12    | GPIO    | RTC_GPIO12 | TOUCH12/ADC2_CH1 | ⚪ 未使用 | 支援觸控/ADC |
| 3.3V    | 電源    | -       | 3.3V電源  | 🔒 已使用   | 必要連接 |

## 右側腳位 (由上至下)

| GPIO編號 | 基本功能 | RTC功能 | 特殊功能 | 使用狀態 | 備註 |
|---------|---------|---------|----------|---------|------|
| GND     | 電源    | -       | 接地      | 🔒 已使用   | 必要連接 |
| IO1     | GPIO    | RTC_GPIO1 | sar_i2c_sda_0 | ✅ 已使用 | I2C SDA (MPU6050/OLED) |
| IO2     | GPIO    | RTC_GPIO2 | TOUCH2/ADC1_CH1 | ⚪ 未使用 | 支援觸控/ADC |
| TXD     | UART    | -       | 串口發送   | ⚠️ 保留   | 預設UART0 |
| RXD     | UART    | -       | 串口接收   | ⚠️ 保留   | 預設UART0 |
| IO42    | GPIO    | -       | MTMS      | ⚪ 未使用   | - |
| IO41    | GPIO    | -       | MTDI      | ⚪ 未使用   | - |
| IO40    | GPIO    | -       | MISO      | ⚪ 未使用   | SPI通訊 |
| IO39    | GPIO    | -       | MOSI      | ⚪ 未使用   | - |
| IO38    | GPIO    | -       | CSO       | ⚪ 未使用   | - |
| IO37    | GPIO    | -       | SPIHD     | ⚪ 未使用   | SPI通訊 |
| IO36    | GPIO    | -       | SPICLK    | ⚪ 未使用   | - |
| IO35    | GPIO    | -       | SPID      | ⚪ 未使用   | SPI數據 |
| IO0     | GPIO    | RTC_GPIO0 | sar_i2c_scl_0 | ✅ 已使用 | I2C SCL (MPU6050/OLED) |
| IO45    | GPIO    | -       | SPIWP     | ⚪ 未使用   | SPI通訊 |
| IO48    | GPIO    | -       | SPICLK_N  | ⚪ 未使用   | SPI差分時鐘 |
| IO47    | GPIO    | -       | SPICLK_P  | ⚪ 未使用   | SPI差分時鐘 |
| IO21    | GPIO    | RTC_GPIO21 | - | ⚪ 未使用   | - |
| IO14    | GPIO    | RTC_GPIO14 | XTAL_32K_P | ⚠️ 保留 | 32KHz晶振 |
| IO13    | GPIO    | RTC_GPIO13 | XTAL_32K_N | ⚠️ 保留 | 32KHz晶振 |
| 5V      | 電源    | -       | 5V輸入    | 🔒 已使用   | USB供電 |

## I2C專用腳位對照表

| GPIO編號 | RTC GPIO | I2C功能 | 備註 |
|---------|----------|---------|------|
| GPIO0   | RTC_GPIO0 | sar_i2c_scl_0 | ✅ I2C時鐘線 (MPU6050/OLED) |
| GPIO1   | RTC_GPIO1 | sar_i2c_sda_0 | ✅ I2C數據線 (MPU6050/OLED) |
| GPIO2   | RTC_GPIO2 | sar_i2c_scl_1 | ⚪ I2C時鐘線(備用) |
| GPIO3   | RTC_GPIO3 | sar_i2c_sda_1 | ⚪ I2C數據線(備用) |

## I2C設備地址對照表
| 設備名稱 | I2C地址 | 腳位 |
|---------|---------|------|
| MPU6050 | 0x68    | SCL: GPIO0, SDA: GPIO1 |
| OLED    | 0x3C    | SCL: GPIO0, SDA: GPIO1 |

## 功能說明
1. RTC功能：在深度睡眠模式下仍可工作的GPIO
2. ADC：類比數位轉換器輸入
3. TOUCH：電容式觸控感測
4. UART：串列通訊介面
5. SPI：串列外設介面
6. JTAG：除錯介面
7. I2C：支援兩組I2C介面
   - 主要I2C：SCL(GPIO0), SDA(GPIO1)
   - 備用I2C：SCL(GPIO2), SDA(GPIO3)

## 使用狀態圖例
- ✅ 已分配使用
- ⚪ 未使用
- ⚠️ 保留/系統用
- 🔒 必要連接
- ⚡ 電源相關

## 注意事項
1. GPIO0 為 Strapping Pin，開機時需注意不要被拉低
2. USB 相關腳位 (GPIO19/20) 建議保留不使用
3. UART0 (TXD/RXD) 為預設下載及除錯埠，建議保留
4. 32KHz 晶振腳位 (GPIO13/14) 如果使用外部晶振需保留
5. 使用 ADC 時注意通道限制和電壓範圍
6. I2C 設備注意位址不要衝突：
   - MPU6050: 0x68
   - OLED: 0x3C
7. 建議在 SCL(GPIO0) 和 SDA(GPIO1) 各接一個 4.7kΩ 上拉電阻到 3.3V
8. I2C 通訊速率建議設定在 400kHz 以下，以確保穩定性

## 更新記錄
- 2024-11-20 創建文檔
- 2024-11-20 更新腳位分配：分配 GPIO0/GPIO1 用於 MPU6050 的 I2C 通訊
- 2024-11-23 更新配置：將 OLED 改為 I2C 模式，與 MPU6050 共用 I2C 匯流排