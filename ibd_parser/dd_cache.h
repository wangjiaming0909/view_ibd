#pragma once
#include "table_loader.h"
#include <unordered_map>
#include <glog/logging.h>

namespace innodb {
namespace dd {
class DD_Abstract_Table;

class DD_Cache {
public:
  static DD_Cache& instance();
  template <typename TABLE>
  TABLE* get_dd_table();
private:
  DD_Cache();
  ~DD_Cache();

  // @brief load this dd table from mysql.ibd,
  // generate the corresponding DD_Table
  template <typename TABLE>
  TABLE* open_dd_table();
  // @brief add this tb into cache
  // @note will always success, some table could be evicted out
  void add_to_cache(DD_Abstract_Table* tb);
  // @brief add DD_Properties into tb_cache
  void init_dd_properties();

  std::unordered_map<std::string, DD_Abstract_Table*> table_cache_;
};

template <typename TABLE>
TABLE* DD_Cache::get_dd_table() {
  auto tb_name = TABLE::NAME;
  TABLE* ret = nullptr;
  DD_Abstract_Table *tb_ret = nullptr;
  auto it = table_cache_.find(tb_name);
  if (it == table_cache_.end()) {
    // load table
    tb_ret = open_dd_table<TABLE>();
    if (tb_ret) {
      // add it to cache
    }
  } else {
    tb_ret = it->second;
  }

  // return the pointer
  if (tb_ret) ret = dynamic_cast<TABLE*>(tb_ret);
  return ret;
}

template <typename TABLE>
TABLE* DD_Cache::open_dd_table() {
  TABLE *tb = new TABLE();
  TableLoader loader(*tb);

  auto ret = loader.load();

  if (ret != 0) {
    delete tb;
    tb = nullptr;
  }
  return tb;
}
} // namespace dd
} // namespace innodb
