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
class CLinearStencilTiling : public ILinearOperator<ValueType>
{
 private:
    std::size_t m_objCols;
    std::size_t m_objRows;
    std::size_t m_objLevels;
    std::size_t m_objCols_tiling;
    std::size_t m_objRows_tiling;
    std::size_t m_objLevels_tiling;
    std::size_t m_objSize1d;
    std::size_t m_objSize2d;
    std::size_t m_objSize3d;
    const ValueType m_factor;

 public:
    CLinearStencilTiling(
      const std::size_t p_objCols,
      const std::size_t p_objRows,
      const std::size_t p_objLevels,
      const std::size_t p_objCols_tiling,
      const std::size_t p_objRows_tiling,
      const std::size_t p_objLevels_tiling,
      const ValueType p_c = ValueType(1.0),
      const ValueType p_h = ValueType(1.0),
      const ValueType p_tau = ValueType(1.0)
      );
      inline static const std::string IDENTIFER = "linear_stencil_tiling";
    void apply(const ValueType * __restrict__ p_x, ValueType * __restrict__ p_y) const;
    ~CLinearStencilTiling();
};

template <typename ValueType, typename VecType>
CLinearStencilTiling<ValueType, VecType>::CLinearStencilTiling(
   const std::size_t p_objCols,
   const std::size_t p_objRows,
   const std::size_t p_objLevels,
   const std::size_t p_objCols_tiling,
   const std::size_t p_objRows_tiling,
   const std::size_t p_objLevels_tiling,
   const ValueType p_c,
   const ValueType p_h,
   const ValueType p_tau
   ):
m_objCols(p_objCols),
m_objRows(p_objRows),
m_objLevels(p_objLevels),
m_objSize1d(p_objCols),
m_objCols_tiling(p_objCols_tiling),
m_objRows_tiling(p_objRows_tiling),
m_objLevels_tiling(p_objLevels_tiling),
m_objSize2d(p_objCols * p_objRows),
m_objSize3d(p_objCols * p_objRows * p_objLevels),
m_factor(p_c*p_tau/(p_h*p_h))
{

}

