#pragma once

#include "file_space_reader.h"
#include <string>
#include <unordered_map>
namespace innodb {
class TableReader {
  std::string file_name_;
  FileSpaceReader fsp_reader_;
  TableReader *ibdata1_reader_ = nullptr;

public:
  TableReader(const char *file, TableReader *ibdata1_reader);
  void dump() { fsp_reader_.dump_space(); }
  void dump_page(unsigned int index);
};

class MySQLDataReader {
  std::string data_dir_;
  std::unordered_map<std::string, TableReader *> table_readers_;

  std::string ibdata1_file_;
  TableReader *ibdata1_reader_ = nullptr;

public:
  MySQLDataReader(const char *data_dir)
      : data_dir_(data_dir), table_readers_() {
    ibdata1_file_ = data_dir_ + "/ibdata1";
    ibdata1_reader_ = new TableReader(ibdata1_file_.c_str(), nullptr);
  }
  ~MySQLDataReader() {
    for (auto &pair : table_readers_) {
      delete pair.second; // Clean up allocated TableReader objects
    }
    delete ibdata1_reader_; // Clean up ibdata1_reader_
  }

  TableReader *get_table_reader(const char *db_name, const char *table_name);
};

} // namespace innodb