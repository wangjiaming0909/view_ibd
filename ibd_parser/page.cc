#include "page.h"
#include <atomic>
#include <cassert>
#include <string.h>

namespace innodb {

#define BUF (byte *)(buf_)

size_t PageID_Hash::operator()(const PageID &pg_id) const {
  return std::hash<SpaceID>()(pg_id.space_id) ^
         std::hash<PageNO>()(pg_id.page_no);
}

bool operator==(const PageID &l, const PageID &r) {
  return l.space_id == r.space_id && l.page_no == r.page_no;
}

Page::Page(unsigned int page_size, byte *d)
    : buf_page_t(d), page_size_(page_size) {}

buf_page_t::buf_page_t(byte* buf) : buf_(buf), page_id_(0,0), page_lock_(), old_(false){}

void Page::dump(std::ostringstream &oss) const {
  FILHeader::dump(BUF, oss);
  auto pg_t_str = get_page_type_str(FILHeader::page_type(BUF));
  auto pg_type = FILHeader::page_type(BUF);
  switch (pg_type) {
  case FIL_PAGE_INDEX:
    IndexPage::dump(BUF, oss);
  default:
    break;
  }
}

PAGE_TYPE buf_page_t::page_type() const {
  return (PAGE_TYPE)FILHeader::page_type(BUF);
}

Page::~Page() {}

void IndexPage::dump(const byte *b, std::ostringstream &oss) {
  IndexHeader::dump(b, oss);
  FSEG_HEADER::dump(b, oss);
  Records::dump(b, oss);
  IndexPageDirectory::dump(b, oss);
}

bool buf_page_t::fix_page() {
  bool expect = false;
  return fixed.compare_exchange_strong(expect, true,
                                       std::memory_order::memory_order_seq_cst);
}

bool buf_page_t::unfix_page() {
  bool expect = true;
  return fixed.compare_exchange_strong(expect, false,
                                       std::memory_order::memory_order_seq_cst);
}

record_ptr_t buf_page_t::first_rec() {
  auto * const infimum = BUF + PAGE_NEW_INFIMUM;
  return std::make_shared<Record>(BUF + RecordHeader::next_offs(infimum), this);
}

} // namespace innodb
