#include "dictionary_segment.hpp"

#include <map>
#include <set>
#include "fixed_width_integer_vector.hpp"
#include "type_cast.hpp"
#include "utils/assert.hpp"
#include "value_segment.hpp"

namespace opossum {

std::shared_ptr<AbstractAttributeVector> get_attribute_vector(size_t vector_size, size_t size) {
  const auto bits_needed = std::bit_width(vector_size - 1);
  Assert(bits_needed <= 32, "Too many values in dictionary, cant use more than 32 bits!");
  if (bits_needed <= 8) {
    return std::make_shared<FixedWidthIntegerVector<uint8_t>>(size);
  }
  if (bits_needed <= 16) {
    return std::make_shared<FixedWidthIntegerVector<uint16_t>>(size);
  }
  return std::make_shared<FixedWidthIntegerVector<uint32_t>>(size);
}

template <typename T>
DictionarySegment<T>::DictionarySegment(const std::shared_ptr<AbstractSegment>& abstract_segment) {
  const auto value_segment = std::dynamic_pointer_cast<ValueSegment<T>>(abstract_segment);
  Assert(value_segment, "DictionarySegment only supports ValueSegments");

  const auto values = value_segment->values();
  const auto size = value_segment->size();

  _is_nullable = value_segment->is_nullable();

  auto dict_map = std::map<T, ValueID>();
  auto last_index = _is_nullable ? 1 : 0;

  for (auto index = size_t{0}; index < size; ++index) {
    if (!value_segment->is_null(index)) {
      const auto value = values[index];
      if (dict_map.find(value) == dict_map.end()) {
        auto inserted = dict_map.insert(std::pair(value, last_index));
        if (inserted.second) {
          last_index++;
        }
      }
    }
  }

  for (auto [key, val] : dict_map) {
    _dictionary.push_back(std::move(key));
  }
  const auto attribute_vector = get_attribute_vector(dict_map.size(), size);

  for (auto index = size_t{0}; index < size; ++index) {
    if (value_segment->is_null(index)) {
      attribute_vector->set(index, null_value_id());
    } else {
      const auto dict_value = dict_map[values[index]];
      attribute_vector->set(index, dict_value);
    }
  }
  _attribute_vector = attribute_vector;
}

template <typename T>
AllTypeVariant DictionarySegment<T>::operator[](const ChunkOffset chunk_offset) const {
  const auto return_type = get_typed_value(chunk_offset);
  if (return_type) {
    return *return_type;
  }
  return NULL_VALUE;
}

template <typename T>
T DictionarySegment<T>::get(const ChunkOffset chunk_offset) const {
  const auto return_value = get_typed_value(chunk_offset);
  Assert(return_value.has_value(), "Value at position: " + std::to_string(chunk_offset) + "is NULL!");
  return *return_value;
}

template <typename T>
std::optional<T> DictionarySegment<T>::get_typed_value(const ChunkOffset chunk_offset) const {
  const auto value_id = attribute_vector()->get(chunk_offset);
  if (value_id == null_value_id()) {
    return std::nullopt;
  }
  return value_of_value_id(value_id);
}

template <typename T>
const std::vector<T>& DictionarySegment<T>::dictionary() const {
  return _dictionary;
}

template <typename T>
std::shared_ptr<const AbstractAttributeVector> DictionarySegment<T>::attribute_vector() const {
  return _attribute_vector;
}

template <typename T>
ValueID DictionarySegment<T>::null_value_id() const {
  return ValueID{0};
}

template <typename T>
const T DictionarySegment<T>::value_of_value_id(const ValueID value_id) const {
  Assert(!(_is_nullable && value_id == null_value_id()), "Can't retrieve value for null value.");
  return dictionary().at(value_id - (_is_nullable ? 1 : 0));
}

template <typename T>
ValueID DictionarySegment<T>::lower_bound(const T value) const {
  auto lower_bound_iterator = std::lower_bound(_dictionary.begin(), _dictionary.end(), value);
  if (lower_bound_iterator == _dictionary.end()) {
    return INVALID_VALUE_ID;
  }
  return ValueID{static_cast<ChunkOffset>(std::distance(_dictionary.begin(), lower_bound_iterator))};
}

template <typename T>
ValueID DictionarySegment<T>::lower_bound(const AllTypeVariant& value) const {
  return lower_bound(type_cast<T>(value));
}

template <typename T>
ValueID DictionarySegment<T>::upper_bound(const T value) const {
  auto upper_bound_iterator = std::upper_bound(_dictionary.begin(), _dictionary.end(), value);
  if (upper_bound_iterator == _dictionary.end()) {
    return INVALID_VALUE_ID;
  }
  return ValueID{static_cast<ChunkOffset>(std::distance(_dictionary.begin(), upper_bound_iterator))};
}

template <typename T>
ValueID DictionarySegment<T>::upper_bound(const AllTypeVariant& value) const {
  return upper_bound(type_cast<T>(value));
}

template <typename T>
ChunkOffset DictionarySegment<T>::unique_values_count() const {
  return _dictionary.size();
}

template <typename T>
ChunkOffset DictionarySegment<T>::size() const {
  return static_cast<ChunkOffset>(attribute_vector()->size());
}

template <typename T>
size_t DictionarySegment<T>::estimate_memory_usage() const {
  auto dict_size = sizeof(T) * dictionary().size();
  auto att_vec_size = attribute_vector()->width() * attribute_vector()->size();
  return dict_size + att_vec_size;
}

EXPLICITLY_INSTANTIATE_DATA_TYPES(DictionarySegment);

}  // namespace opossum
