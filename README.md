# force_sensor_test

这个目录是从 `ct_rl_pkg` 中拆出来的独立力传感器读取工程，目标是最小依赖、可单独编译运行，同时尽量复用原项目里的协议解析代码。

## 复用来源

- `src/hardware/sriRDSerialDemo/sriCommParser.*`
- `src/hardware/sriRDSerialDemo/sriCommCircularBuffer.*`
- `src/hardware/sriRDSerialDemo/sriCommATParser.*`
- `src/hardware/sriRDSerialDemo/sriCommM8218Parser.*`
- `include/ct_rl_pkg/hardware/dataStructForce.h`

`force_sensor_test/src/sri` 中的源码是从上述文件最小化搬运过来的，保留了原始协议解析逻辑，并修正了独立运行时会暴露出来的内存管理问题。

## 项目内力传感器逻辑

当前项目的力传感器主链路是：

1. `src/main.cpp`
   通过 `serialSixFTSensor` 发送 `AT+GSD=STOP`、`AT+SMPR=400`、`AT+GSD` 启停和配置传感器
2. `src/hardware/protocolSeiral.cpp`
   启动串口接收流程
3. `src/hardware/sriRDSerialDemo/*`
   解析 `ACK` 和 `M8218` 连续数据帧
4. `src/hardware/sriRDSerialDemo/sriCommManager.cpp`
   把解析结果写入全局 `forcedata`
5. `include/ct_rl_pkg/hardware/dataStructForce.h`
   对外提供 `forcedata.F[3]`、`forcedata.M[3]` 等数据

其中当前主程序真正走的是 `CSRICommManager::OnCommM8218()` 这条逻辑，不是 `sixaxis_callback_handle()` 那套手工解包路径。

## 坐标映射与置零

独立工程保持了 `CSRICommManager::OnCommM8218()` 的行为：

- 前 9 帧用于记录 offset
- 从第 10 帧开始输出去零后的 6 维力/力矩

坐标映射保持和原项目一致：

- `forcedata.F[0] = fz - offset_z`
- `forcedata.F[1] = -fy - offset_y`
- `forcedata.F[2] = fx - offset_x`
- `forcedata.M[0] = mz - offset_mz`
- `forcedata.M[1] = -my - offset_my`
- `forcedata.M[2] = -mx - offset_mx`

## 编译

```bash
cd force_sensor_test
cmake -S . -B build
cmake --build build -j
```

## 运行

默认串口：

```bash
./build/force_sensor_test
```

指定串口：

```bash
./build/force_sensor_test /dev/ttyUSB1
```

按 `Ctrl+C` 退出。
