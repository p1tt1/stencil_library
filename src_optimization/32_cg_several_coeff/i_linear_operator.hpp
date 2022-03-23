#pragma once

template <typename ValueType>
class ILinearOperator
{
 public:
    virtual void apply(const ValueType * __restrict__ p_x, ValueType * __restrict__ p_y) const = 0;
};
