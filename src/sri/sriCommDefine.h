#ifndef SRI_COMM_DEFINE_H
#define SRI_COMM_DEFINE_H

#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

typedef unsigned char BYTE;

typedef std::function<bool(std::string)> SRICommNetworkFailureCallbackFunction;
typedef std::function<bool(std::string)> SRICommATCallbackFunction;
typedef std::function<bool(float fx, float fy, float fz, float mx, float my, float mz)>
    SRICommM8218CallbackFunction;

#endif
