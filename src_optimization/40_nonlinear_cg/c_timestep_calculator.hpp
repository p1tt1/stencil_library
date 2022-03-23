/*
*
* => Work with relative tolerance
*
*/

#pragma once

#include <omp.h>
#include <string>
#include "i_nonlinear_operator.hpp"
#include "i_solver.hpp"
#include "i_timestep_calculator.hpp"

#define SWAP_PTR(p_x_new,p_x_old,p_x_tmp) (p_x_tmp=p_x_new, p_x_new=p_x_old, p_x_old=p_x_tmp)

template <typename ValueType>
class C_TimestepCalculator : public ITimestepCalculator<ValueType>
{
    public:
        inline static const std::string IDENTIFER = "c_timestep_calculator";
        std::size_t operator()(
            const std::size_t p_size,
            const ISolver<ValueType> & p_Solver,
            INonlinearOperator<ValueType> & p_Op,
            const ValueType * __restrict__ p_y,
            ValueType * __restrict__ p_x,
            const ValueType p_epsilon_solver,
            const ValueType p_epsilon_step,
            const std::size_t p_iter_solver_max,
            const std::size_t p_iter_step_max,
            const std::size_t p_bufferSize
        ) const;
};

template <typename ValueType>
std::size_t C_TimestepCalculator<ValueType>::operator()(
    const std::size_t p_size,
    const ISolver<ValueType> & p_Solver,
    INonlinearOperator<ValueType> & p_Op,
    const ValueType * __restrict__ p_y,
    ValueType * __restrict__ p_x,
    const ValueType p_epsilon_solver,
    const ValueType p_epsilon_step,
    const std::size_t p_iter_solver_max,
    const std::size_t p_iter_step_max,
    const std::size_t p_bufferSize
) const
{
    ValueType * l_x_k0_raw;
    ValueType * l_x_k1_raw;
    ValueType * l_x_k0;
    ValueType * l_x_k1;
    ValueType * l_z;
    ValueType * l_x_tmp;
    ValueType l_res_0 = ValueType(0);
    ValueType l_res_k = ValueType(0);
    ValueType l_tol_rel;
    std::size_t l_iter = 0;
    std::size_t l_iter_solver;

    l_x_k0_raw = new ValueType[p_size+2*p_bufferSize];
    l_x_k1_raw = new ValueType[p_size+2*p_bufferSize];
    l_z = new ValueType[p_size];
    l_x_k0 = &(l_x_k0_raw[p_bufferSize]);
    l_x_k1 = &(l_x_k1_raw[p_bufferSize]);

    #pragma omp parallel for
    for(std::size_t i = 0; i < p_size; ++i)
    {
        l_x_k0[i] = ValueType(0);
        l_x_k1[i] = ValueType(0);
        l_z[i] = ValueType(0);
    }

    for (std::size_t i = 0; i < p_bufferSize; ++i)
    {
        l_x_k0_raw[i] = 0;
        l_x_k1_raw[i] = 0;
        l_x_k0_raw[p_size+p_bufferSize+i] = 0;
        l_x_k1_raw[p_size+p_bufferSize+i] = 0;
    }

    // std::cout << "p_y:" << std::endl;
    // printArray1d(p_y, p_size);

    p_Op.setState(p_y);
    l_iter_solver = p_Solver(p_size, p_Op, p_y, p_y, l_x_k1, p_epsilon_solver, p_iter_solver_max, p_bufferSize);
    p_Op.apply(l_x_k1, l_z);

    //
    // NOTE: norm2 operation
    //
    #pragma omp parallel for reduction(+: l_res_0)
    for(std::size_t i = 0; i < p_size; ++i)
    {
        l_res_0 += (l_z[i] - p_y[i]) * (l_z[i] - p_y[i]);
    }

    // l_tol_rel = l_res_0 * p_epsilon_step;
    l_tol_rel = p_epsilon_step;

    l_res_k = l_res_0;

    while (l_iter < p_iter_step_max)
    {
        if(l_res_k < l_tol_rel)
        {
            break;
        }

        SWAP_PTR(l_x_k0, l_x_k1, l_x_tmp);

        std::cout << C_TimestepCalculator<ValueType>::IDENTIFER
                  << ": iter: "    << l_iter
                  << " sol_iter: " << l_iter_solver
                  << " tol: " << l_res_k  << ">" << l_tol_rel << std::endl;

        p_Op.setState(l_x_k0);
        l_iter_solver = p_Solver(p_size, p_Op, l_x_k0, p_y, l_x_k1, p_epsilon_solver, p_iter_solver_max, p_bufferSize);
        p_Op.apply(l_x_k1, l_z);

        l_res_k = 0;
        #pragma omp parallel for reduction(+: l_res_k)
        for(std::size_t i = 0; i < p_size; ++i)
        {
            l_res_k += (l_z[i] - p_y[i]) * (l_z[i] - p_y[i]);
        }

        l_iter++;
    }

    #pragma omp parallel for
    for(std::size_t i = 0; i < p_size; ++i)
    {
        p_x[i] = l_x_k1[i];
    }

    delete [] l_x_k0_raw;
    delete [] l_x_k1_raw;
    delete [] l_z;

    return(l_iter);
}
