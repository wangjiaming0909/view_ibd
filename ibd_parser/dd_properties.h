#pragma once

#include "dd.h"
#include "dd_table.h"

#define DD_PROPETIES_PAGE_NO 4

namespace innodb {
namespace dd {

class DD_Properties : public DD_Abstract_Table {
public:
  DD_Properties();

  static constexpr const char* NAME = "mysql.dd_properties";

  int get_property(const char* key, std::string& ret);

  int load_properties();

private:
  void init();

  std::unordered_map<std::string, std::string> properties_map_;
};

} // namespace dd
} // namespace innodb
