
#pragma once

#include <string>
#include <omp.h>
#include "i_linear_operator.hpp"

template <typename ValueType, typename VecType>
class CLinearStencilPrecalc : public ILinearOperator<ValueType>
{
 private:
   std::size_t m_objCols;
   std::size_t m_objRows;
   std::size_t m_objLevels;
   std::size_t m_objSize1d;
   std::size_t m_objSize2d;
   std::size_t m_objSize3d;
   const ValueType m_factor;
   const ValueType m_epsilon;
   ValueType * m_v_LL;
   ValueType * m_v_RL;
   ValueType * m_v_CL;
   ValueType * m_v;
   ValueType * m_v_CU;
   ValueType * m_v_RU;
   ValueType * m_v_LU;

 public:
    CLinearStencilPrecalc(
      const std::size_t p_objCols,
      const std::size_t p_objRows,
      const std::size_t p_objLevels,
      ValueType * p_c,
      const ValueType p_h = ValueType(1.0),
      const ValueType p_tau = ValueType(1.0),
      const ValueType p_epsilon = ValueType(1e-15)
      );
      inline static const std::string IDENTIFER = "several_coeff_precalc";
    void apply(const ValueType * __restrict__ p_x, ValueType * __restrict__ p_y) const;
    ~CLinearStencilPrecalc();
};

template <typename ValueType, typename VecType>
CLinearStencilPrecalc<ValueType, VecType>::CLinearStencilPrecalc(
   const std::size_t p_objCols,
   const std::size_t p_objRows,
   const std::size_t p_objLevels,
   ValueType * p_c,
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
m_factor(p_tau/(p_h*p_h)),
m_epsilon(p_epsilon)
{
   //
   // NOTE "+1" so that upper vectors can be referenced in a shifted way
   //
   m_v_LL = new ValueType[m_objSize3d+m_objSize2d];
   m_v_RL = new ValueType[m_objSize3d+m_objSize1d];
   m_v_CL = new ValueType[m_objSize3d+1];
   m_v    = new ValueType[m_objSize3d];

   m_v_CU = &(m_v_CL[1]);
   m_v_RU = &(m_v_RL[m_objSize1d]);
   m_v_LU = &(m_v_LL[m_objSize2d]);

   //
   // first touch
   //
   #pragma omp parallel for
   for (size_t i = 0; i < m_objSize3d; ++i)
   {
      m_v_LU[i] = ValueType(0);
      m_v_RU[i] = ValueType(0);
      m_v_CU[i] = ValueType(0);
      m_v[i]    = ValueType(0);
   }
   m_v_CL[0] = ValueType(0);
   #pragma omp parallel for
   for (size_t i = 0; i < m_objSize1d; ++i)
   {
      m_v_RL[i] = ValueType(0);
   }
   #pragma omp parallel for
   for (size_t i = 0; i < m_objSize2d; ++i)
   {
      m_v_LL[i] = ValueType(0);
   }

   #pragma omp parallel for
   for(size_t i=0; i<m_objLevels; ++i)
   {
      for(size_t j=0; j<m_objRows; ++j)
      {
         for(size_t k=0; k<m_objCols; ++k)
         {
            size_t l_pos = i*m_objSize2d + j*m_objSize1d + k;
            if(i==m_objLevels-1)
            {
               m_v_LU[l_pos] = 0;
            }
            else
            {
               m_v_LU[l_pos] = 2*m_factor*p_c[l_pos]*p_c[l_pos+m_objSize2d]/(p_c[l_pos]+p_c[l_pos+m_objSize2d]+m_epsilon);
            }

            if(j==m_objRows-1)
            {
               m_v_RU[l_pos] = 0;
            }
            else
            {
               m_v_RU[l_pos] = 2*m_factor*p_c[l_pos]*p_c[l_pos+m_objSize1d]/(p_c[l_pos]+p_c[l_pos+m_objSize1d]+m_epsilon);
            }

            if(k==m_objCols-1)
            {
               m_v_CU[l_pos] = 0;
            }
            else
            {
               m_v_CU[l_pos] = 2*m_factor*p_c[l_pos]*p_c[l_pos+1]/(p_c[l_pos]+p_c[l_pos+1]+m_epsilon);
            }
         }
      }
   }
   #pragma omp parallel for
   for (size_t i = 0; i < m_objSize3d; ++i)
   {
      m_v[i]=1+m_v_LL[i]+m_v_RL[i]+m_v_CL[i]+m_v_CU[i]+m_v_RU[i]+m_v_LU[i];
   }

}

template <typename ValueType, typename VecType>
void CLinearStencilPrecalc<ValueType, VecType>::apply(const ValueType * __restrict__ p_x, ValueType * __restrict__ p_y) const
{
   std::size_t l_pos;

   VecType l_x_LL_Vec;
   VecType l_x_RL_Vec;
   VecType l_x_CL_Vec;
   VecType l_x_Vec;
   VecType l_x_CU_Vec;
   VecType l_x_RU_Vec;
   VecType l_x_LU_Vec;

   VecType l_y_Vec;

   VecType l_v_LL_Vec;
   VecType l_v_RL_Vec;
   VecType l_v_CL_Vec;
   VecType l_v_Vec;
   VecType l_v_CU_Vec;
   VecType l_v_RU_Vec;
   VecType l_v_LU_Vec;

   #pragma omp for
   for (std::size_t l_pos_L=0; l_pos_L<m_objLevels; ++l_pos_L)
   {
      for (std::size_t l_pos_R=0; l_pos_R<m_objRows; ++l_pos_R)
      {
         for (std::size_t l_pos_C=0; l_pos_C<m_objCols; l_pos_C+=VecType::size())
         {
            l_pos = l_pos_L * m_objSize2d + l_pos_R * m_objSize1d + l_pos_C;

            l_x_LL_Vec.load(p_x + l_pos - m_objSize2d);
            l_x_RL_Vec.load(p_x + l_pos - m_objSize1d);
            l_x_CL_Vec.load(p_x + l_pos - 1          );
            l_x_Vec.load(   p_x + l_pos              );
            l_x_CU_Vec.load(p_x + l_pos + 1          );
            l_x_RU_Vec.load(p_x + l_pos + m_objSize1d);
            l_x_LU_Vec.load(p_x + l_pos + m_objSize2d);

            l_v_LL_Vec.load(m_v_LL + l_pos);
            l_v_RL_Vec.load(m_v_RL + l_pos);
            l_v_CL_Vec.load(m_v_CL + l_pos);
            l_v_Vec.load(   m_v    + l_pos);
            l_v_CU_Vec.load(m_v_CU + l_pos);
            l_v_RU_Vec.load(m_v_RU + l_pos);
            l_v_LU_Vec.load(m_v_LU + l_pos);

            l_y_Vec =
               l_v_Vec    * l_x_Vec
            -  l_v_LL_Vec * l_x_LL_Vec
            -  l_v_RL_Vec * l_x_RL_Vec
            -  l_v_CL_Vec * l_x_CL_Vec
            -  l_v_CU_Vec * l_x_CU_Vec
            -  l_v_RU_Vec * l_x_RU_Vec
            -  l_v_LU_Vec * l_x_LU_Vec
            ;
            l_y_Vec.store(p_y + l_pos);
         }
      }
   }
}

template <typename ValueType, typename VecType>
CLinearStencilPrecalc<ValueType, VecType>::~CLinearStencilPrecalc()
{

}
