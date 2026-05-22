#include "sriCommATParser.h"

CSRICommATParser::CSRICommATParser() : mAtCallbackFunction(nullptr) {
  mCircularBuffer.Init(102400);
}

CSRICommATParser::~CSRICommATParser() = default;

bool CSRICommATParser::SetATCallbackFunction(
    SRICommATCallbackFunction atCallbackFunction) {
  mAtCallbackFunction = atCallbackFunction;
  return true;
}

bool CSRICommATParser::OnReceivedData(BYTE* data, int dataLen) {
  if (data == nullptr || dataLen <= 0) {
    return false;
  }

  mCircularBuffer.Write(data, dataLen);

  int delLen = 0;
  std::string ack;
  if (!ParseDataFromBuffer(delLen, ack)) {
    mCircularBuffer.Clear(delLen);
    return false;
  }

  mCircularBuffer.Clear(delLen);
  if (mAtCallbackFunction != nullptr) {
    mAtCallbackFunction(ack);
  }
  return true;
}

bool CSRICommATParser::OnNetworkFailure(std::string) {
  return true;
}

bool CSRICommATParser::ParseDataFromBuffer(int& delLen, std::string& ack) {
  int dataLen = 0;
  BYTE* data = mCircularBuffer.ReadTry(dataLen);
  if (data == nullptr) {
    return false;
  }

  if (dataLen < 4) {
    delLen = 0;
    ack.clear();
    delete[] data;
    return false;
  }

  const int headIndex = ParseGetHeadIndex(data, dataLen);
  if (headIndex == -1) {
    delLen = dataLen - 3;
    ack.clear();
    delete[] data;
    return false;
  }

  const int endIndex = ParseGetEndIndex(data, dataLen, headIndex + 4);
  if (endIndex == -1) {
    delLen = headIndex;
    ack.clear();
    delete[] data;
    return false;
  }

  const int len = endIndex - headIndex + 2;
  ack.assign(reinterpret_cast<char*>(data + headIndex), len);
  delLen = endIndex + 2;
  delete[] data;
  return true;
}

int CSRICommATParser::ParseGetHeadIndex(BYTE* data, int dataLen) {
  for (int i = 0; i < dataLen - 3; ++i) {
    if (data[i] == 'A' && data[i + 1] == 'C' && data[i + 2] == 'K' &&
        data[i + 3] == '+') {
      return i;
    }
  }
  return -1;
}

int CSRICommATParser::ParseGetEndIndex(BYTE* data, int dataLen, int index) {
  for (int i = index; i < dataLen - 1; ++i) {
    if (data[i] == 13 && data[i + 1] == 10) {
      return i;
    }
  }
  return -1;
}
