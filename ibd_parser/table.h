#pragma once
#include "dd_index.h"
#include <string>

namespace innodb {

class Schema {
private:
  std::string name;
};

class Table;

class Index {
  SpaceID sp_id_;
  uint n_uniqs_;
  Table *tb_;

public:
  friend class Table;
  template <typename Func>
  void scan(Func&& func);
  // @brief search the key
  // @return 0 if found, -1 if not found
  template <typename Func, typename Key>
  int search(Func&& func, const Key& key);
};

class Table {
  // @brief open the table from dd, load it's meta data
  void open();

  Index* get_cluster_idx();

private:
  std::string schema;
  std::string name;
  std::string fil_name_;

  Schema *schema_;

  SpaceID space_id_;

  std::vector<Index*> indexes_;

  dd::DD_Index idx_;
};

template <typename Func>
void Index::scan(Func&& func) {

}

template <typename Func, typename Key>
int Index::search(Func &&func, const Key& key) {

}
} // namespace innodb
