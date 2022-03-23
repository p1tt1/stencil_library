
#pragma once

#include "i_linear_operator.hpp"

template <typename ValueType>
class ISolver
{
 public:
    virtual std::size_t operator()(
            const std::size_t p_size,
            const ILinearOperator<ValueType> & p_A,
            const ValueType * __restrict__ p_x_0,
            const ValueType * __restrict__ p_b,
            ValueType * __restrict__ p_x_1,
            const ValueType p_epsilon,
            const std::size_t p_iterMax,
            const std::size_t p_bufferSize
        ) const = 0;
};
