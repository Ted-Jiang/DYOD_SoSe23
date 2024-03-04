#pragma once

#include "abstract_segment.hpp"

namespace opossum {

// ValueSegment is a segment type that stores all its values in a vector.
template <typename T>
class ValueSegment : public AbstractSegment {
 public:
  explicit ValueSegment(bool nullable = false);

  // Returns the value at a certain position. If you want to write efficient operators, back off!
  //当final关键字在方法声明的末尾时，表示该方法不能在任何派生类中被重写。
  // 这主要用于虚函数。例如，AllTypeVariant operator[](const ChunkOffset chunk_offset) const final表示这个方法在派生类中不能被重写。
  AllTypeVariant operator[](const ChunkOffset chunk_offset) const final;

  // Returns whether a value is NULL.
  // const：当const关键字在方法声明的末尾时，表示该方法是一个常量成员函数，它不能修改对象的任何数据成员（除非它们被声明为mutable）
  // 这意味着你可以在一个常量对象上调用这个方法。
  bool is_null(const ChunkOffset chunk_offset) const;

  // Returns the value at a certain position. Throws an error if value is NULL.
  T get(const ChunkOffset chunk_offset) const;

  // Returns the value at a certain position. Returns std::nullopt if the value is NULL.
  std::optional<T> get_typed_value(const ChunkOffset chunk_offset) const;

  // Adds a value at the end of the segment.
  void append(const AllTypeVariant& value);

  // Returns the number of entries.
  ChunkOffset size() const final;

  // Returns all values. This is the preferred method to check a value at a certain index. Usually you need to access
  // more than a single value anyway.
  // e.g. const auto& values = value_segment.values(); and then: values[i]; in your loop.
  const std::vector<T>& values() const;

  // Returns whether segment supports NULL values.
  bool is_nullable() const;

  // Returns NULL value vector that indicates whether a value is NULL with true at position i. Throw an exception if
  // is_nullable() returns false. This is the preferred method to check for a NULL value at a certain index. Usually
  // you need to access more than a single value anyway.
  const std::vector<bool>& null_values() const;

  // Returns the calculated memory usage.
  size_t estimate_memory_usage() const final;

 protected:
  // Implementation goes here
};

EXPLICITLY_DECLARE_DATA_TYPES(ValueSegment);

}  // namespace opossum
