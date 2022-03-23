
#pragma once

#include "i_nonlinear_operator.hpp"
#include "i_solver.hpp"

template <typename ValueType>
class ITimestepCalculator
{
 public:
    virtual std::size_t operator()(
        const std::size_t p_size,
        const ISolver<ValueType> & p_Solver,
        INonlinearOperator<ValueType> & p_Op,
        const ValueType * __restrict__ p_b,
        ValueType * __restrict__ p_x,
        const ValueType p_epsilon_solver,
        const ValueType p_epsilon_step,
        const std::size_t p_iter_solver_max,
        const std::size_t p_iter_step_max,
        const std::size_t p_bufferSize
    ) const = 0;
};
