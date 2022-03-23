
#pragma once

#include "vcl/vectormath_exp.h"

template <typename VecType>
class CStateFunctionCostly0
{
   public:
      static inline void apply(VecType & p_v)
      {
         constexpr double l_exp0 = 4.0/3.0;
         constexpr double l_exp1 = 5.0/3.0;
         p_v = pow(p_v,l_exp0) + pow(p_v,l_exp1);
      }
};
