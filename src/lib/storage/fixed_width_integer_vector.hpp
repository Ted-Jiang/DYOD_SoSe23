//
// Created by Jiang, Yang on 2024/3/5.
//
#include "abstract_attribute_vector.hpp"
#include "types.hpp"

#pragma once
namespace opossum {
template <typename uintX_t>
class FixedWidthIntegerVector: public AbstractAttributeVector {
  public:
   explicit FixedWidthIntegerVector(size_t size);

   ValueID get(const size_t index) const override;

   void set(const size_t index, const ValueID value_id) override;

   size_t size() const override;

   AttributeVectorWidth width() const override;

   protected:
    std::vector<uintX_t> _values;
};
} // namespace opossum