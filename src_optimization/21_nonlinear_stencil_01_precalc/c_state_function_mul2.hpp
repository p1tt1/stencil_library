
#pragma once

template <typename ValueType>
class CStateFunctionMul2
{
   public:
      static inline void apply(ValueType & p_v)
      {
         p_v *= 2;
      }
};
