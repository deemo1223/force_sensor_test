#include "force_sensor/ForceSensorReader.h"

#include <cerrno>
#include <cstdio>
#include <chrono>
#include <iostream>
#include <vector>

ForceSensorReader::ForceSensorReader() {
  at_parser_.SetATCallbackFunction([this](std::string ack) {
    return onAck(std::move(ack));
  });
  data_parser_.SetM8218CallbackFunction(
      [this](float fx, float fy, float fz, float mx, float my, float mz) {
        return onForceFrame(fx, fy, fz, mx, my, mz);
      });
}

ForceSensorReader::~ForceSensorReader() {
  stop();
}

bool ForceSensorReader::start(const std::string& device, int baudrate) {
  if (running_) {
    return false;
  }

  if (!serial_.openPort(device, baudrate)) {
    return false;
  }

  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    running_ = true;
  }
  // reset zeroing state so we compute a fresh offset on each start
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    zeroing_done_ = false;
    zeroing_samples_ = 0;
    zeroing_sums_ = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    zeroing_start_time_ = std::chrono::steady_clock::now();
  }
  read_thread_ = std::thread([this]() { readLoop(); });

  // if (!sendCommand("GSD", "STOP")) {
  //   std::cerr << "failed to stop streaming before configuration\n";
  //   stop();
  //   return false;
  // }
  // if (!sendCommand("SGDM", "(A01,A02,A03,A04,A05,A06);C;1;(WMA:1)")) {
  //   std::cerr << "failed to set receive channel mode\n";
  //   stop();
  //   return false;
  // }
  // if (!sendCommand("SMPRM", "L")) {
  //   std::cerr << "failed to set sampling mode\n";
  //   stop();
  //   return false;
  // }
  // if (!sendCommand("SMPF", "300")) {
  //   std::cerr << "failed to set sample frequency\n";
  //   stop();
  //   return false;
  // }
  // if (!sendCommand("DCKMD", "SUM")) {
  //   std::cerr << "failed to set checksum mode\n";
  //   stop();
  //   return false;
  // }
  if (serial_.writeString("AT+GSD\r\n") < 0) {
    std::perror("write AT+GSD");
    stop();
    return false;
  }

  return true;
}

void ForceSensorReader::stop() {
  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (!running_) {
      return;
    }
    running_ = false;
  }

  sendCommand("GSD", "STOP");

  if (read_thread_.joinable()) {
    read_thread_.join();
  }
  serial_.closePort();
}

bool ForceSensorReader::isRunning() const noexcept {
  std::lock_guard<std::mutex> lock(state_mutex_);
  return running_;
}

std::optional<forcesensor_data> ForceSensorReader::latestReading() const noexcept {
  std::lock_guard<std::mutex> lock(state_mutex_);
  return current_reading_;
}

bool ForceSensorReader::sendCommand(const std::string& command, const std::string& value) {
  {
    std::lock_guard<std::mutex> lock(ack_mutex_);
    last_ack_.reset();
  }

  const std::string packet = value.empty()
      ? "AT+" + command + "\r\n"
      : "AT+" + command + "=" + value + "\r\n";
  if (serial_.writeString(packet) < 0) {
    std::perror(("write " + command).c_str());
    return false;
  }

  std::unique_lock<std::mutex> lock(ack_mutex_);
  const bool received = ack_cv_.wait_for(lock, std::chrono::seconds(2), [this, &command]() {
    return last_ack_.has_value() && last_ack_->rfind("ACK+" + command, 0) == 0;
  });

  if (!received) {
    return false;
  }

  std::cout << *last_ack_ << '\n';
  return true;
}

bool ForceSensorReader::onAck(std::string ack) {
  std::lock_guard<std::mutex> lock(ack_mutex_);
  last_ack_ = std::move(ack);
  ack_cv_.notify_all();
  return true;
}

bool ForceSensorReader::onForceFrame(float fx, float fy, float fz, float mx, float my, float mz) {
  // map incoming values to stored channel ordering
  double vals[6];
  vals[0] = fz;
  vals[1] = -fy;
  vals[2] = fx;
  vals[3] = mz;
  vals[4] = -my;
  vals[5] = -mx;

  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (!zeroing_done_) {
      if (zeroing_samples_ == 0) {
        zeroing_start_time_ = std::chrono::steady_clock::now();
      }
      for (int i = 0; i < 6; ++i) {
        zeroing_sums_[i] += vals[i];
      }
      ++zeroing_samples_;

      const float elapsed = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::steady_clock::now() - zeroing_start_time_).count();
      if (elapsed >= zeroing_duration_seconds_) {
        for (int i = 0; i < 6; ++i) {
          ft_offset_data_[i] = static_cast<float>(zeroing_sums_[i] / static_cast<double>(zeroing_samples_));
        }
        zeroing_done_ = true;
        std::cout << "Zero offset computed: ";
        for (int i = 0; i < 6; ++i) {
          std::cout << ft_offset_data_[i] << (i + 1 < 6 ? ", " : "\n");
        }
      }
      return true;
    }

    forcesensor_data reading{};
    reading.F[0] = static_cast<float>(vals[0] - ft_offset_data_[0]);
    reading.F[1] = static_cast<float>(vals[1] - ft_offset_data_[1]);
    reading.F[2] = static_cast<float>(vals[2] - ft_offset_data_[2]);
    reading.M[0] = static_cast<float>(vals[3] - ft_offset_data_[3]);
    reading.M[1] = static_cast<float>(vals[4] - ft_offset_data_[4]);
    reading.M[2] = static_cast<float>(vals[5] - ft_offset_data_[5]);

    current_reading_ = reading;
  }

  return true;
}

void ForceSensorReader::readLoop() {
  std::vector<uint8_t> buffer(8192);
  while (isRunning()) {
    const ssize_t n = serial_.readSome(buffer.data(), buffer.size());
    if (n > 0) {
      at_parser_.OnReceivedData(buffer.data(), static_cast<int>(n));
      data_parser_.OnReceivedData(buffer.data(), static_cast<int>(n));
    } else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
      std::perror("serial read");
      std::lock_guard<std::mutex> lock(state_mutex_);
      running_ = false;
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}
