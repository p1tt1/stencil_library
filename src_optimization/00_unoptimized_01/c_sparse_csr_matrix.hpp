
#pragma once

#include <string>
#include <iostream>
#include <iomanip>

#include "i_linear_operator.hpp"

template<typename ValueType>
class CSparseCSRMatrix : public ILinearOperator<ValueType>
{
 private:
    const std::size_t m_size;
    ValueType * m_values;
    std::size_t * m_rowIds;
    std::size_t * m_colIds;
    ValueType get(
        const std::size_t p_row,
        const std::size_t p_col
    ) const;
    void set(
        const std::size_t p_row,
        const std::size_t p_col,
        const ValueType& p_value
    );
 public:
    CSparseCSRMatrix(
      const std::size_t p_objCols,
      const std::size_t p_objRows,
      const std::size_t p_objLevels,
      const ValueType p_c = ValueType(1.0),
      const ValueType p_h = ValueType(1.0),
      const ValueType p_tau = ValueType(1.0)
    );
    inline static const std::string IDENTIFER = "sparse_csr_matrix";
    inline static const std::size_t ELEMENTS_PER_ROW = 7;
    void apply(
      const ValueType * __restrict__ p_x,
      ValueType * __restrict__ p_y
    ) const;
    void print() const;
   ~CSparseCSRMatrix();
};

template<typename ValueType>
CSparseCSRMatrix<ValueType>::CSparseCSRMatrix(
      const std::size_t p_objCols,
      const std::size_t p_objRows,
      const std::size_t p_objLevels,
      const ValueType p_c,
      const ValueType p_h,
      const ValueType p_tau
) :
m_size(p_objCols * p_objRows * p_objLevels)
{
    // std::cout << "CSparseCSRMatrix::Constructor" << std::endl;
    std::size_t l_objSize1d = p_objCols;
    std::size_t l_objSize2d = l_objSize1d * p_objRows;
    ValueType l_factor = p_c*p_tau/(p_h*p_h);

    m_values = new ValueType[m_size * ELEMENTS_PER_ROW];
    m_rowIds = new std::size_t[m_size + 1];
    m_colIds = new std::size_t[m_size * ELEMENTS_PER_ROW];

    for (std::size_t i = 0; i < m_size * ELEMENTS_PER_ROW; ++i)
    {
        m_values[i] = ValueType();
        m_colIds[i] = 0;
    }
    for (std::size_t i = 0; i < m_size + 1; ++i)
    {
        m_rowIds[i] = 0;
    }

    for (std::size_t i = 1; i <= m_size; ++i)
    {
        m_rowIds[i] = m_rowIds[i-1] + ELEMENTS_PER_ROW;
    }
    for (std::size_t i = 0; i < m_size; ++i)
    {
        m_colIds[m_rowIds[i]] = 1;
    }

    for(std::size_t i=0; i<p_objLevels; ++i)
    {
        for(std::size_t j=0; j<p_objRows; ++j)
        {
            for(std::size_t k=0; k<p_objCols; ++k)
            {
                std::size_t l_Pos = i*l_objSize2d + j*l_objSize1d + k;

                ValueType l_value = ValueType();
                if(k > 0)
                {
                    this->set(l_Pos, l_Pos-1, -l_factor);
                    l_value++;
                }
                if(k < p_objCols-1)
                {
                    this->set(l_Pos, l_Pos+1, -l_factor);
                    l_value++;
                }

                if(j > 0)
                {
                    this->set(l_Pos, l_Pos-l_objSize1d, -l_factor);
                    l_value++;
                }
                if(j < p_objRows-1)
                {
                    this->set(l_Pos, l_Pos+l_objSize1d, -l_factor);
                    l_value++;
                }

                if(i > 0)
                {
                    this->set(l_Pos, l_Pos-l_objSize2d, -l_factor);
                    l_value++;
                }
                if(i < p_objLevels-1)
                {
                    this->set(l_Pos, l_Pos+l_objSize2d, -l_factor);
                    l_value++;
                }

                this->set(l_Pos, l_Pos, 1 + l_factor*l_value);
            }
        }
    }
}

