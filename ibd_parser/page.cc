#include "page.h"
#include "defines.h"
#include <string.h>
#include <cassert>

using namespace innodb;

Page::Page(unsigned int page_size, std::streampos offset)
    : page_size_(page_size), offset_(offset), buf_(nullptr) {
  buf_ = new char[page_size];
  memset(buf_, 0, page_size);
}

void Page::dump(std::ostringstream &oss) const {
  get_fil_header().dump(oss);
  auto pg_t_str = get_page_type_str(e(get_fil_header().page_type));
  auto pg_type = e(get_fil_header().page_type);
  switch(pg_type){
    case FIL_PAGE_INDEX:
      IndexPage::dump(*this, oss);
    default:
      break;
  }
}

Page::~Page() { delete[] buf_; }


void IndexPage::dump(const Page& page, std::ostringstream &oss) {
  const auto *index_page = get_index_page(page);
  index_page->get_index_header(page)->dump(oss);
  index_page->get_fseg_header(page)->dump(oss);
  index_page->get_infimum(page)->dump(oss);
  index_page->get_supremum(page)->dump(oss);
}

const IndexPage* IndexPage::get_index_page(const Page& page) {
  return page.get<IndexPage>(FILHeader::FILHEADER_SIZE);
}

const IndexHeader *IndexPage::get_index_header(const Page &pg) const {
  return pg.get<IndexHeader>(FILHeader::FILHEADER_SIZE);
}
const FSEG_HEADER* IndexPage::get_fseg_header(const Page& pg) const {
  return pg.get<FSEG_HEADER>(FILHeader::FILHEADER_SIZE +
                             IndexHeader::INDEX_HEADER_SIZE);
}
const IndexSystemRecord_INFIMUM* IndexPage::get_infimum(const Page&pg) const {
  return pg.get<IndexSystemRecord_INFIMUM>(
      FILHeader::FILHEADER_SIZE + IndexHeader::INDEX_HEADER_SIZE +
      FSEG_HEADER::FSEG_HEADER_SIZE);
}
const IndexSystemRecord_SUPREMUM *IndexPage::get_supremum(const Page &pg) const {
  return pg.get<IndexSystemRecord_SUPREMUM>(
      FILHeader::FILHEADER_SIZE + IndexHeader::INDEX_HEADER_SIZE +
      IndexSystemRecord_INFIMUM::INDEX_SYSTEM_RECORD_SIZE +
      FSEG_HEADER::FSEG_HEADER_SIZE);
}
