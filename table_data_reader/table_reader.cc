#include "table_reader.h"
#include "glog/logging.h"

namespace innodb {
TableReader::TableReader(const char *file, TableReader *ibdata1_reader)
    : file_name_(file), fsp_reader_(file), ibdata1_reader_(ibdata1_reader) {}

void TableReader::dump_page(unsigned int index) {
  auto page = fsp_reader_.get_page(index);
  if (page) {
    std::ostringstream oss;
    oss << "Dumping page " << index << " from table: " << file_name_ << "\n";
    page->dump(oss);
    LOG(INFO) << oss.str();
  } else {
    LOG(ERROR) << "Failed to get page " << index
               << " from table: " << file_name_;
  }
}

TableReader *MySQLDataReader::get_table_reader(const char *db_name,
                                               const char *table_name) {
  if (std::string(table_name) == "ibdata1")
    return ibdata1_reader_;
  std::string full_path = data_dir_ + "/" + db_name + "/" + table_name + ".ibd";
  LOG(INFO) << "Reading table from: " << full_path;
  auto it = table_readers_.find(full_path);
  if (it != table_readers_.end()) {
    LOG(INFO) << "Table already exists in cache.";
    return it->second;
  } else {
    TableReader *table_reader =
        new TableReader(full_path.c_str(), ibdata1_reader_);
    LOG(INFO) << "Adding new table to cache.";
    auto result = table_readers_.emplace(full_path, table_reader);
    if (result.second) {
      return result.first->second;
    } else {
      LOG(ERROR) << "Failed to add table reader for: " << full_path;
      return nullptr;
    }
  }
  return 0;
}
} // namespace innodb