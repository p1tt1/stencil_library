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
* 4 barriers within the loop
*
*  l_alpha_init commented out
*
*/

#pragma once

#include <omp.h>
#include <string>
#include "i_linear_operator.hpp"

template <typename ValueType>
class C_CG_B4_Hard_Part
{
    public:
        inline static const std::string IDENTIFER = "cg_b4_hard_part";
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
std::size_t C_CG_B4_Hard_Part<ValueType>::operator()(
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
    std::size_t l_iter;
    ValueType l_lambda = 0.;
    ValueType l_alpha_0 = 0.;
    ValueType l_alpha_1 = 0.;

    ValueType * l_p_raw = new ValueType[p_size+2*p_bufferSize];
    ValueType * l_p = &(l_p_raw[p_bufferSize]);
    ValueType * l_r = new ValueType[p_size];
    ValueType * l_upsilon = new ValueType[p_size];

    #pragma omp parallel
    {
        std::size_t l_thread_id = omp_get_thread_num();
        std::size_t l_nthreads = omp_get_num_threads();
        std::size_t l_i_ltb = p_size * l_thread_id       / l_nthreads;
        std::size_t l_i_utb = p_size * (l_thread_id + 1) / l_nthreads;
        std::size_t l_i_ltb_buffer = p_bufferSize * l_thread_id       / l_nthreads;
        std::size_t l_i_utb_buffer = p_bufferSize * (l_thread_id + 1) / l_nthreads;

        std::size_t l_iter_t = 0;
        ValueType l_alpha_init_t;
        ValueType l_alpha_0_t;
        ValueType l_alpha_1_t;
        ValueType l_lambda_t;
        ValueType l_beta_t;

        p_A.apply(p_x_0,l_r);
        #pragma omp barrier
        // --------------------------------------------------------------------

        for (std::size_t i = l_i_ltb; i < l_i_utb; ++i)
        {
            p_x_1[i] = p_x_0[i];
            l_r[i] = p_b[i] - l_r[i];
            l_p[i] = l_r[i];
        }
        // --------------------------------------------------------------------

        for (std::size_t i = l_i_ltb_buffer; i < l_i_utb_buffer; ++i)
        {
            l_p_raw[i] = 0;
            l_p_raw[p_size+p_bufferSize+i] = 0;
        }
        // --------------------------------------------------------------------

        //
        // NOTE: norm2 operation
        //
        l_alpha_0_t = 0;
        for (std::size_t i = l_i_ltb; i < l_i_utb; ++i)
        {
            l_alpha_0_t += l_r[i] * l_r[i];
        }
        #pragma omp atomic
        l_alpha_0 += l_alpha_0_t;
        #pragma omp barrier
        // --------------------------------------------------------------------

        l_alpha_init_t = l_alpha_0;
        // #pragma omp master
        // {
        //     std::cout << std::endl;
        //     std::cout << "l_alpha_init_t: " << l_alpha_init_t << std::endl;
        // }

        l_alpha_0_t = l_alpha_0;

        while(l_iter_t < p_iterMax)
        {
            // std::cout << l_thread_id << " : START WHILE" << std::endl;
            // if(l_alpha_0 < p_epsilon * l_alpha_init_t)
            if(l_alpha_0_t < p_epsilon)
            {
                // std::cout << l_thread_id << " : IN BREAK" << std::endl;
                break;
            }
            // std::cout << l_thread_id << " : AFTER IF -> break" << std::endl;

            p_A.apply(l_p,l_upsilon);

            //
            // NOTE ohne diese bariere geht es hier nicht...
            //
            #pragma omp barrier
            // --------------------------------------------------------------------

            //
            // Note: dot prod
            //
            l_lambda_t = 0;
            for (std::size_t i = l_i_ltb; i < l_i_utb; ++i)
            {
                l_lambda_t += l_upsilon[i] * l_p[i];
            }
            #pragma omp atomic
            l_lambda += l_lambda_t;
            #pragma omp barrier
            // --------------------------------------------------------------------

            l_lambda_t = l_alpha_0_t / l_lambda;

            if(l_thread_id == 0)
            {
                l_alpha_1 = 0.;
                // std::cout << "----------------------" << std::endl;
                // std::cout << "l_iter_t: " << l_iter_t << std::endl;
                // std::cout << "l_alpha_0_t: " << l_alpha_0_t << std::endl;
                // std::cout << "l_lambda: " << l_lambda << std::endl;
                // std::cout << "l_lambda_t: " << l_lambda_t << std::endl;
            }

            for (std::size_t i = l_i_ltb; i < l_i_utb; ++i)
            {
                p_x_1[i] = p_x_1[i] + l_lambda_t * l_p[i];
                l_r[i] = l_r[i] - l_lambda_t * l_upsilon[i];
            }

            //
            // NOTE: norm2 operation
            //
            l_alpha_1_t = 0;
            for (std::size_t i = l_i_ltb; i < l_i_utb; ++i)
            {
                l_alpha_1_t += l_r[i] * l_r[i];
            }
            // std::cout << "l_alpha_1_t: " << l_alpha_1_t << std::endl;
            #pragma omp atomic
            l_alpha_1 += l_alpha_1_t;
            #pragma omp barrier
            // --------------------------------------------------------------------

            if(l_thread_id == 0)
            {
                l_lambda = 0.;
            }

            l_beta_t = l_alpha_1/l_alpha_0_t;

            for (std::size_t i = l_i_ltb; i < l_i_utb; ++i)
            {
                l_p[i] = l_r[i] + l_beta_t * l_p[i];
            }
            #pragma omp barrier
            // --------------------------------------------------------------------

            l_alpha_0_t = l_alpha_1;
            l_iter_t++;
            // if(l_thread_id == 0)
            // {
            //     std::cout << "l_alpha_1 = " << l_alpha_1 << std::endl;
            //     l_alpha_1 = 0.;
            //     l_lambda = 0.;
            // }
        }
        // std::cout << l_thread_id << " : AFTER WHILE" << std::endl;
        if(l_thread_id == 0)
        {
            l_iter = l_iter_t;
        }
    }

    delete [] l_p_raw;
    delete [] l_r;
    delete [] l_upsilon;

    return(l_iter);
}
