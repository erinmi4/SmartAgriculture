# MQ-2 + DHT11 温湿度烟雾监测系统

## 功能实现

本系统成功集成了MQ-2烟雾传感器和DHT11温湿度传感器，实现了环境监测的完整功能。

## 显示方案

### LCD显示格式 (16x2)
```
第一行: S:XXXppm T:XX'C    (S=烟雾, T=温度)
第二行: STATUS H:XX%RH     (状态 + 湿度)
```

### 状态显示说明
- **OK**: 烟雾浓度 < 100 ppm (正常)
- **LOW**: 烟雾浓度 100-300 ppm (轻微)  
- **MED**: 烟雾浓度 300-500 ppm (中等)
- **HIGH!**: 烟雾浓度 > 500 ppm (危险)

## 传感器配置

### MQ-2烟雾传感器
- **接口**: USART3 (PB10-TX, PB11-RX)
- **波特率**: 9600
- **更新频率**: 2秒

### DHT11温湿度传感器  
- **接口**: PE5 (单总线)
- **测量范围**: 
  - 温度: 0-50°C (±2°C)
  - 湿度: 20-90%RH (±5%RH)
- **更新频率**: 2秒

## 主要功能

### 1. 实时监测
- 烟雾浓度实时检测和显示
- 温度湿度实时检测和显示
- 自动错误处理和状态提示

### 2. 紧凑显示
- 两个传感器数据同时显示在LCD上
- 缩写显示以节省空间
- 直观的状态指示

### 3. 系统稳定性
- 错误处理机制
- DHT11读取失败时显示"T:Er"
- 连续监测不中断

## 显示示例

### 正常状态
```
S: 45ppm T:23'C
OK    H:65%RH
```

### 警告状态
```
S:250ppm T:25'C  
LOW   H:70%RH
```

### 危险状态
```
S:600ppm T:28'C
HIGH! H:75%RH
```

### DHT11错误状态
```
S: 80ppm T:Er
Status: OK
```

## 代码特点

### 1. 集成设计
- MQ-2和DHT11功能都集成在main.c中
- 使用现有的delay.c函数
- 避免链接问题

### 2. 高效显示
- 16字符有效利用LCD空间
- 状态和数据同时显示
- 易于阅读的格式

### 3. 实时性
- 2秒更新一次显示
- 传感器数据实时获取
- 响应迅速

## 硬件连接

### MQ-2传感器
- VCC → 5V
- GND → GND  
- TX → PB11 (USART3_RX)
- RX → PB10 (USART3_TX)

### DHT11传感器
- VCC → 3.3V/5V
- GND → GND
- DATA → PE5

### LCD1602
- 按照lcd.h中的引脚定义连接

## 使用说明

1. **上电初始化**: 显示"Sensors Ready"
2. **正常运行**: 同时显示烟雾、温度、湿度数据
3. **异常处理**: DHT11读取失败时显示错误信息
4. **状态监控**: 根据烟雾浓度自动显示状态

## 扩展功能建议

1. **报警功能**: 添加蜂鸣器在危险状态时报警
2. **数据记录**: 保存历史数据
3. **阈值设置**: 可调整报警阈值
4. **无线传输**: 通过WiFi上传数据
5. **更多传感器**: 添加其他环境传感器

这个系统成功实现了多传感器环境监测，提供了完整的烟雾、温度、湿度监测解决方案！
