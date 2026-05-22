#ifndef SRI_COMM_AT_PARSER_H
#define SRI_COMM_AT_PARSER_H

#include "sriCommCircularBuffer.h"
#include "sriCommParser.h"

class CSRICommATParser : public CSRICommParser {
public:
  CSRICommATParser();
  ~CSRICommATParser() override;

  bool SetATCallbackFunction(SRICommATCallbackFunction atCallbackFunction);

  bool OnReceivedData(BYTE* data, int dataLen) override;
  bool OnNetworkFailure(std::string infor) override;

private:
  bool ParseDataFromBuffer(int& delLen, std::string& ack);
  int ParseGetHeadIndex(BYTE* data, int dataLen);
  int ParseGetEndIndex(BYTE* data, int dataLen, int index);

  CSRICommCircularBuffer mCircularBuffer;
  SRICommATCallbackFunction mAtCallbackFunction;
};

#endif
