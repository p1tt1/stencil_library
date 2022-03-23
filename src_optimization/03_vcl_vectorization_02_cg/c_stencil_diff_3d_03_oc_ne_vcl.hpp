/*
 * test memoryless approach without condition statements
 */

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
#include "vcl/vectorclass.h"
#include "i_operator.hpp"

template <typename ValueType>
class CStencilDiff3D_03_Oc_Ne_Vcl : public IOperator<ValueType>
{
 private:
   const std::size_t m_sCols;
   const std::size_t m_sRows;
   const std::size_t m_sLevels;
   const std::size_t m_sCols_tiling;
   const std::size_t m_sRows_tiling;
   const std::size_t m_sLevels_tiling;
   const std::size_t m_size1d;
   const std::size_t m_size2d;
   const std::size_t m_size3d;
   ValueType m_factor;

 public:
   CStencilDiff3D_03_Oc_Ne_Vcl(
      const std::size_t p_sCols,
      const std::size_t p_sRows,
      const std::size_t p_sLevels,
      const std::size_t p_sCols_tiling,
      const std::size_t p_sRows_tiling,
      const std::size_t p_sLevels_tiling,
      const ValueType p_c = ValueType(1),
      const ValueType p_h = ValueType(1),
      const ValueType p_tau = ValueType(1)
);
   inline static const std::string IDENTIFER = "03_oc_ne_vcl";
   void apply(const ValueType * __restrict__ p_x, ValueType * __restrict__ p_y) const;
   ~CStencilDiff3D_03_Oc_Ne_Vcl();
};

template <typename ValueType>
CStencilDiff3D_03_Oc_Ne_Vcl<ValueType>::CStencilDiff3D_03_Oc_Ne_Vcl(
   const std::size_t p_sCols,
   const std::size_t p_sRows,
   const std::size_t p_sLevels,
   const std::size_t p_sCols_tiling,
   const std::size_t p_sRows_tiling,
   const std::size_t p_sLevels_tiling,
   const ValueType p_c,
   const ValueType p_h,
   const ValueType p_tau
):
m_sCols(p_sCols),
m_sRows(p_sRows),
m_sLevels(p_sLevels),
m_sCols_tiling(p_sCols_tiling),
m_sRows_tiling(p_sRows_tiling),
m_sLevels_tiling(p_sLevels_tiling),
m_size1d(p_sCols),
m_size2d(p_sCols * p_sRows),
m_size3d(p_sCols * p_sRows * p_sLevels),
m_factor(p_c*p_tau/(p_h*p_h))
{

}

