#pragma once
#include "dd.h"
#include <string>

namespace innodb {
namespace dd {
class DD_Index;

class DD_Table {
public:
  DD_Table(const char *schema, const char *name, PageNO page_no);
  ~DD_Table();

private:
  std::string name_;
  std::string schema_;
  DD_Table_id id_;
  PageNO page_no_;

  DD_Index *index_;
};
} // namespace dd
} // namespace innodb
