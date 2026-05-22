#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <sys/types.h>
#include <termios.h>

class PosixSerialPort {
public:
  ~PosixSerialPort();

  bool openPort(const std::string& device, int baudrate);
  ssize_t writeString(const std::string& data);
  ssize_t readSome(uint8_t* buffer, size_t size);
  void closePort();

private:
  static speed_t toBaud(int baudrate);

  int fd_ = -1;
};