template <typename ValueType>
void CStencilDiff3D_03_Oc_Ne_Vcl<ValueType>::apply(const ValueType * __restrict__ p_x, ValueType * __restrict__ p_y) const
{
   std::size_t l_thread_id = omp_get_thread_num();
   std::size_t l_nthreads = omp_get_num_threads();
   std::size_t l_IzT_ltb;
   std::size_t l_IzT_utb;
   std::size_t lPos;
   std::size_t lPos_x;
   std::size_t lPos_y;
   std::size_t lPos_z;

   //
   // NOTE:
   //       CL => Col Lower
   //       RL => Row Lower
   //       LL => Level Lower
   //       CU => Col Upper
   //       RU => Row Upper
   //       LU => Level Upper
   //
   ValueType l_c_LL;
   ValueType l_c_RL;
   ValueType l_c_CL;
   ValueType l_c_CU;
   ValueType l_c_RU;
   ValueType l_c_LU;

   Vec4d l_pos_x_Vec;
   Vec4d l_c_CL_Vec;
   Vec4d l_c_CU_Vec;

   Vec4d l_x_LL_Vec;
   Vec4d l_x_RL_Vec;
   Vec4d l_x_CL_Vec;
   Vec4d l_x_Vec;
   Vec4d l_x_CU_Vec;
   Vec4d l_x_RU_Vec;
   Vec4d l_x_LU_Vec;
   Vec4d l_y_Vec;

   l_IzT_ltb = ((m_sLevels) * (l_thread_id)    ) / (l_nthreads * m_sLevels_tiling);
   l_IzT_utb = ((m_sLevels) * (l_thread_id + 1)) / (l_nthreads * m_sLevels_tiling);
   for (                std::size_t lI_zT=l_IzT_ltb; lI_zT<l_IzT_utb;     ++lI_zT)
   {
      //
      // TODO ab hier weiter loop in ++lI_yT umwandeln.... hatte hier beim letzten mal Probleme
      //
      for (             std::size_t lI_yT=0; lI_yT<m_sRows;   lI_yT+=m_sRows_tiling)
      {
         for (          std::size_t lI_xT=0;        lI_xT<m_sCols;                lI_xT+=m_sCols_tiling)
         {
            for (       std::size_t lI_z=0;         lI_z<m_sLevels_tiling; ++lI_z)
            {
               for (    std::size_t lI_y=0;         lI_y<m_sRows_tiling; ++lI_y)
               {
                  for ( std::size_t lI_x=0;         lI_x<m_sCols_tiling;            lI_x+=VEC_SIZE)
                  {
                     lPos_z = lI_zT*m_sLevels_tiling+lI_z;
                     lPos_y = lI_yT+lI_y;
                     lPos_x = lI_xT+lI_x;
                     lPos = lPos_z*m_size2d + lPos_y*m_size1d + lPos_x;

                     l_c_LL = (1-((m_sLevels-1-lPos_z)/(m_sLevels-1)))  *m_factor;
                     l_c_RL = (1-((m_sRows-1-lPos_y)/(m_sRows-1)))      *m_factor;
                     l_c_RU = (1-(lPos_y/(m_sRows-1)))                  *m_factor;
                     l_c_LU = (1-(lPos_z/(m_sLevels-1)))                *m_factor;

                     l_pos_x_Vec = Vec4d(lPos_x, lPos_x+1, lPos_x+2, lPos_x+3);

                     l_c_CL_Vec = select(l_pos_x_Vec>0, m_factor, 0.0);

                     // l_c_CL_Vec = (1-((m_sCols-1-l_pos_x_Vec)/(m_sCols-1)))      *m_factor;
                     // l_c_CL = (1-((m_sCols-1-lPos_x)/(m_sCols-1)))      *m_factor;

                     l_c_CU_Vec = select(l_pos_x_Vec<m_sCols-1, m_factor, 0.0);

                     // l_c_CU_Vec = (1-(l_pos_x_Vec/(m_sCols-1)))                  *m_factor;
                     // l_c_CU = (1-(lPos_x/(m_sCols-1)))                  *m_factor;

                     l_x_LL_Vec.load(p_x + lPos - m_size2d);
                     l_x_RL_Vec.load(p_x + lPos - m_size1d);
                     l_x_CL_Vec.load(p_x + lPos - 1);
                     l_x_Vec.load(p_x + lPos);
                     l_x_CU_Vec.load(p_x + lPos + 1);
                     l_x_RU_Vec.load(p_x + lPos + m_size1d);
                     l_x_LU_Vec.load(p_x + lPos + m_size2d);

                     l_y_Vec = (1+(l_c_LL+l_c_RL+l_c_CL_Vec+l_c_CU_Vec+l_c_RU+l_c_LU)) * l_x_Vec
                                 - l_c_LL                                              * l_x_LL_Vec
                                 - l_c_RL                                              * l_x_RL_Vec
                                 - l_c_CL_Vec                                          * l_x_CL_Vec
                                 - l_c_CU_Vec                                          * l_x_CU_Vec
                                 - l_c_RU                                              * l_x_RU_Vec
                                 - l_c_LU                                              * l_x_LU_Vec;

                     l_y_Vec.store(p_y + lPos);
                     // p_y[lPos] = (1+(l_c_LL+l_c_RL+l_c_CL+l_c_CU+l_c_RU+l_c_LU)) * p_x[lPos]
                     //             -   l_c_LL                                      * p_x[lPos-m_size2d]
                     //             -   l_c_RL                                      * p_x[lPos-m_size1d]
                     //             -   l_c_CL                                      * p_x[lPos-1]
                     //             -   l_c_CU                                      * p_x[lPos+1]
                     //             -   l_c_RU                                      * p_x[lPos+m_size1d]
                     //             -   l_c_LU                                      * p_x[lPos+m_size2d];
                  }
               }
            }
         }
      }
   }
}

template <typename ValueType>
CStencilDiff3D_03_Oc_Ne_Vcl<ValueType>::~CStencilDiff3D_03_Oc_Ne_Vcl()
{
}
