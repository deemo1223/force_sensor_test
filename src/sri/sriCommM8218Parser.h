#ifndef SRI_COMM_M8218_PARSER_H
#define SRI_COMM_M8218_PARSER_H

#include "sriCommCircularBuffer.h"
#include "sriCommParser.h"

class CSRICommM8218Parser : public CSRICommParser {
public:
  CSRICommM8218Parser();
  ~CSRICommM8218Parser() override;

  bool SetM8218CallbackFunction(
      SRICommM8218CallbackFunction m8218CallbackFunction);

  bool OnReceivedData(BYTE* data, int dataLen) override;
  bool OnNetworkFailure(std::string infor) override;

private:
  bool ParseDataFromBuffer(int& delLen, float& fx, float& fy, float& fz,
                           float& mx, float& my, float& mz);
  int ParseGetHeadIndex(BYTE* data, int dataLen);

  CSRICommCircularBuffer mCircularBuffer;
  SRICommM8218CallbackFunction mM8218CallbackFunction;
};

#endif
