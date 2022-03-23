
#pragma once

#include <string>
#include <omp.h>
#include "i_nonlinear_operator.hpp"

template <template<typename VecType> typename StateFunc, typename ValueType, typename VecType>
class CNonlinearStencil : public INonlinearOperator<ValueType>
{
 private:
    std::size_t m_objCols;
    std::size_t m_objRows;
    std::size_t m_objLevels;
    std::size_t m_objSize1d;
    std::size_t m_objSize2d;
    std::size_t m_objSize3d;
    const ValueType * m_s;
    const ValueType m_factor;
    const ValueType m_epsilon;

 public:
    CNonlinearStencil(
      const std::size_t p_objCols,
      const std::size_t p_objRows,
      const std::size_t p_objLevels,
      ValueType * p_s,
      const ValueType p_h = ValueType(1.0),
      const ValueType p_tau = ValueType(1.0),
      const ValueType p_epsilon = ValueType(1e-15)
      );
      inline static const std::string IDENTIFER = "nonlinear";
    void apply(const ValueType * __restrict__ p_x, ValueType * __restrict__ p_y) const;
    void setState(const ValueType * __restrict__ p_s);
    ~CNonlinearStencil();
};

template <template<typename VecType> typename StateFunc, typename ValueType, typename VecType>
CNonlinearStencil<StateFunc, ValueType, VecType>::CNonlinearStencil(
   const std::size_t p_objCols,
   const std::size_t p_objRows,
   const std::size_t p_objLevels,
   ValueType * p_s,
   const ValueType p_h,
   const ValueType p_tau,
   const ValueType p_epsilon
   ):
m_objCols(p_objCols),
m_objRows(p_objRows),
m_objLevels(p_objLevels),
m_objSize1d(p_objCols),
m_objSize2d(p_objCols * p_objRows),
m_objSize3d(p_objCols * p_objRows * p_objLevels),
m_s(p_s),
m_factor(p_tau/(p_h*p_h)),
m_epsilon(p_epsilon)
{

}

template <template<typename VecType> typename StateFunc, typename ValueType, typename VecType>
void CNonlinearStencil<StateFunc, ValueType, VecType>::setState(const ValueType * __restrict__ p_s)
{
   m_s = p_s;
}

