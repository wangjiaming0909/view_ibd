#pragma once

#include "headers.h"
#include "ut.h"
#include <sys/types.h>
#include <memory>

namespace innodb {

class buf_page_t;
class Record;
using record_ptr_t = std::shared_ptr<Record>;

class Record {
  byte *rec_;
  buf_page_t* page_;

  public:
  Record(byte* rec, buf_page_t* pg) : rec_(rec), page_(pg) {}
  bool is_infimum() const;
  bool is_supremum() const;
  bool is_node_ptr() const;
  record_ptr_t next_rec();
  uint field_num() const;
  template <typename T>
  int get_field(uint field_pos);
};

class Tuple {
  uint len;
  uint field_no;

  public:
  int compare(Record &rec);
};

inline bool Record::is_infimum() const {
  return RecordHeader::rec_status(rec_) == REC_STATUS_INFIMUM;
}

inline bool Record::is_supremum() const {
  return RecordHeader::rec_status(rec_) == REC_STATUS_SUPREMUM;
}

inline bool Record::is_node_ptr() const {
  return RecordHeader::rec_status(rec_) == REC_STATUS_NODE_PTR;
}

inline record_ptr_t Record::next_rec() {
  return std::make_shared<Record>(RecordHeader::next_offs(rec_), page_);
}

} // namespace innodb
