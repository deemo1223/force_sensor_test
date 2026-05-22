#include "sriCommParser.h"

CSRICommParser::CSRICommParser() = default;

CSRICommParser::~CSRICommParser() = default;

bool CSRICommParser::OnReceivedData(BYTE* data, int dataLen) {
  if (data == nullptr) {
    return false;
  }
  if (dataLen <= 0) {
    return false;
  }
  return true;
}

bool CSRICommParser::OnNetworkFailure(std::string) {
  return true;
}
