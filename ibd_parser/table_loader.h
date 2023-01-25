#pragma once

namespace innodb {

namespace dd {
class DD_Cache;
}
class Table;

class TableLoader {
public:
  TableLoader(Table &tb);
  ~TableLoader() = default;

  int load();

private:
  dd::DD_Cache& dd_cache();
  int fill_properties();
  int fill_table();
  int fill_index();

private:
  Table &tb_;
};

} // namespace innodb
