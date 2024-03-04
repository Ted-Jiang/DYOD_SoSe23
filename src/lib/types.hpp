#pragma once

#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <tuple>
#include <vector>

#include "strong_typedef.hpp"

/**
 * We use STRONG_TYPEDEF to avoid things like adding chunk ids and value ids.
 * Because implicit constructors are deleted, you cannot initialize a ChunkID
 * like this
 *   ChunkID x = 3;
 * but need to use
 *   ChunkID x{3}; or
 *   auto x = ChunkID{3};
 *
 * WorkerID, TaskID, CommitID, and TransactionID are used in std::atomics and
 * therefore need to be trivially copyable. That's currently not possible with
 * the strong typedef (as far as I know).
 *
 * TODO(anyone): Also, strongly typing ChunkOffset causes a lot of errors in
 * the group key and adaptive radix tree implementations. Unfortunately, I
 * wasn’t able to properly resolve these issues because I am not familiar with
 * the code there
 */

STRONG_TYPEDEF(uint32_t, ChunkID);
STRONG_TYPEDEF(uint16_t, ColumnID);
STRONG_TYPEDEF(opossum::ColumnID::base_type, ColumnCount);
STRONG_TYPEDEF(uint32_t, ValueID);  // Cannot be larger than ChunkOffset

namespace opossum {

using ChunkOffset = uint32_t;
using AttributeVectorWidth = uint8_t;

constexpr ChunkOffset INVALID_CHUNK_OFFSET{std::numeric_limits<ChunkOffset>::max()};
constexpr ChunkID INVALID_CHUNK_ID{std::numeric_limits<ChunkID::base_type>::max()};

struct RowID {
  ChunkID chunk_id;
  ChunkOffset chunk_offset;

  // Faster than row_id == NULL_ROW_ID, since we only compare the ChunkOffset.
  bool is_null() const {
    return chunk_offset == INVALID_CHUNK_OFFSET;
  }

  // Joins need to use RowIDs as keys for maps.
  bool operator<(const RowID& rhs) const {
    return std::tie(chunk_id, chunk_offset) < std::tie(rhs.chunk_id, rhs.chunk_offset);
  }

  bool operator==(const RowID& rhs) const {
    return std::tie(chunk_id, chunk_offset) == std::tie(rhs.chunk_id, rhs.chunk_offset);
  }
};

// Declaring one part of a RowID as invalid would suffice to represent NULL values. However, this way we add an extra
// safety net which ensures that NULL values are handled correctly. E.g., getting a chunk with INVALID_CHUNK_ID
// immediately crashes.
constexpr RowID NULL_ROW_ID = RowID{INVALID_CHUNK_ID, INVALID_CHUNK_OFFSET};

// Even though ValueIDs do not have to use the full width of ValueID (uint32_t), this will also work for smaller ValueID
// types (uint8_t, uint16_t) since after a down-cast INVALID_VALUE_ID will look like their numeric_limit::max().
constexpr ValueID INVALID_VALUE_ID{std::numeric_limits<ValueID::base_type>::max()};

enum class ScanType { OpEquals, OpNotEquals, OpLessThan, OpLessThanEquals, OpGreaterThan, OpGreaterThanEquals };

using PosList = std::vector<RowID>;

// Prevents unnecessary, potentially expensive, copies by deleting copy constructor and copy assignment operator.
class Noncopyable {
 protected:
  // 这是默认构造函数，它允许创建Noncopyable的实例
  Noncopyable() = default;
  // 这是移动构造函数，它允许创建Noncopyable的实例
  Noncopyable(Noncopyable&&) = default;
  // 这是移动赋值运算符，它允许将一个Noncopyable实例的所有权从一个对象移动到另一个对象
  // 如果你不写Noncopyable& operator=(Noncopyable&&) = default;这一行，
  // C++编译器将不会为你的类自动生成移动赋值运算符。这意味着，你将无法使用移动赋值语法来将一个Noncopyable实例的所有权从一个对象移动到另一个对象。
  // 在你的类中，如果你没有显式地声明移动赋值运算符，但你声明了复制构造函数或复制赋值运算符（在你的Noncopyable类中，这两个都被删除了），
  // 那么编译器就不会为你的类自动生成移动赋值运算符。  因此，如果你希望你的类支持移动赋值，你需要显式地声明移动赋值运算符，就像你在Noncopyable类中所做的那样。
  Noncopyable& operator=(Noncopyable&&) = default;
  ~Noncopyable() = default;
  // 这是拷贝构造函数，它被删除了，因此不能创建Noncopyable的拷贝
  Noncopyable(const Noncopyable&) = delete;
  //这是复制-赋值运算符，但它也被删除了，这意味着你不能通过赋值来复制Noncopyable的实例。
  const Noncopyable& operator=(const Noncopyable&) = delete;
};

}  // namespace opossum
