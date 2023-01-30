#pragma once

#include <sys/types.h>
#include "table.h"

namespace innodb {

class Record {
  void *data_;
  Table* tb_;

  public:
  uint field_num() const ;
  template <typename T>
  int get_field(uint field_pos);
};

class Tuple {
  uint len;
  uint field_no;

  public:
  int compare(Record &rec);
};
} // namespace innodb
