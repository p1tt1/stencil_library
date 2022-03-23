/*
* based on
* @book{grebhofer19_num,
*  author = {Ulrich Grebhofer},
*  title = {Numerische Verfahren},
*  series = {[]},
*  publisher = {De Gruyter Oldenbourg},
*  year = {2019},
*  pages = {nil},
*  doi = {10.1515/9783110644173},
*  url = {http://dx.doi.org/10.1515/9783110644173},
* }
*
* 7 barriers within the loop
*
*  l_alpha_init commented out
*
*/

#pragma once

#include <omp.h>
#include <string>
#include "i_linear_operator.hpp"

template <typename ValueType>
class C_CG_B7
{
    public:
        inline static const std::string IDENTIFER = "cg_b7";
        std::size_t operator()(
            const std::size_t p_size,
            const ILinearOperator<ValueType> & p_A,
            const ValueType * __restrict__ p_x_0,
            const ValueType * __restrict__ p_b,
            ValueType * __restrict__ p_x_1,
            const ValueType p_epsilon,
            const std::size_t p_iterMax,
            const std::size_t p_bufferSize
        ) const;
};

template <typename ValueType>
std::size_t C_CG_B7<ValueType>::operator()(
    const std::size_t p_size,
    const ILinearOperator<ValueType> & p_A,
    const ValueType * __restrict__ p_x_0,
    const ValueType * __restrict__ p_b,
    ValueType * __restrict__ p_x_1,
    const ValueType p_epsilon,
    const std::size_t p_iterMax,
    const std::size_t p_bufferSize
) const
{
    std::size_t l_iter = 0;
    ValueType l_lambda = 0.;
    ValueType l_alpha_0 = 0.;
    ValueType l_alpha_1 = 0.;

    ValueType * l_p_raw = new ValueType[p_size+2*p_bufferSize];
    ValueType * l_p = &(l_p_raw[p_bufferSize]);
    ValueType * l_r = new ValueType[p_size];
    ValueType * l_upsilon = new ValueType[p_size];

    #pragma omp parallel
    {
        ValueType l_alpha_init;
        p_A.apply(p_x_0,l_r);
        #pragma omp barrier
        // --------------------------------------------------------------------

        #pragma omp for
        for(std::size_t i = 0; i < p_size; ++i)
        {
            p_x_1[i] = p_x_0[i];
            l_r[i] = p_b[i] - l_r[i];
            l_p[i] = l_r[i];
        }
        // --------------------------------------------------------------------

        #pragma omp for
        for (std::size_t i = 0; i < p_bufferSize; ++i)
        {
            l_p_raw[i] = 0;
            l_p_raw[p_size+p_bufferSize+i] = 0;
        }
        // --------------------------------------------------------------------

        //
        // NOTE: norm2 operation
        //
        #pragma omp for reduction(+: l_alpha_0)
        for(std::size_t i = 0; i < p_size; ++i)
        {
            l_alpha_0 += l_r[i] * l_r[i];
        }
        // --------------------------------------------------------------------
        l_alpha_init = l_alpha_0;
        // #pragma omp master
        // {
        //     std::cout << std::endl;
        //     std::cout << "l_alpha_init: " << l_alpha_init << std::endl;
        // }

        while(l_iter < p_iterMax)
        {
            // if(l_alpha_0 < p_epsilon * l_alpha_init)
            if(l_alpha_0 < p_epsilon)
            {
                break;
            }

            p_A.apply(l_p,l_upsilon);
            #pragma omp barrier
            // --------------------------------------------------------------------

            //
            // Note: dot prod
            //
            #pragma omp for reduction(+: l_lambda)
            for(std::size_t i = 0; i < p_size; ++i)
            {
                l_lambda += l_upsilon[i] * l_p[i];
            }
            // --------------------------------------------------------------------
            #pragma omp master
            {
                // std::cout << std::endl;
                // std::cout << "l_iter: " << l_iter << std::endl;
                // std::cout << "l_lambda = " << l_lambda << std::endl;
                // std::cout << "l_alpha_0 = " << l_alpha_0 << std::endl;
                l_lambda = l_alpha_0 / l_lambda;
                // std::cout << "l_lambda = " << l_lambda << std::endl;
            }
            #pragma omp barrier
            // --------------------------------------------------------------------

            #pragma omp for
            for(std::size_t i = 0; i < p_size; ++i)
            {
                p_x_1[i] = p_x_1[i] + l_lambda * l_p[i];
                l_r[i] = l_r[i] - l_lambda * l_upsilon[i];
            }
            // --------------------------------------------------------------------

            //
            // NOTE: norm2 operation
            //
            #pragma omp for reduction(+: l_alpha_1)
            for(std::size_t i = 0; i < p_size; ++i)
            {
                l_alpha_1 += l_r[i] * l_r[i];
            }
            // --------------------------------------------------------------------

            #pragma omp for
            for(std::size_t i = 0; i < p_size; ++i)
            {
                l_p[i] = l_r[i] + (l_alpha_1 * l_p[i]) / l_alpha_0;
            }
            // --------------------------------------------------------------------

            #pragma omp master
            {
                // std::cout << "l_iter = " << l_iter << std::endl;
                // std::cout << "l_alpha_init = " << l_alpha_init << std::endl;
                // std::cout << "l_alpha_1 = " << l_alpha_1 << std::endl;
                // std::cout << "-------------------" << std::endl;
                l_alpha_0 = l_alpha_1;
                l_alpha_1 = 0.;
                l_lambda = 0.;
                l_iter++;
            }
            #pragma omp barrier
            // --------------------------------------------------------------------
        }
    }

    delete [] l_p_raw;
    delete [] l_r;
    delete [] l_upsilon;

    return(l_iter);
}
