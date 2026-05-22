#include "force_sensor/PosixSerialPort.h"

#include <cerrno>
#include <fcntl.h>
#include <iostream>
#include <termios.h>
#include <unistd.h>

PosixSerialPort::~PosixSerialPort() {
  closePort();
}

bool PosixSerialPort::openPort(const std::string& device, int baudrate) {
  fd_ = open(device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd_ < 0) {
    std::perror(("open " + device).c_str());
    return false;
  }

  termios tty{};
  if (tcgetattr(fd_, &tty) != 0) {
    std::perror("tcgetattr");
    closePort();
    return false;
  }

  cfmakeraw(&tty);
  tty.c_cflag |= CLOCAL | CREAD;
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;
  tty.c_cflag &= ~PARENB;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CRTSCTS;
  tty.c_cc[VTIME] = 1;
  tty.c_cc[VMIN] = 0;

  const speed_t speed = toBaud(baudrate);
  cfsetispeed(&tty, speed);
  cfsetospeed(&tty, speed);

  if (tcsetattr(fd_, TCSANOW, &tty) != 0) {
    std::perror("tcsetattr");
    closePort();
    return false;
  }

  tcflush(fd_, TCIOFLUSH);
  return true;
}

ssize_t PosixSerialPort::writeString(const std::string& data) {
  if (fd_ < 0) {
    return -1;
  }
  return write(fd_, data.data(), data.size());
}

ssize_t PosixSerialPort::readSome(uint8_t* buffer, size_t size) {
  if (fd_ < 0) {
    return -1;
  }
  return read(fd_, buffer, size);
}

void PosixSerialPort::closePort() {
  if (fd_ >= 0) {
    close(fd_);
    fd_ = -1;
  }
}

speed_t PosixSerialPort::toBaud(int baudrate) {
  switch (baudrate) {
    case 2400: return B2400;
    case 4800: return B4800;
    case 9600: return B9600;
    case 19200: return B19200;
    case 38400: return B38400;
    case 57600: return B57600;
    case 115200: return B115200;
    default: return B115200;
  }
}
