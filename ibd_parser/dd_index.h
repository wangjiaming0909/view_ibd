#pragma once

#include "dd.h"
#include <vector>

namespace innodb {
namespace dd {
class DD_Table;
class Index_Element;

enum class IndexType : uint8_t { PRIMARY, UNIQUE };
enum class IndexAlgorithm : uint8_t {BTREE};

class DD_Index {
private:
  IndexType type_;
  DD_Table *table_;
  uint32_t ordinal_pos_;
  std::vector<Index_Element*> elements_;
};
} // namespace dd
} // namespace innodb
