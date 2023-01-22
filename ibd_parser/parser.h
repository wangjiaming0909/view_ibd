#pragma once
#include <unordered_set>
#include <unordered_map>
#include "defines.h"


namespace innodb {
class Table;
class Schema;
namespace dd{
class DD_Cache;
}
}

class Parser {
public:
  // mysql data dir
  Parser(const char* path);
  int init();

  innodb::Table* get_table(const char* schema, const char* table);

PRIVATE:
  std::string data_dir_;
  std::unordered_map<std::string, innodb::Schema*> schemas_;
  std::unordered_map<std::string, innodb::Table*> tables_;

  innodb::dd::DD_Cache& dd_cache();

PRIVATE:
  // @brief scan the whole folder, fill schemas and tables into parser
  // @return 0 for succeed, -1 for err
  int scan_data_dir();
  bool is_defined_dir(const char* path);

  std::unordered_set<std::string> tbs_;
};
