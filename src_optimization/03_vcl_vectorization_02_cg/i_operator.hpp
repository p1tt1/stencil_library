
#ifndef Included_IOperator_H
#define Included_IOperator_H

template <typename ValueType>
class IOperator
{
 public:
    virtual void apply(const ValueType * __restrict__ p_x, ValueType * __restrict__ p_y) const = 0;
};

#endif      // Included_IOperator_H
