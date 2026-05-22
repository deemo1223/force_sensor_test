# force_sensor_test

独立的力传感器读取工程，从 `ct_rl_pkg` 拆离出来。目标是最小依赖、可单独编译运行，同时复用原项目的协议解析代码。

## 项目结构

```
src/
├── main.cpp                    # 简洁的主程序（参数解析、启动/停止、数据输出）
├── force_sensor/
│   ├── ForceSensorReader.h     # 传感器读取控制器（核心逻辑）
│   ├── ForceSensorReader.cpp
│   ├── PosixSerialPort.h       # POSIX 串口操作层
│   └── PosixSerialPort.cpp
└── sri/                        # 协议解析层（复用自 ct_rl_pkg）
    ├── sriCommParser.*
    ├── sriCommCircularBuffer.*
    ├── sriCommATParser.*
    ├── sriCommM8218Parser.*
    └── dataStructForce.h
```

## 复用来源

协议解析代码来自 `ct_rl_pkg`：

- `src/hardware/sriRDSerialDemo/sriCommParser.*`
- `src/hardware/sriRDSerialDemo/sriCommCircularBuffer.*`
- `src/hardware/sriRDSerialDemo/sriCommATParser.*`
- `src/hardware/sriRDSerialDemo/sriCommM8218Parser.*`
- `include/ct_rl_pkg/hardware/dataStructForce.h`

## 工作流程

1. **主程序** (`src/main.cpp`)
   - 解析命令行参数（串口设备、波特率）
   - 创建并启动 `ForceSensorReader`
   - 定时轮询最新数据，格式化输出
   - 处理 Ctrl+C 信号优雅关闭

2. **传感器控制层** (`src/force_sensor/ForceSensorReader`)
   - 通过 AT 指令配置传感器（停止、采样率、校验模式）
   - 启动后台读线程接收数据
   - 管理 ACK 应答和数据解析回调
   - 对外提供线程安全的 `latestReading()` 接口

3. **串口操作层** (`src/force_sensor/PosixSerialPort`)
   - 打开/关闭 POSIX 串口
   - 配置 termios 参数（无缓冲、非阻塞 I/O）
   - 提供 `readSome()` 和 `writeString()` 方法

4. **协议解析** (`src/sri/*`)
   - `CSRICommATParser`：解析 AT 指令应答
   - `CSRICommM8218Parser`：解析六维力/力矩数据帧
   - 通过回调函数将结果传回 `ForceSensorReader`

## 坐标系与置零

- 前 9 帧数据用于记录偏置（offset）
- 从第 10 帧开始输出去零后的测量值

坐标映射（保持与原项目一致）：

- `F[0] = fz - offset_z`
- `F[1] = -fy - offset_y`
- `F[2] = fx - offset_x`
- `M[0] = mz - offset_mz`
- `M[1] = -my - offset_my`
- `M[2] = -mx - offset_mx`

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
