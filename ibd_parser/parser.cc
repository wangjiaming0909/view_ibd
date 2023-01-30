#include "parser.h"
#include "dd_cache.h"
#include "table.h"
#include <filesystem>
#include <glog/logging.h>

Parser::Parser(const char *path) : data_dir_(path) {}

Parser::~Parser() {
  for (auto &p : schemas_)
    delete p.second;
  for (auto &p : tables_)
    delete p.second;
}

int Parser::init() {
  return scan_data_dir();
}

innodb::Table *Parser::get_table(const char *schema, const char *tb_name) {
  std::string schema_tb_name = schema;
  schema_tb_name += ".";
  schema_tb_name += tb_name;
  auto it = tables_.find(schema_tb_name);
  if (it != tables_.end()) {
    if (it->second->is_inited()) {
      innodb::TableLoader loader(*it->second);
      if (0 != loader.load()) return nullptr;
    }
    return it->second;
  }
  return nullptr;
}

innodb::dd::DD_Cache &Parser::dd_cache() {
  return innodb::dd::DD_Cache::instance();
}

int Parser::scan_data_dir() {
  int ret = 0;
  auto path = std::filesystem::path(data_dir_);
  do {
    if (!std::filesystem::exists(path)) {
      ret = -1;
      LOG(ERROR) << "parser err, " << data_dir_ << " not exists";
      break;
    }

    if (!std::filesystem::is_directory(data_dir_)) {
      ret = -1;
      LOG(ERROR) << "parser err, " << data_dir_ << " is not a directory";
      break;
    }

    // scan all files and folders
    for (auto &p : std::filesystem::directory_iterator(path)) {
      if (p.is_directory() && !is_defined_dir(p.path().c_str())) {
        std::string schema_name = p.path().filename().c_str();
        auto *schema = new innodb::Schema(schema_name.c_str());
        schemas_[schema_name] = schema;
        // iterator all tables
        for (auto &p2 : std::filesystem::directory_iterator(p.path())) {
          if (p2.is_regular_file()) {
            auto tb_name = p2.path().filename().replace_extension();
            std::string schema_tb = schema_name + ".";
            schema_tb += tb_name;
            tbs_.insert(schema_tb);
            auto *table =
                new innodb::Table(schema_name.c_str(), tb_name.c_str());
            schema->add_table(tb_name.c_str(), table);
            tables_[schema_tb] = table;
            LOG(INFO) << "found a tb: " << schema_tb;
          }
        }
      }
    }
  } while (0);
  return ret;
}

int Parser::init_tables() {

}

bool Parser::is_defined_dir(const char *path) {
  std::filesystem::path p = path;
  static std::unordered_set<std::string> defined_dir = {
      "information_schema", "mysql", "performance_schema", "sys",
      "#innodb_temp"};
  return defined_dir.find(p.filename()) != defined_dir.end();
}
