#include "sriCommM8218Parser.h"

#include <cstring>

CSRICommM8218Parser::CSRICommM8218Parser() : mM8218CallbackFunction(nullptr) {
  mCircularBuffer.Init(102400);
}

CSRICommM8218Parser::~CSRICommM8218Parser() = default;

bool CSRICommM8218Parser::SetM8218CallbackFunction(
    SRICommM8218CallbackFunction m8218CallbackFunction) {
  mM8218CallbackFunction = m8218CallbackFunction;
  return true;
}

bool CSRICommM8218Parser::OnReceivedData(BYTE* data, int dataLen) {
  if (data == nullptr || dataLen <= 0) {
    return false;
  }

  mCircularBuffer.Write(data, dataLen);

  int delLen = 0;
  float fx = 0.f;
  float fy = 0.f;
  float fz = 0.f;
  float mx = 0.f;
  float my = 0.f;
  float mz = 0.f;
  if (!ParseDataFromBuffer(delLen, fx, fy, fz, mx, my, mz)) {
    mCircularBuffer.Clear(delLen);
    return false;
  }

  mCircularBuffer.Clear(delLen);
  if (mM8218CallbackFunction != nullptr) {
    mM8218CallbackFunction(fx, fy, fz, mx, my, mz);
  }
  return true;
}

bool CSRICommM8218Parser::OnNetworkFailure(std::string) {
  return true;
}

bool CSRICommM8218Parser::ParseDataFromBuffer(int& delLen, float& fx, float& fy,
                                              float& fz, float& mx, float& my,
                                              float& mz) {
  delLen = 0;
  int dataLen = 0;
  BYTE* data = mCircularBuffer.ReadTry(dataLen);
  if (data == nullptr) {
    return false;
  }

  const int headIndex = ParseGetHeadIndex(data, dataLen);
  if (headIndex == -1) {
    delLen = dataLen - 1;
    delete[] data;
    return false;
  }

  if (headIndex + 31 > dataLen) {
    delLen = headIndex;
    delete[] data;
    return false;
  }

  int index = headIndex + 2;
  const int frameLen = data[index] * 256 + data[index + 1];
  if (frameLen != 27) {
    delLen = index;
    delete[] data;
    return false;
  }
  index += 2;
  index += 2;

  std::memcpy(&fx, data + index, 4);
  index += 4;
  std::memcpy(&fy, data + index, 4);
  index += 4;
  std::memcpy(&fz, data + index, 4);
  index += 4;
  std::memcpy(&mx, data + index, 4);
  index += 4;
  std::memcpy(&my, data + index, 4);
  index += 4;
  std::memcpy(&mz, data + index, 4);
  index += 4;

  const BYTE check = data[index];
  ++index;
  delLen = index;

  BYTE checkNew = 0x00;
  for (int i = headIndex + 6; i <= headIndex + 29; ++i) {
    checkNew = static_cast<BYTE>(checkNew + data[i]);
  }

  delete[] data;
  return checkNew == check;
}

int CSRICommM8218Parser::ParseGetHeadIndex(BYTE* data, int dataLen) {
  int headIndex = -1;
  int countHead = 0;
  for (int i = 0; i < dataLen - 1; ++i) {
    if (data[i] == 0xAA && data[i + 1] == 0x55) {
      headIndex = i;
      ++countHead;
    }
  }

  if (headIndex + 31 > dataLen && countHead > 1) {
    int reverseCount = 0;
    for (int i = 0; i < dataLen - 1; ++i) {
      if (data[i] == 0xAA && data[i + 1] == 0x55) {
        headIndex = i;
        ++reverseCount;
        if (reverseCount == countHead - 1) {
          return headIndex;
        }
      }
    }
  }
  return headIndex;
}
