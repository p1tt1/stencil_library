
#pragma once

#include <string>
#include <omp.h>
#include "i_nonlinear_operator.hpp"

template <template<typename ValueType> typename StateFunc, typename ValueType, typename VecType>
class CNonlinearStencilPrecalc : public INonlinearOperator<ValueType>
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
    CNonlinearStencilPrecalc(
      const std::size_t p_objCols,
      const std::size_t p_objRows,
      const std::size_t p_objLevels,
      ValueType * p_s,
      const ValueType p_h = ValueType(1.0),
      const ValueType p_tau = ValueType(1.0),
      const ValueType p_epsilon = ValueType(1e-15)
      );
      inline static const std::string IDENTIFER = "nonlinear_precalc";
    void apply(const ValueType * __restrict__ p_x, ValueType * __restrict__ p_y) const;
    void setState(const ValueType * __restrict__ p_s);
    ~CNonlinearStencilPrecalc();
};

template <template<typename VecType> typename StateFunc, typename ValueType, typename VecType>
CNonlinearStencilPrecalc<StateFunc, ValueType, VecType>::CNonlinearStencilPrecalc(
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

   setState(p_s);
}

template <template<typename VecType> typename StateFunc, typename ValueType, typename VecType>
void CNonlinearStencilPrecalc<StateFunc, ValueType, VecType>::setState(const ValueType * __restrict__ p_s)
{
   #pragma omp parallel for
   for(size_t i=0; i<m_objLevels; ++i)
   {
      for(size_t j=0; j<m_objRows; ++j)
      {
         for(size_t k=0; k<m_objCols; ++k)
         {
            size_t l_pos = i*m_objSize2d + j*m_objSize1d + k;
            ValueType l_v_tmp = p_s[l_pos];
            StateFunc<ValueType>::apply(l_v_tmp);
            ValueType l_v_tmp_U;

            if(i==m_objLevels-1)
            {
               m_v_LU[l_pos] = 0;
            }
            else
            {
               l_v_tmp_U = p_s[l_pos+m_objSize2d];
               StateFunc<ValueType>::apply(l_v_tmp_U);
               m_v_LU[l_pos] = 2*m_factor*l_v_tmp*l_v_tmp_U/(l_v_tmp+l_v_tmp_U+m_epsilon);
            }

            if(j==m_objRows-1)
            {
               m_v_RU[l_pos] = 0;
            }
            else
            {
               l_v_tmp_U = p_s[l_pos+m_objSize1d];
               StateFunc<ValueType>::apply(l_v_tmp_U);
               m_v_RU[l_pos] = 2*m_factor*l_v_tmp*l_v_tmp_U/(l_v_tmp+l_v_tmp_U+m_epsilon);
            }

            if(k==m_objCols-1)
            {
               m_v_CU[l_pos] = 0;
            }
            else
            {
               l_v_tmp_U = p_s[l_pos+1];
               StateFunc<ValueType>::apply(l_v_tmp_U);
               m_v_CU[l_pos] = 2*m_factor*l_v_tmp*l_v_tmp_U/(l_v_tmp+l_v_tmp_U+m_epsilon);
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

template <template<typename VecType> typename StateFunc, typename ValueType, typename VecType>
void CNonlinearStencilPrecalc<StateFunc, ValueType, VecType>::apply(const ValueType * __restrict__ p_x, ValueType * __restrict__ p_y) const
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

template <template<typename VecType> typename StateFunc, typename ValueType, typename VecType>
CNonlinearStencilPrecalc<StateFunc, ValueType, VecType>::~CNonlinearStencilPrecalc()
{

}
