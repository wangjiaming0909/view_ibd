#include "parser.h"
#include "dd_cache.h"
#include <filesystem>
#include <glog/logging.h>

Parser::Parser(const char *path) : data_dir_(path) {}

int Parser::init() {
  auto ret = scan_data_dir();
  return ret;
}

innodb::Table *Parser::get_table(const char *schema, const char *tb_name) {
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
        // iterator all tables
        for (auto &p2 : std::filesystem::directory_iterator(p.path())) {
          if (p2.is_regular_file()) {
            auto tb_path = p2.path();
            std::string schema_tb = tb_path.parent_path().filename().c_str();
            schema_tb += ".";
            schema_tb += p2.path().filename().replace_extension();
            tbs_.insert(schema_tb);
            LOG(INFO) << "found a tb: " << schema_tb;
          }
        }
      }
    }
  } while (0);
  return ret;
}

bool Parser::is_defined_dir(const char *path) {
  std::filesystem::path p = path;
  static std::unordered_set<std::string> defined_dir = {
      "information_schema", "mysql", "performance_schema", "sys",
      "#innodb_temp"};
  return defined_dir.find(p.filename()) != defined_dir.end();
}
