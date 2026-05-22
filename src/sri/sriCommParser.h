#ifndef SRI_COMM_PARSER_H
#define SRI_COMM_PARSER_H

#include "sriCommDefine.h"

class CSRICommParser {
public:
  CSRICommParser();
  virtual ~CSRICommParser();

  virtual bool OnReceivedData(BYTE* data, int dataLen);
  virtual bool OnNetworkFailure(std::string infor);
};

#endif