template <template<typename VecType> typename StateFunc, typename ValueType, typename VecType>
void CNonlinearStencil<StateFunc, ValueType, VecType>::apply(const ValueType * __restrict__ p_x, ValueType * __restrict__ p_y) const
{
   std::size_t l_pos;

   VecType l_pos_C_Vec;

   VecType l_factor_LL_Vec;
   VecType l_factor_RL_Vec;
   VecType l_factor_CL_Vec;
   VecType l_factor_CU_Vec;
   VecType l_factor_RU_Vec;
   VecType l_factor_LU_Vec;

   VecType l_x_LL_Vec;
   VecType l_x_RL_Vec;
   VecType l_x_CL_Vec;
   VecType l_x_Vec;
   VecType l_x_CU_Vec;
   VecType l_x_RU_Vec;
   VecType l_x_LU_Vec;

   VecType l_y_Vec;

   VecType l_c_LL_Vec;
   VecType l_c_RL_Vec;
   VecType l_c_CL_Vec;
   VecType l_c_Vec;
   VecType l_c_CU_Vec;
   VecType l_c_RU_Vec;
   VecType l_c_LU_Vec;

   #pragma omp for
   for (std::size_t l_pos_L=0; l_pos_L<m_objLevels; ++l_pos_L)
   {
      for (std::size_t l_pos_R=0; l_pos_R<m_objRows; ++l_pos_R)
      {
         for (std::size_t l_pos_C=0; l_pos_C<m_objCols; l_pos_C+=VecType::size())
         {
            l_pos = l_pos_L * m_objSize2d + l_pos_R * m_objSize1d + l_pos_C;

            //
            // WORKAROUND hardcoded vector size of 4
            //
            l_pos_C_Vec = VecType(l_pos_C, l_pos_C+1, l_pos_C+2, l_pos_C+3);

            l_x_LL_Vec.load(p_x + l_pos - m_objSize2d);
            l_x_RL_Vec.load(p_x + l_pos - m_objSize1d);
            l_x_CL_Vec.load(p_x + l_pos - 1          );
            l_x_Vec.load(   p_x + l_pos              );
            l_x_CU_Vec.load(p_x + l_pos + 1          );
            l_x_RU_Vec.load(p_x + l_pos + m_objSize1d);
            l_x_LU_Vec.load(p_x + l_pos + m_objSize2d);

            l_c_LL_Vec.load(m_s + l_pos - m_objSize2d);
            l_c_RL_Vec.load(m_s + l_pos - m_objSize1d);
            l_c_CL_Vec.load(m_s + l_pos - 1          );
            l_c_Vec.load(   m_s + l_pos              );
            l_c_CU_Vec.load(m_s + l_pos + 1          );
            l_c_RU_Vec.load(m_s + l_pos + m_objSize1d);
            l_c_LU_Vec.load(m_s + l_pos + m_objSize2d);

            StateFunc<VecType>::apply(l_c_LL_Vec);
            StateFunc<VecType>::apply(l_c_RL_Vec);
            StateFunc<VecType>::apply(l_c_CL_Vec);
            StateFunc<VecType>::apply(l_c_Vec);
            StateFunc<VecType>::apply(l_c_CU_Vec);
            StateFunc<VecType>::apply(l_c_RU_Vec);
            StateFunc<VecType>::apply(l_c_LU_Vec);

            l_factor_CL_Vec = select(l_pos_C_Vec>0,           m_factor, 0.0);
            l_factor_CU_Vec = select(l_pos_C_Vec<m_objCols-1,   m_factor, 0.0);

            l_factor_LL_Vec = (1-((m_objLevels-1-l_pos_L)/(m_objLevels-1))) * m_factor * 2 * l_c_Vec * l_c_LL_Vec / (l_c_Vec+l_c_LL_Vec+m_epsilon);
            l_factor_RL_Vec = (1-((m_objRows-1-l_pos_R)  /(m_objRows-1)))   * m_factor * 2 * l_c_Vec * l_c_RL_Vec / (l_c_Vec+l_c_RL_Vec+m_epsilon);
            l_factor_CL_Vec *=                                                           2 * l_c_Vec * l_c_CL_Vec / (l_c_Vec+l_c_CL_Vec+m_epsilon);
            l_factor_CU_Vec *=                                                           2 * l_c_Vec * l_c_CU_Vec / (l_c_Vec+l_c_CU_Vec+m_epsilon);
            l_factor_RU_Vec = (1-(l_pos_R                /(m_objRows-1)))   * m_factor * 2 * l_c_Vec * l_c_RU_Vec / (l_c_Vec+l_c_RU_Vec+m_epsilon);
            l_factor_LU_Vec = (1-(l_pos_L                /(m_objLevels-1))) * m_factor * 2 * l_c_Vec * l_c_LU_Vec / (l_c_Vec+l_c_LU_Vec+m_epsilon);

            l_y_Vec =
                     ( 1                +
                        l_factor_LL_Vec +
                        l_factor_RL_Vec +
                        l_factor_CL_Vec +
                        l_factor_CU_Vec +
                        l_factor_RU_Vec +
                        l_factor_LU_Vec
                     )                  * l_x_Vec
                  - l_factor_LL_Vec     * l_x_LL_Vec
                  - l_factor_RL_Vec     * l_x_RL_Vec
                  - l_factor_CL_Vec     * l_x_CL_Vec
                  - l_factor_CU_Vec     * l_x_CU_Vec
                  - l_factor_RU_Vec     * l_x_RU_Vec
                  - l_factor_LU_Vec     * l_x_LU_Vec;
            l_y_Vec.store(p_y + l_pos);
         }
      }
   }
}

template <template<typename VecType> typename StateFunc, typename ValueType, typename VecType>
CNonlinearStencil<StateFunc, ValueType, VecType>::~CNonlinearStencil()
{

}
