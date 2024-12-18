# Teleplot 數據格式指南 📊

## 1. 基本數值格式 📈

### 單一數值
```
>名稱:數值
```
例如：
- `>temperature:25.5`
- `>humidity:80`
- `>pressure:1013`

### 帶單位數值
```
>名稱:數值§單位
```
例如：
- `>speed:30§km/h`
- `>voltage:3.3§V`
- `>distance:100§m`

## 2. 多數據格式 📊

### 同一圖表多數據（分號分隔）
```
>名稱:值1;值2;值3
```
例如：
- `>acceleration:1.1;-0.5;9.8`
- `>rgb:255;128;0`

## 3. 時間序列格式 ⏱️

### 帶時間戳的數據
```
>名稱:時間戳:數值
```
例如：
- `>temp:1634567890:25.5`
- `>pressure:1634567890:1013`

## 4. XY 圖表格式 📉

### 二維座標數據
```
>名稱:x值:y值|xy
```
例如：
- `>position:10.5:20.3|xy`
- `>trajectory:0:100|xy`

## 5. 文字訊息格式 📝

### 狀態文字
```
>名稱:文字內容|t
```
例如：
- `>status:Running|t`
- `>error:Sensor Error|t`
- `>info:System OK|t`

## 6. 特殊標記 🏷️

### 控制標記
- `|clr` - 清除之前數據
- `|np` - 不繪製數據點
- `|t` - 文字模式

例如：
- `>data:100|clr`
- `>temp:25.5|np`

## 7. 組合格式 🔄

### 多重數據範例
```
>angles:roll;pitch;yaw
>position:x:y|xy
>status:正常運行中|t
```

## 注意事項 ⚠️

1. 名稱規則：
   - 避免使用特殊字符
   - 保持簡短明確
   - 區分大小寫

2. 數值規則：
   - 可使用整數或浮點數
   - 科學記號支援
   - 注意數值範圍

3. 時間戳規則：
   - 使用毫秒級時間戳
   - 需為遞增值

4. 分隔符使用：
   - `:` 分隔主要元素
   - `;` 分隔多數據
   - `|` 標記特殊功能
   - `§` 分隔單位

5. 更新頻率：
   - 建議 ≤ 100Hz
   - 避免數據堵塞