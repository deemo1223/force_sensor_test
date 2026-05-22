#ifndef FORCESENSOR_DATA_H
#define FORCESENSOR_DATA_H

// #include "math/cppTypes.h"
#include <iostream>

typedef struct 
{
    float F[3]; //Fx Fy Fz
    float M[3]; //Mx My Mz
    float F_filter[3];  //经过1st_RC_filter后的数据
    float M_filter[3];
    //float F_stdDev[3];    //标准差数据
    float F_extrem[3][4]; //力数据的极值数据
    //float F_stdDev_extrem[3][2];//标准差的极值数据
} forcesensor_data;




#endif