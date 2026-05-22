#include "sriCommCircularBuffer.h"

#include <cstring>

CSRICommCircularBuffer::CSRICommCircularBuffer()
    : mBuffer(nullptr),
      mBufferSize(0),
      mWIndex(0),
      mRIndex(0),
      mTotalWCount(0),
      mTotalRCount(0) {}

CSRICommCircularBuffer::~CSRICommCircularBuffer() {
  delete[] mBuffer;
}

bool CSRICommCircularBuffer::Init(int bufferMaxSize) {
  if (bufferMaxSize <= 0) {
    return false;
  }
  delete[] mBuffer;
  mBuffer = new BYTE[bufferMaxSize];
  mBufferSize = bufferMaxSize;
  mWIndex = 0;
  mRIndex = 0;
  mTotalWCount = 0;
  mTotalRCount = 0;
  return true;
}

int CSRICommCircularBuffer::GetLength() {
  int wIndex = 0;
  int rIndex = 0;
  return GetLength(wIndex, rIndex);
}

int CSRICommCircularBuffer::GetLength(int& wIndex, int& rIndex) {
  wIndex = mWIndex;
  rIndex = mRIndex;

  if (wIndex == rIndex) {
    return 0;
  }
  if (wIndex > rIndex) {
    return wIndex - rIndex;
  }
  return mBufferSize - rIndex + wIndex;
}

bool CSRICommCircularBuffer::Clear() {
  mWIndex = 0;
  mRIndex = 0;
  return true;
}

bool CSRICommCircularBuffer::Clear(int clearLen) {
  if (clearLen < 0) {
    return false;
  }
  if (clearLen == 0) {
    return true;
  }

  int wIndex = 0;
  int rIndex = 0;
  const int length = GetLength(wIndex, rIndex);
  if (clearLen > length) {
    return Clear();
  }

  mRIndex = (rIndex + clearLen) % mBufferSize;
  return true;
}

int CSRICommCircularBuffer::Write(BYTE* data, int dataLen) {
  int len = dataLen;
  if (len <= 0) {
    return 0;
  }
  if (len > mBufferSize) {
    len = mBufferSize;
  }

  int wIndex = 0;
  int rIndex = 0;
  const int length = GetLength(wIndex, rIndex);
  const int spaceLen = mBufferSize - length - 1;
  if (len > spaceLen) {
    return 0;
  }

  if (wIndex + len <= mBufferSize) {
    std::memcpy(mBuffer + wIndex, data, len);
  } else {
    const int firstBlockLen = mBufferSize - wIndex;
    const int secondBlockLen = len - firstBlockLen;
    std::memcpy(mBuffer + wIndex, data, firstBlockLen);
    std::memcpy(mBuffer, data + firstBlockLen, secondBlockLen);
  }

  mWIndex = (wIndex + len) % mBufferSize;
  mTotalWCount += len;
  return len;
}

int CSRICommCircularBuffer::Write(BYTE data) {
  int wIndex = 0;
  int rIndex = 0;
  const int length = GetLength(wIndex, rIndex);
  const int spaceLen = mBufferSize - length - 1;
  if (spaceLen < 1) {
    return 0;
  }

  mBuffer[wIndex] = data;
  mWIndex = (wIndex + 1) % mBufferSize;
  mTotalWCount += 1;
  return 1;
}

BYTE* CSRICommCircularBuffer::Read(int& dataLen, int readLen, bool delData) {
  int wIndex = 0;
  int rIndex = 0;
  const int length = GetLength(wIndex, rIndex);
  const int len = (readLen <= 0) ? length : std::min(length, readLen);
  if (len <= 0) {
    dataLen = 0;
    return nullptr;
  }

  BYTE* data = new BYTE[len];
  dataLen = len;

  if (rIndex + len <= mBufferSize) {
    std::memcpy(data, mBuffer + rIndex, len);
  } else {
    const int firstBlockLen = mBufferSize - rIndex;
    const int secondBlockLen = len - firstBlockLen;
    std::memcpy(data, mBuffer + rIndex, firstBlockLen);
    std::memcpy(data + firstBlockLen, mBuffer, secondBlockLen);
  }

  if (delData) {
    mRIndex = (rIndex + len) % mBufferSize;
  }

  mTotalRCount += len;
  return data;
}

BYTE* CSRICommCircularBuffer::ReadTry(int& dataLen, int readLen) {
  return Read(dataLen, readLen, false);
}
