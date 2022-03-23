
#pragma once

template <typename VecType>
class CStateFunctionPow2
{
   public:
      static inline void apply(VecType & p_v)
      {
         p_v = pow(p_v,2);
      }
};
