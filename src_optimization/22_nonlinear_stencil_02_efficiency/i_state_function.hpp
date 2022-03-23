#pragma once

template <typename VecType>
class IStateFunction
{
 public:
    static inline virtual void apply(const VecType * p_v) = 0;
};
