/*
 *                   C6 --------E11-------- C7
 *                 / |                     / |
 *               E9  |                   E10 |
 *              /    |                  /    |
 *             C4 ---------E8--------- C5    E7
 *             |     E6                |     |
 *             |     |                 |     |
 *             |     |                 E5    |
 *             E4    C2 -------E3------|---- C3
 *             |    /                  |    /
 *             |  E1                   |   E2
 *             | /                     | /
 *             C0 ---------E0--------- C1
 *
 * S0: E0-E2-E3-E1
 * S1: E0-E5-E8-E4
 * S2: E2-E7-E10-E5
 * S3: E3-E7-E11-E6
 * S4: E1-E6-E9-E4
 * S5: E8-E10-E11-E9
 *
 */

#pragma once

#include <string>
#include <omp.h>
#include "i_linear_operator.hpp"

template <typename ValueType, typename VecType>
class CLinearStencil : public ILinearOperator<ValueType>
{
 private:
    std::size_t m_objCols;
    std::size_t m_objRows;
    std::size_t m_objLevels;
    std::size_t m_objSize1d;
    std::size_t m_objSize2d;
    std::size_t m_objSize3d;
    const ValueType m_factor;

 public:
    CLinearStencil(
      const std::size_t p_objCols,
      const std::size_t p_objRows,
      const std::size_t p_objLevels,
      const ValueType p_c = ValueType(1.0),
      const ValueType p_h = ValueType(1.0),
      const ValueType p_tau = ValueType(1.0)
      );
      inline static const std::string IDENTIFER = "one_coeff";
    void apply(const ValueType * __restrict__ p_x, ValueType * __restrict__ p_y) const;
    ~CLinearStencil();
};

template <typename ValueType, typename VecType>
CLinearStencil<ValueType, VecType>::CLinearStencil(
   const std::size_t p_objCols,
   const std::size_t p_objRows,
   const std::size_t p_objLevels,
   const ValueType p_c,
   const ValueType p_h,
   const ValueType p_tau
   ):
m_objCols(p_objCols),
m_objRows(p_objRows),
m_objLevels(p_objLevels),
m_objSize1d(p_objCols),
m_objSize2d(p_objCols * p_objRows),
m_objSize3d(p_objCols * p_objRows * p_objLevels),
m_factor(p_c*p_tau/(p_h*p_h))
{

}

template <typename ValueType, typename VecType>
void CLinearStencil<ValueType, VecType>::apply(const ValueType * __restrict__ p_x, ValueType * __restrict__ p_y) const
{
   std::size_t l_pos;

   VecType l_pos_C_Vec;

   ValueType l_factor_LL;
   ValueType l_factor_RL;
   ValueType l_factor_RU;
   ValueType l_factor_LU;

   VecType l_factor_CL_Vec;
   VecType l_factor_CU_Vec;

   VecType l_x_LL_Vec;
   VecType l_x_RL_Vec;
   VecType l_x_CL_Vec;
   VecType l_x_Vec;
   VecType l_x_CU_Vec;
   VecType l_x_RU_Vec;
   VecType l_x_LU_Vec;

   VecType l_y_Vec;

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

            l_factor_CL_Vec = select(l_pos_C_Vec>0,           m_factor, 0.0);
            l_factor_CU_Vec = select(l_pos_C_Vec<m_objCols-1,   m_factor, 0.0);

            l_factor_LL = (1-((m_objLevels-1-l_pos_L)/(m_objLevels-1))) * m_factor;
            l_factor_RL = (1-((m_objRows-1-l_pos_R)  /(m_objRows-1)))   * m_factor;
            l_factor_RU = (1-(l_pos_R                /(m_objRows-1)))   * m_factor;
            l_factor_LU = (1-(l_pos_L                /(m_objLevels-1))) * m_factor;

            l_y_Vec =
                     ( 1               +
                        l_factor_LL     +
                        l_factor_RL     +
                        l_factor_CL_Vec +
                        l_factor_CU_Vec +
                        l_factor_RU     +
                        l_factor_LU
                     )                 * l_x_Vec
                  - l_factor_LL       * l_x_LL_Vec
                  - l_factor_RL       * l_x_RL_Vec
                  - l_factor_CL_Vec   * l_x_CL_Vec
                  - l_factor_CU_Vec   * l_x_CU_Vec
                  - l_factor_RU       * l_x_RU_Vec
                  - l_factor_LU       * l_x_LU_Vec;
            l_y_Vec.store(p_y + l_pos);
         }
      }
   }
}

template <typename ValueType, typename VecType>
CLinearStencil<ValueType, VecType>::~CLinearStencil()
{

}
