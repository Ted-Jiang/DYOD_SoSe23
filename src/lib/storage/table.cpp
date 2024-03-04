#include "table.hpp"

#include "resolve_type.hpp"
#include "utils/assert.hpp"
#include "value_segment.hpp"

namespace opossum {

Table::Table(const ChunkOffset target_chunk_size)
    : _chunks{}, _column_names{}, _column_types{}, _column_nullable{}, _target_chunk_size(target_chunk_size) {
  create_new_chunk();
}

// Notice:
//push_back：这个方法接受一个已经构造的对象，并将其复制或移动到容器的末尾。如果你传递的是一个临时对象或者可以被移动的对象，
//    那么push_back会尽可能地使用移动语义以提高效率。但是，如果你传递的对象不能被移动（只能被复制），或者需要先创建一个对象然后再添加到容器中，
//    那么push_back可能会导致额外的复制操作。
//emplace_back：这个方法接受一组参数，并在容器的末尾直接构造一个新的对象，避免了不必要的复制或移动操作。
//    这意味着，如果你要添加到容器的对象的构造函数接受多个参数，你可以直接将这些参数传递给emplace_back，它会在容器的末尾直接构造出新的对象
void Table::add_column_definition(const std::string& name, const std::string& type, const bool nullable) {
  _column_names.emplace_back(name);
  _column_types.emplace_back(type);
  _column_nullable.emplace_back(nullable);
}

void Table::add_column(const std::string& name, const std::string& type, const bool nullable) {
  Assert(row_count() == 0, "Cannot add column definition to non-empty table");
  for (const auto& chunk : _chunks) {
    auto new_segment = std::shared_ptr<AbstractSegment>{};
    resolve_data_type(type, [&](auto data_type) {
      using DataType = typename decltype(data_type)::type;
      new_segment = std::make_shared<ValueSegment<DataType>>(nullable);
    });
    chunk->add_segment(new_segment);
  }
  add_column_definition(name, type, nullable);
}

void Table::create_new_chunk() {
  auto new_chunk = std::make_shared<Chunk>();
  for (auto index = uint16_t{0}; index < _column_names.size(); ++index) {
    auto new_segment = std::shared_ptr<AbstractSegment>{};
    resolve_data_type(_column_types[index], [&](auto data_type) {
      using DataType = typename decltype(data_type)::type;
      new_segment = std::make_shared<ValueSegment<DataType>>(_column_nullable[index]);
    });
    new_chunk->add_segment(new_segment);
  }
  _chunks.emplace_back(new_chunk);
}

void Table::append(const std::vector<AllTypeVariant>& values) {
  Assert(values.size() == _column_names.size(), "Number of values does not match number of columns.");
  if (_chunks.back()->size() >= _target_chunk_size) {
    create_new_chunk();
  }
  _chunks.back()->append(values);
}

ColumnCount Table::column_count() const {
  /*static_cast<ColumnCount>是C++中的一种类型转换操作，它将_column_names.size()的返回值（通常是size_t类型）转换为ColumnCount类型
   * static_cast是C++中四种类型转换操作之一，其他三种类型转换操作如下：
    * dynamic_cast：主要用于类层次间的上行转换和下行转换，还可以用于类之间的交叉转换。
    * const_cast：用于修改类型的const或volatile属性。
    * reinterpret_cast：用于进行各种不同类型的指针和引用的转换，转换有可能产生运行时错误
*/
  return static_cast<ColumnCount>(_column_names.size());
}

uint64_t Table::row_count() const {
  return ((_chunks.size() - 1) * _target_chunk_size) + _chunks.back()->size();
}

ChunkID Table::chunk_count() const {
  return ChunkID{_chunks.size()};
}

ColumnID Table::column_id_by_name(const std::string& column_name) const {
  const auto column_iterator = std::find(_column_names.begin(), _column_names.end(), column_name);
  Assert(column_iterator != _column_names.end(), "Column with name: " + column_name + " doesn't exist.");
  return static_cast<ColumnID>(std::distance(_column_names.begin(), column_iterator));
}

ChunkOffset Table::target_chunk_size() const {
  return _target_chunk_size;
}

const std::vector<std::string>& Table::column_names() const {
  return _column_names;
}

const std::string& Table::column_name(const ColumnID column_id) const {
  return _column_names.at(column_id);
}

const std::string& Table::column_type(const ColumnID column_id) const {
  return _column_types.at(column_id);
}

bool Table::column_nullable(const ColumnID column_id) const {
  return _column_nullable.at(column_id);
}

std::shared_ptr<Chunk> Table::get_chunk(ChunkID chunk_id) {
  return _chunks.at(chunk_id);
}

std::shared_ptr<const Chunk> Table::get_chunk(ChunkID chunk_id) const {
  return _chunks.at(chunk_id);
}

// GCOVR_EXCL_START
void Table::compress_chunk(const ChunkID chunk_id) {
  // Implementation goes here
  Fail("Implementation is missing.");
}
// GCOVR_EXCL_STOP

}  // namespace opossum
