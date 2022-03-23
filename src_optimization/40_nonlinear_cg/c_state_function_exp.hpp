
#pragma once

#include "vcl/vectormath_exp.h"

template <typename VecType>
class CStateFunctionExp
{
   public:
      static inline void apply(VecType & p_v)
      {
         p_v *= exp(p_v);
      }
};