template <typename ValueType, typename VecType>
void CLinearStencilTiling<ValueType, VecType>::apply(const ValueType * __restrict__ p_x, ValueType * __restrict__ p_y) const
{
   //---------------------------------------------------------------
   // Notes: corners
   //---------------------------------------------------------------

   std::size_t l_pos;

   //
   // C0
   //
   l_pos = 0;
   p_y[l_pos] = (1+3*m_factor) * p_x[l_pos]
               -    m_factor  * p_x[l_pos+1]
               -    m_factor  * p_x[l_pos+m_objSize1d]
               -    m_factor  * p_x[l_pos+m_objSize2d];

   //
   // C1
   //
   l_pos = m_objSize1d - 1;
   p_y[l_pos] = (1+3*m_factor) * p_x[l_pos]
               -    m_factor  * p_x[l_pos-1]
               -    m_factor  * p_x[l_pos+m_objSize1d]
               -    m_factor  * p_x[l_pos+m_objSize2d];

   //
   // C2
   //
   l_pos = m_objSize2d-m_objSize1d;
   p_y[l_pos] = (1+3*m_factor) * p_x[l_pos]
               -    m_factor  * p_x[l_pos-m_objSize1d]
               -    m_factor  * p_x[l_pos+1]
               -    m_factor  * p_x[l_pos+m_objSize2d];

   //
   // C3
   //
   l_pos = m_objSize2d-1;
   p_y[l_pos] = (1+3*m_factor) * p_x[l_pos]
               -    m_factor  * p_x[l_pos-m_objSize1d]
               -    m_factor  * p_x[l_pos-1]
               -    m_factor  * p_x[l_pos+m_objSize2d];

   //
   // C4
   //
   l_pos = m_objSize3d-m_objSize2d;
   p_y[l_pos] = (1+3*m_factor) * p_x[l_pos]
               -    m_factor  * p_x[l_pos-m_objSize2d]
               -    m_factor  * p_x[l_pos+1]
               -    m_factor  * p_x[l_pos+m_objSize1d];

   //
   // C5
   //
   l_pos = m_objSize3d-m_objSize2d+m_objSize1d-1;
   p_y[l_pos] = (1+3*m_factor) * p_x[l_pos]
               -    m_factor  * p_x[l_pos-m_objSize2d]
               -    m_factor  * p_x[l_pos-1]
               -    m_factor  * p_x[l_pos+m_objSize1d];

   //
   // C6
   //
   l_pos = m_objSize3d-m_objSize1d;
   p_y[l_pos] = (1+3*m_factor) * p_x[l_pos]
               -    m_factor  * p_x[l_pos-m_objSize2d]
               -    m_factor  * p_x[l_pos-m_objSize1d]
               -    m_factor  * p_x[l_pos+1];

   //
   // C7
   //
   l_pos = m_objSize3d-1;
   p_y[l_pos] = (1+3*m_factor) * p_x[l_pos]
               -    m_factor  * p_x[l_pos-m_objSize2d]
               -    m_factor  * p_x[l_pos-m_objSize1d]
               -    m_factor  * p_x[l_pos-1];

   #pragma omp parallel
   {
      VecType l_x_LL_Vec;
      VecType l_x_RL_Vec;
      VecType l_x_CL_Vec;
      VecType l_x_Vec;
      VecType l_x_CU_Vec;
      VecType l_x_RU_Vec;
      VecType l_x_LU_Vec;
      VecType l_y_Vec;

      //---------------------------------------------------------------
      // Notes: edges
      //---------------------------------------------------------------

      //
      // E0
      //
      #pragma omp for
      for (std::size_t i=1; i<m_objSize1d-1; ++i)
      {
         p_y[i] = (1+4*m_factor) * p_x[i]
                  -    m_factor  * p_x[i-1]
                  -    m_factor  * p_x[i+1]
                  -    m_factor  * p_x[i+m_objSize1d]
                  -    m_factor  * p_x[i+m_objSize2d];
      }

      //
      // E1
      //
      #pragma omp for
      for (std::size_t i=m_objSize1d; i<m_objSize2d-(2*m_objSize1d)+1; i+=m_objSize1d)
      {
         p_y[i] = (1+4*m_factor) * p_x[i]
                  -    m_factor  * p_x[i-m_objSize1d]
                  -    m_factor  * p_x[i+1]
                  -    m_factor  * p_x[i+m_objSize1d]
                  -    m_factor  * p_x[i+m_objSize2d];
      }

      //
      // E2
      //
      #pragma omp for
      for (std::size_t i=2*m_objSize1d-1; i<m_objSize2d-m_objSize1d; i+=m_objSize1d)
      {
         p_y[i] = (1+4*m_factor) * p_x[i]
                  -    m_factor  * p_x[i-m_objSize1d]
                  -    m_factor  * p_x[i-1]
                  -    m_factor  * p_x[i+m_objSize1d]
                  -    m_factor  * p_x[i+m_objSize2d];
      }

      //
      // E3
      //
      #pragma omp for
      for (std::size_t i=m_objSize2d-m_objSize1d+1; i<m_objSize2d-1; ++i)
      {
         p_y[i] = (1+4*m_factor) * p_x[i]
                  -    m_factor  * p_x[i-m_objSize1d]
                  -    m_factor  * p_x[i-1]
                  -    m_factor  * p_x[i+1]
                  -    m_factor  * p_x[i+m_objSize2d];
      }

      //
      // E4
      //
      #pragma omp for
      for (std::size_t i=m_objSize2d; i<m_objSize3d-2*m_objSize2d+1; i+=m_objSize2d)
      {
         p_y[i] = (1+4*m_factor) * p_x[i]
                  -    m_factor  * p_x[i-m_objSize2d]
                  -    m_factor  * p_x[i+1]
                  -    m_factor  * p_x[i+m_objSize1d]
                  -    m_factor  * p_x[i+m_objSize2d];
      }

      //
      // E5
      //
      #pragma omp for
      for (std::size_t i=m_objSize2d+m_objSize1d-1; i<m_objSize3d-(2*m_objSize2d)+m_objSize2d; i+=m_objSize2d)
      {
         p_y[i] = (1+4*m_factor) * p_x[i]
                  -    m_factor  * p_x[i-m_objSize2d]
                  -    m_factor  * p_x[i-1]
                  -    m_factor  * p_x[i+m_objSize1d]
                  -    m_factor  * p_x[i+m_objSize2d];
      }

      //
      // E6
      //
      #pragma omp for
      for (std::size_t i=2*m_objSize2d-m_objSize1d; i<m_objSize3d-m_objSize2d-m_objSize1d+1; i+=m_objSize2d)
      {
         p_y[i] = (1+4*m_factor) * p_x[i]
                  -    m_factor  * p_x[i-m_objSize2d]
                  -    m_factor  * p_x[i-m_objSize1d]
                  -    m_factor  * p_x[i+1]
                  -    m_factor  * p_x[i+m_objSize2d];
      }

      //
      // E7
      //
      #pragma omp for
      for (std::size_t i=2*m_objSize2d-1; i<m_objSize3d-m_objSize2d; i+=m_objSize2d)
      {
         p_y[i] = (1+4*m_factor) * p_x[i]
                  -    m_factor  * p_x[i-m_objSize2d]
                  -    m_factor  * p_x[i-m_objSize1d]
                  -    m_factor  * p_x[i-1]
                  -    m_factor  * p_x[i+m_objSize2d];
      }

      //
      // E8
      //
      #pragma omp for
      for (std::size_t i=m_objSize3d-m_objSize2d+1; i<m_objSize3d-m_objSize2d+m_objSize1d-1; ++i)
      {
         p_y[i] = (1+4*m_factor) * p_x[i]
                  -    m_factor  * p_x[i-m_objSize2d]
                  -    m_factor  * p_x[i-1]
                  -    m_factor  * p_x[i+1]
                  -    m_factor  * p_x[i+m_objSize1d];
      }

      //
      // E9
      //
      #pragma omp for
      for (std::size_t i=m_objSize3d-m_objSize2d+m_objSize1d; i<m_objSize3d-(2*m_objSize1d)+1; i+=m_objSize1d)
      {
         p_y[i] = (1+4*m_factor) * p_x[i]
                  -    m_factor  * p_x[i-m_objSize2d]
                  -    m_factor  * p_x[i-m_objSize1d]
                  -    m_factor  * p_x[i+1]
                  -    m_factor  * p_x[i+m_objSize1d];
      }

      //
      // E10
      //
      #pragma omp for
      for (std::size_t i=m_objSize3d-m_objSize2d+(2*m_objSize1d)-1; i<m_objSize3d-m_objSize1d; i+=m_objSize1d)
      {
         p_y[i] = (1+4*m_factor) * p_x[i]
                  -    m_factor  * p_x[i-m_objSize2d]
                  -    m_factor  * p_x[i-m_objSize1d]
                  -    m_factor  * p_x[i-1]
                  -    m_factor  * p_x[i+m_objSize1d];
      }

      //
      // E11
      //
      #pragma omp for
      for (std::size_t i=m_objSize3d-m_objSize1d+1; i<m_objSize3d-1; ++i)
      {
         p_y[i] = (1+4*m_factor) * p_x[i]
                  -    m_factor  * p_x[i-m_objSize2d]
                  -    m_factor  * p_x[i-m_objSize1d]
                  -    m_factor  * p_x[i-1]
                  -    m_factor  * p_x[i+1];
      }

      //---------------------------------------------------------------
      // Note: surfaces
      //---------------------------------------------------------------

      //
      // S0
      //
      #pragma omp for
      for (std::size_t i=m_objSize1d; i<m_objSize2d-(2*m_objSize1d)+1; i+=m_objSize1d)
      {
         for (std::size_t j=1; j<m_objSize1d-1; ++j)
         {
            p_y[i+j] = (1+5*m_factor) * p_x[i+j]
                     -    m_factor  * p_x[i+j-m_objSize1d]
                     -    m_factor  * p_x[i+j-1]
                     -    m_factor  * p_x[i+j+1]
                     -    m_factor  * p_x[i+j+m_objSize1d]
                     -    m_factor  * p_x[i+j+m_objSize2d];
         }
      }

      //
      // S1
      //
      #pragma omp for
      for (std::size_t i=m_objSize2d; i<m_objSize3d-2*m_objSize2d+1; i+=m_objSize2d)
      {
         for (std::size_t j=1; j<m_objSize1d-1; ++j)
         {
            p_y[i+j] = (1+5*m_factor) * p_x[i+j]
                     -    m_factor  * p_x[i+j-m_objSize2d]
                     -    m_factor  * p_x[i+j-1]
                     -    m_factor  * p_x[i+j+1]
                     -    m_factor  * p_x[i+j+m_objSize1d]
                     -    m_factor  * p_x[i+j+m_objSize2d];
         }
      }

      //
      // S2
      //
      #pragma omp for
      for (std::size_t i=m_objSize2d; i<m_objSize3d-2*m_objSize2d+1; i+=m_objSize2d)
      {
         for (std::size_t j=2*m_objSize1d-1; j<m_objSize2d-m_objSize1d; j+=m_objSize1d)
         {
            p_y[i+j] = (1+5*m_factor) * p_x[i+j]
                     -    m_factor  * p_x[i+j-m_objSize2d]
                     -    m_factor  * p_x[i+j-m_objSize1d]
                     -    m_factor  * p_x[i+j-1]
                     -    m_factor  * p_x[i+j+m_objSize1d]
                     -    m_factor  * p_x[i+j+m_objSize2d];
         }
      }

      //
      // S3
      //
      #pragma omp for
      for (std::size_t i=m_objSize2d; i<m_objSize3d-2*m_objSize2d+1; i+=m_objSize2d)
      {
         for (std::size_t j=m_objSize2d-m_objSize1d+1; j<m_objSize2d-1; ++j)
         {
            p_y[i+j] = (1+5*m_factor) * p_x[i+j]
                     -    m_factor  * p_x[i+j-m_objSize2d]
                     -    m_factor  * p_x[i+j-m_objSize1d]
                     -    m_factor  * p_x[i+j-1]
                     -    m_factor  * p_x[i+j+1]
                     -    m_factor  * p_x[i+j+m_objSize2d];
         }
      }

      //
      // S4
      //
      #pragma omp for
      for (std::size_t i=m_objSize2d; i<m_objSize3d-2*m_objSize2d+1; i+=m_objSize2d)
      {
         for (std::size_t j=m_objSize1d; j<m_objSize2d-(2*m_objSize1d)+1; j+=m_objSize1d)
         {
            p_y[i+j] = (1+5*m_factor) * p_x[i+j]
                     -    m_factor  * p_x[i+j-m_objSize2d]
                     -    m_factor  * p_x[i+j-m_objSize1d]
                     -    m_factor  * p_x[i+j+1]
                     -    m_factor  * p_x[i+j+m_objSize1d]
                     -    m_factor  * p_x[i+j+m_objSize2d];
         }
      }

      //
      // S5
      //
      #pragma omp for
      for (std::size_t i=m_objSize1d; i<m_objSize2d-(2*m_objSize1d)+1; i+=m_objSize1d)
      {
         for (std::size_t j=m_objSize3d-m_objSize2d+1; j<m_objSize3d-m_objSize2d+m_objSize1d-1; ++j)
         {
            p_y[i+j] = (1+5*m_factor) * p_x[i+j]
                     -    m_factor  * p_x[i+j-m_objSize2d]
                     -    m_factor  * p_x[i+j-m_objSize1d]
                     -    m_factor  * p_x[i+j-1]
                     -    m_factor  * p_x[i+j+1]
                     -    m_factor  * p_x[i+j+m_objSize1d];
         }
      }

      //---------------------------------------------------------------
      // Note: inside
      //---------------------------------------------------------------

      #pragma omp for
      for (std::size_t l_iL_t=m_objSize2d; l_iL_t<m_objSize3d-2*m_objSize2d+1; l_iL_t+=m_objSize2d*m_objLevels_tiling)
      {
         for (std::size_t l_iR_t=m_objSize1d; l_iR_t<m_objSize2d-(2*m_objSize1d)+1; l_iR_t+=m_objSize1d*m_objRows_tiling)
         {
            for ( std::size_t l_iC_t=1; l_iC_t<m_objSize1d-1; l_iC_t+=m_objCols_tiling)
            {
               for ( std::size_t l_iL=0; l_iL<m_objSize2d*m_objLevels_tiling; l_iL+=m_objSize2d)
               {
                  for ( std::size_t l_iR=0; l_iR<m_objSize1d*m_objRows_tiling; l_iR+=m_objSize1d)
                  {
                     for ( std::size_t l_iC=0; l_iC<m_objCols_tiling; l_iC+=VecType::size())
                     {
                        l_pos =   l_iL_t
                              + l_iR_t
                              + l_iC_t
                              + l_iL
                              + l_iR
                              + l_iC;

                        l_x_LL_Vec.load(p_x + l_pos - m_objSize2d);
                        l_x_RL_Vec.load(p_x + l_pos - m_objSize1d);
                        l_x_CL_Vec.load(p_x + l_pos - 1          );
                        l_x_Vec.load(   p_x + l_pos              );
                        l_x_CU_Vec.load(p_x + l_pos + 1          );
                        l_x_RU_Vec.load(p_x + l_pos + m_objSize1d);
                        l_x_LU_Vec.load(p_x + l_pos + m_objSize2d);

                        l_y_Vec = (1+6*m_factor) * l_x_Vec
                                 -     m_factor  * l_x_LL_Vec
                                 -     m_factor  * l_x_RL_Vec
                                 -     m_factor  * l_x_CL_Vec
                                 -     m_factor  * l_x_CU_Vec
                                 -     m_factor  * l_x_RU_Vec
                                 -     m_factor  * l_x_LU_Vec;
                        l_y_Vec.store(p_y + l_pos);
                     }
                  }
               }
            }
         }
      }
   }
}

template <typename ValueType, typename VecType>
CLinearStencilTiling<ValueType, VecType>::~CLinearStencilTiling()
{

}
