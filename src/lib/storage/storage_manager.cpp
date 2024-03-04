#include "storage_manager.hpp"

#include <utility>

#include "utils/assert.hpp"

namespace opossum {

StorageManager& StorageManager::get() {
  static StorageManager storage_manager;
  return storage_manager;
}

void StorageManager::add_table(const std::string& name, std::shared_ptr<Table> table) {
  Assert(!has_table(name), "Table already exists");
  _tables[name] = std::move(table);
}

void StorageManager::drop_table(const std::string& name) {
  Assert(has_table(name), "Table does not exist");
  _tables.erase(name);
}

std::shared_ptr<Table> StorageManager::get_table(const std::string& name) const {
  Assert(has_table(name), "Table does not exist");
  return _tables.at(name);
}

bool StorageManager::has_table(const std::string& name) const {
  // Implementation goes here
  return _tables.contains(name);
}

std::vector<std::string> StorageManager::table_names() const {
  // Implementation goes here
  auto keys = std::vector<std::string>{};
  for (auto const& [key, value] : _tables) {
    keys.push_back(key);
  }
  return keys;
}

void StorageManager::print(std::ostream& out) const {
  for (auto const& [table_name, table] : _tables) {
    out << "=== " << table_name << " ===" << std::endl;
    out << "#columns: " << table->column_count() << std::endl;
    out << "#rows: " << table->row_count() << std::endl;
    out << "#chunks: " << table->chunk_count() << std::endl;
    out << "columns:" << std::endl;
    const auto column_count = table->column_count();
    for (auto column_id = ColumnID{0}; column_id < column_count; column_id++) {
      out << "  " << table->column_name(column_id) << " (" << table->column_type(column_id) << ")" << std::endl;
    }
  }
}

void StorageManager::reset() {
  // Implementation goes here
  _tables.clear();
}

}  // namespace opossum
