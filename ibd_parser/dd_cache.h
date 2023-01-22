#pragma once
#include <unordered_map>

namespace innodb {
namespace dd {
class DD_Table;

class DD_Cache {
public:
  static DD_Cache& instance();
  DD_Table* open_dd_table(const char* name);
private:
  DD_Cache();
  ~DD_Cache();
  DD_Cache* instance_;

  std::unordered_map<std::string, DD_Table*> table_cache_;
};
} // namespace dd
} // namespace innodb
