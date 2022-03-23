#pragma once

#include "i_linear_operator.hpp"

template <typename ValueType>
class INonlinearOperator: public ILinearOperator<ValueType>
{
 public:
    virtual void setState(const ValueType * __restrict__ p_s) = 0;
};