template <typename ValueType>
ValueType CSparseCSRMatrix<ValueType>::get(
    const std::size_t p_row,
    const std::size_t p_col
) const
{
    // std::cout << "CSparseCSRMatrix::get(" << p_row << "," << p_col << ")" << std::endl;
#ifdef _CHECKBOUNDS_
    if (p_row < 0 || p_col < 0 || p_row >= m_size || p_col >= m_size)
    {
        throw std::out_of_range("Matrix access row index out of range");
    }
#endif
    if (p_row == p_col) {
        return m_values[m_rowIds[p_row]];
    }
    std::size_t l_posStart=m_rowIds[p_row];
    std::size_t l_posEnd = l_posStart + m_colIds[l_posStart];
    for( std::size_t i = l_posStart+1; i < l_posEnd; ++i )
    {
        if ( m_colIds[i] == p_col )
        {
            return m_values[i];
        }
    }
    return 0;
}

template<typename ValueType>
void CSparseCSRMatrix<ValueType>::set(
    const std::size_t p_row,
    const std::size_t p_col,
    const ValueType& p_value
)
{
    // std::cout << "CSparseCSRMatrix::set" << std::endl;
    // std::cout << "p_row: " << p_row << std::endl;
    // std::cout << "p_col: " << p_col << std::endl;
#ifdef _CHECKBOUNDS_
    if (p_row < 0 || p_col < 0 || p_row >= m_size || p_col >= m_size)
    {
        throw std::out_of_range("Matrix access row index out of range");
    }
#endif
    if ( p_row == p_col )
    {
        m_values[m_rowIds[p_row]] = p_value;
        return;
    }
    std::size_t l_posStart = m_rowIds[p_row];
    std::size_t l_posEnd = l_posStart + m_colIds[l_posStart];
    for(std::size_t i = l_posStart+1; i < l_posEnd; ++i)
    {
        if (m_colIds[i] == p_col)
        {
            m_values[i] = p_value;
            return;
        }
    }
    if (l_posEnd >= m_rowIds[p_row+1] )
    {
        std::cout << "BÄ;M" << std::endl;
        throw std::out_of_range("Matrix access row index out of range");
    }
    ++(m_colIds[l_posStart]);
    m_colIds[l_posEnd] = p_col;
    m_values[l_posEnd] = p_value;
    return;
}

template <typename ValueType>
void CSparseCSRMatrix<ValueType>::apply(
   const ValueType * __restrict__ p_x,
   ValueType * __restrict__ p_y
) const
{
    for (std::size_t i = 0; i < m_size; ++i){
        ValueType l_value = ValueType();
        std::size_t l_posStart = m_rowIds[i];
        std::size_t l_posEnd = l_posStart + m_colIds[l_posStart];
        l_value +=  m_values[l_posStart] * p_x[i];
        for (size_t j = l_posStart+1; j < l_posEnd; ++j)
        {
            l_value +=  m_values[j] * p_x[m_colIds[j]];
        }
        p_y[i] = l_value;
    }
    return;
}

template <typename ValueType>
inline void CSparseCSRMatrix<ValueType>::print() const
{
    constexpr std::size_t PRINT_VALUE_GAP = 2;

    std::cout << "(" << m_size << "x";
    std::cout << m_size << ") matrix:" << std::endl;
    for (std::size_t i = 0; i < m_size; ++i)
    {
        std::cout << std::setprecision(3);
        for (std::size_t j = 0; j < m_size; ++j)
        {
            std::cout << std::setw(PRINT_VALUE_GAP) << this->get(i,j) << " ";
        }
        std::cout << std::endl;
    }
}

template <typename ValueType>
CSparseCSRMatrix<ValueType>::~CSparseCSRMatrix()
{
   delete [] m_values;
   delete [] m_rowIds;
   delete [] m_colIds;
}
