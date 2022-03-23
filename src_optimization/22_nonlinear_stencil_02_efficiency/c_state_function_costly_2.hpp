
#pragma once

#include "vcl/vectormath_exp.h"

template <typename VecType>
class CStateFunctionCostly2
{
   public:
      static inline void apply(VecType & p_v)
      {
         p_v = pow(1+pow(10.0*p_v,3.5),-0.71);
      }
};
