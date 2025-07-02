# DHT11错误修正说明

## 问题分析
DHT11显示error的原因主要有以下几点：

### 1. 读取时间间隔问题
**原问题**：原代码中DHT11在主循环中每次都尝试读取，但DHT11需要至少2秒的间隔时间。

**解决方案**：
```c
void Data_Collection(void)
{
    static uint32_t last_dht11_read = 0;
    
    // DHT11需要2秒间隔，每2秒读取一次
    if(system_tick - last_dht11_read >= 2000)
    {
        last_dht11_read = system_tick;
        
        if(dht11_read_dat(&sensor_data.temperature, &sensor_data.humidity) == 0)
        {
            sensor_data.dht11_status = 1; // 读取成功
        }
        else
        {
            sensor_data.dht11_status = 0; // 读取失败
            sensor_data.error_count++;
        }
    }
}
```

### 2. 初始化测试
**新增功能**：在系统初始化时增加DHT11的测试读取，确保传感器正常工作。

```c
// 测试DHT11读取
lcd_print_str(1, 0, "Test DHT11...");
Mdelay_Lib(1000); // DHT11需要启动时间

uint8_t test_temp, test_humi;
if(dht11_read_dat(&test_temp, &test_humi) == 0)
{
    sensor_data.dht11_status = 1;
    init_status |= 0x02; // 设置DHT11初始化成功标志
    sprintf(str, "DHT11 OK: %d,%d", test_temp, test_humi);
    lcd_print_str(1, 0, str);
}
else
{
    sensor_data.dht11_status = 0;
    lcd_print_str(1, 0, "DHT11 Failed");
    sensor_data.error_count++;
}
```

### 3. 时钟同步优化
**原问题**：system_tick每次增加1，但延时是50ms，导致时钟不准确。

**解决方案**：
```c
// 主循环延时调整为1ms，与system_tick保持一致
Mdelay_Lib(1);  // 1ms延时
system_tick++;  // 时钟增加1ms
```

## DHT11正确使用方式

### 参考DHT11_main.c的经验：
1. **读取间隔**：至少2秒间隔
2. **返回值判断**：0表示成功，非0表示失败
3. **参数类型**：使用`unsigned char`类型指针
4. **启动时间**：系统启动后需要给DHT11一些时间稳定

### 修正后的数据采集逻辑：
```c
// DHT11: 2秒间隔读取
// 光照/烟雾/MPU6050: 500ms间隔读取
// 这样既保证了DHT11的稳定性，又保证了其他传感器的实时性
```

## 预期效果

修正后，DHT11应该能够：
1. **初始化时显示**：`DHT11 OK: 25,60`（示例数值）
2. **运行时正常显示**：温湿度页面显示实际温湿度值而不是Error
3. **状态指示**：init_status的bit1被设置，表示DHT11工作正常

## 硬件检查建议

如果修正后仍显示error，请检查：
1. **接线**：DHT11的数据线是否正确连接到PE5
2. **供电**：DHT11的VCC和GND是否正确连接
3. **上拉电阻**：数据线是否有4.7K上拉电阻
4. **传感器状态**：DHT11传感器本身是否损坏

## 调试方法

可以在Data_Collection函数中添加调试信息：
```c
// 调试代码（临时使用）
if(system_tick % 3000 == 0) // 每3秒打印一次调试信息
{
    char debug[32];
    sprintf(debug, "DHT11 Try: %d", system_tick/1000);
    // 可以通过其他方式输出调试信息
}
```

这样就能解决DHT11显示error的问题了！
