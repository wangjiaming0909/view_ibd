#pragma once

#include <sys/types.h>
#include "table.h"

namespace innodb {

class Record {
  void *data;
  FieldType type;
};

class Tuple {
  uint len;
  uint field_no;

  public:
  int compare(Record &rec);
};
} // namespace innodb
