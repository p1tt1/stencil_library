
#pragma once

template <typename VecType>
class CStateFunctionMul2
{
   public:
      static inline void apply(VecType & p_v)
      {
         p_v *= 2;
      }
};
