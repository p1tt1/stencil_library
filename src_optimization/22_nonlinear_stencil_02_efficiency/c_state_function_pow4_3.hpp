
#pragma once

#include "vcl/vectormath_exp.h"

template <typename VecType>
class CStateFunctionPow4_3
{
   public:
      static inline void apply(VecType & p_v)
      {
         constexpr double l_exp = 4.0/3.0;
         p_v = pow(p_v,l_exp);
      }
};
