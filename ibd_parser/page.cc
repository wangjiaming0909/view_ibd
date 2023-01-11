#include "page.h"
#include <string.h>
#include <cassert>

using namespace innodb;

#define BUF (byte*)(buf_)

Page::Page(unsigned int page_size, std::streampos offset)
    : page_size_(page_size), offset_(offset), buf_(nullptr), buf_allocated_(nullptr) {
  buf_allocated_ = new unsigned char[page_size * 2];
  unsigned long ptr = (unsigned long)(buf_allocated_ + page_size - 1);
  unsigned long s = page_size - 1;
  auto mask = ~s;
  buf_ = (unsigned char*)(ptr & mask);
  memset(buf_, 0, page_size);
}

void Page::dump(std::ostringstream &oss) const {
  FILHeader::dump(BUF, oss);
  auto pg_t_str = get_page_type_str(FILHeader::page_type(BUF));
  auto pg_type = FILHeader::page_type(BUF);
  switch(pg_type){
    case FIL_PAGE_INDEX:
      IndexPage::dump(BUF, oss);
    default:
      break;
  }
}

Page::~Page() { delete[] buf_allocated_; }


void IndexPage::dump(const byte* b, std::ostringstream &oss) {
    IndexHeader::dump(b, oss);
    FSEG_HEADER::dump(b, oss);
    Records::dump(b, oss);
    IndexPageDirectory::dump(b, oss);
}

