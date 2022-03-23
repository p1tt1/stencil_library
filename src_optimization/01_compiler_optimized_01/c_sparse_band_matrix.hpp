
#pragma once

#include <string>
#include "i_linear_operator.hpp"

template <typename ValueType>
class CSparseBandMatrix : public ILinearOperator<ValueType>
{
 private:
   const std::size_t m_objSize1d;
   const std::size_t m_objSize2d;
   const std::size_t m_objSize3d;
   ValueType * m_v_LL;
   ValueType * m_v_RL;
   ValueType * m_v_CL;
   ValueType * m_v;
   ValueType * m_v_CU;
   ValueType * m_v_RU;
   ValueType * m_v_LU;

 public:
   CSparseBandMatrix(
      const std::size_t p_objCols,
      const std::size_t p_objRows,
      const std::size_t p_objLevels,
      const ValueType p_c = ValueType(1.0),
      const ValueType p_h = ValueType(1.0),
      const ValueType p_tau = ValueType(1.0)
   );
   inline static const std::string IDENTIFER = "sparse_band_matrix";
   void apply(
      const ValueType * __restrict__ p_x,
      ValueType * __restrict__ p_y
   ) const;
   ~CSparseBandMatrix();
};

template <typename ValueType>
CSparseBandMatrix<ValueType>::CSparseBandMatrix(
   const std::size_t p_objCols,
   const std::size_t p_objRows,
   const std::size_t p_objLevels,
   const ValueType p_c,
   const ValueType p_h,
   const ValueType p_tau
):
m_objSize1d(p_objCols),
m_objSize2d(p_objCols * p_objRows),
m_objSize3d(p_objCols * p_objRows * p_objLevels)
{
   ValueType l_factor = p_c*p_tau/(p_h*p_h);

   //
   // NOTE "+1" so that upper vectors can be referenced in a shifted way
   //
   m_v_LL = new ValueType[m_objSize3d+m_objSize2d];
   m_v_RL = new ValueType[m_objSize3d+m_objSize1d];
   m_v_CL = new ValueType[m_objSize3d+1];
   m_v = new ValueType[m_objSize3d];

   m_v_CU = &(m_v_CL[1]);
   m_v_RU = &(m_v_RL[m_objSize1d]);
   m_v_LU = &(m_v_LL[m_objSize2d]);

   for (std::size_t i = 0; i < m_objSize3d; ++i)
   {
      m_v_LL[i] = ValueType(0);
      m_v_RL[i] = ValueType(0);
      m_v_CL[i] = ValueType(0);
      m_v[i] = ValueType(0);
   }

   for (size_t i = m_objSize3d; i < m_objSize3d + m_objSize2d; ++i)
   {
      m_v_LL[i] = ValueType(0);
   }

   for (size_t i = m_objSize3d; i < m_objSize3d + m_objSize1d; ++i)
   {
      m_v_RL[i] = ValueType(0);
   }

   m_v_CL[m_objSize3d] = ValueType(0);


   for(std::size_t i=0; i<p_objLevels; ++i)
   {
      for(std::size_t j=0; j<p_objRows; ++j)
      {
         for(std::size_t k=0; k<p_objCols; ++k)
         {
            std::size_t lPos = i*m_objSize2d + j*m_objSize1d + k;

            if(i > 0)
            {
               m_v_LL[lPos] = -l_factor;
               m_v[lPos]++;
            }
            if(i < p_objLevels-1)
            {
               m_v[lPos]++;
            }

            if(j > 0)
            {
               m_v_RL[lPos] = -l_factor;
               m_v[lPos]++;
            }
            if(j < p_objRows-1)
            {
               m_v[lPos]++;
            }

            if(k > 0)
            {
               m_v_CL[lPos] = -l_factor;
               m_v[lPos]++;
            }
            if(k < p_objCols-1)
            {
               m_v[lPos]++;
            }

            m_v[lPos] = 1 + l_factor*m_v[lPos];
         }
      }
   }
}

template <typename ValueType>
void CSparseBandMatrix<ValueType>::apply(
   const ValueType * __restrict__ p_x,
   ValueType * __restrict__ p_y) const
{
   p_y[0] =   m_v[0]    * p_x[0]
            + m_v_CU[0] * p_x[1]
            + m_v_RU[0] * p_x[m_objSize1d]
            + m_v_LU[0] * p_x[m_objSize2d];

   for (size_t i=1;i<m_objSize1d;++i)
   {
      p_y[i] =   m_v[i]    * p_x[i]
               + m_v_CL[i] * p_x[i-1]
               + m_v_CU[i] * p_x[i+1]
               + m_v_RU[i] * p_x[i+m_objSize1d]
               + m_v_LU[i] * p_x[i+m_objSize2d];
   }

   for (size_t i=m_objSize1d;i<m_objSize2d;++i)
   {
      p_y[i] =   m_v[i]    * p_x[i]
               + m_v_RL[i] * p_x[i-m_objSize1d]
               + m_v_CL[i] * p_x[i-1]
               + m_v_CU[i] * p_x[i+1]
               + m_v_RU[i] * p_x[i+m_objSize1d]
               + m_v_LU[i] * p_x[i+m_objSize2d];
   }

   for (size_t i=m_objSize2d;i<m_objSize3d-m_objSize2d;++i)
   {
      p_y[i] =   m_v[i]    * p_x[i]
               + m_v_LL[i] * p_x[i-m_objSize2d]
               + m_v_RL[i] * p_x[i-m_objSize1d]
               + m_v_CL[i] * p_x[i-1]
               + m_v_CU[i] * p_x[i+1]
               + m_v_RU[i] * p_x[i+m_objSize1d]
               + m_v_LU[i] * p_x[i+m_objSize2d];
   }

   for (size_t i=m_objSize3d-m_objSize2d;i<m_objSize3d-m_objSize1d;++i)
   {
      p_y[i] =   m_v[i]    * p_x[i]
               + m_v_LL[i] * p_x[i-m_objSize2d]
               + m_v_RL[i] * p_x[i-m_objSize1d]
               + m_v_CL[i] * p_x[i-1]
               + m_v_CU[i] * p_x[i+1]
               + m_v_RU[i] * p_x[i+m_objSize1d];
   }

   for (size_t i=m_objSize3d-m_objSize1d;i<m_objSize3d-1;++i)
   {
      p_y[i] =   m_v[i]    * p_x[i]
               + m_v_LL[i] * p_x[i-m_objSize2d]
               + m_v_RL[i] * p_x[i-m_objSize1d]
               + m_v_CL[i] * p_x[i-1]
               + m_v_CU[i] * p_x[i+1];
   }

   p_y[m_objSize3d-1] =   m_v[m_objSize3d-1]    * p_x[m_objSize3d-1]
                        + m_v_LL[m_objSize3d-1] * p_x[m_objSize3d-1-m_objSize2d]
                        + m_v_RL[m_objSize3d-1] * p_x[m_objSize3d-1-m_objSize1d]
                        + m_v_CL[m_objSize3d-1] * p_x[m_objSize3d-2]
                        + m_v_CU[m_objSize3d-1] * p_x[m_objSize3d];
}

template <typename ValueType>
CSparseBandMatrix<ValueType>::~CSparseBandMatrix()
{
   delete [] m_v_LL;
   delete [] m_v_RL;
   delete [] m_v_CL;
   delete [] m_v;
}
