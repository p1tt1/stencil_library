
#pragma once

template <typename VecType>
struct SSF_mul2 {
  inline void operator()(VecType & p_v)
      {
         p_v *= 2;
      }
};
// class CStateFunctionMul2 : public IStateFunction<VecType>
// {
//    public:
//       static inline virtual void apply(const VecType & p_v)
//       {
//          p_v *=
//       }
// };
