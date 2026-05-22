#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

#include "sri/dataStructForce.h"
#include "sri/sriCommATParser.h"
#include "sri/sriCommM8218Parser.h"
#include "force_sensor/PosixSerialPort.h"

class ForceSensorReader {
public:
  ForceSensorReader();
  ~ForceSensorReader();

  bool start(const std::string& device, int baudrate);
  void stop();
  bool isRunning() const noexcept;
  std::optional<forcesensor_data> latestReading() const noexcept;

private:
  bool sendCommand(const std::string& command, const std::string& value = "");
  bool onAck(std::string ack);
  bool onForceFrame(float fx, float fy, float fz, float mx, float my, float mz);
  void readLoop();

  PosixSerialPort serial_;
  CSRICommATParser at_parser_;
  CSRICommM8218Parser data_parser_;
  std::thread read_thread_;
  mutable std::mutex state_mutex_;
  std::mutex ack_mutex_;
  std::condition_variable ack_cv_;
  std::optional<std::string> last_ack_;
  std::optional<forcesensor_data> current_reading_;
  bool running_ = false;
  int frame_count_ = 0;
  float ft_offset_data_[6] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
};
