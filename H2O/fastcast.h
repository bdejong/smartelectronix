#pragma once

   //2^36 * 1.5, (52-_shiftamt=36) uses limited precision to floor
   //16.16 fixed point representation

#ifdef MAC
        #define iexp_                           0
        #define iman_                           1
#else //LittleEndian
        #define iexp_                           1
        #define iman_                           0
#endif

const double _double2fixmagic = 68719476736.0*1.5;
const long   _shiftamt        = 16;

inline long fastcast(double val)
{
   val += _double2fixmagic;
   return ((long*)&val)[iman_] >> _shiftamt;
};

inline long fastcast(float val)
{
   return fastcast((double)val);
};
