#pragma once

#include "dd.h"
#include "dd_table.h"

namespace innodb {
namespace dd {

class DD_Properties : public DD_Table {
public:
  static constexpr PageNO page_no = 4;
  DD_Properties() : DD_Table(DD_SPACE, "dd_properties", page_no) {}
};
} // namespace dd
} // namespace innodb
