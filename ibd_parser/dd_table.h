#pragma once
#include "dd.h"
#include <string>
#include "table.h"

namespace innodb {
namespace dd {
class DD_Index;

class DD_Abstract_Table : public Table {
public:
  DD_Abstract_Table(const char *schema, const char *name, PageNO page_no)
      : Table(schema, name), page_no_(page_no), page_id_(DD_SPACE_ID, page_no) {
  }
  virtual ~DD_Abstract_Table() = default;
  virtual int record_oper(byte *) { return 0; }
  virtual bool is_dd_tb() const { return true; }

protected:
  DD_Table_id id_;
  PageNO page_no_;
  PageID page_id_;

  DD_Index *index_;
};

class DD_Tables : public DD_Abstract_Table {
public:
  DD_Tables() : DD_Abstract_Table(DD_SPACE, NAME, 0) {}
  static constexpr const char *NAME = "mysql.tables";
};

} // namespace dd
} // namespace innodb
