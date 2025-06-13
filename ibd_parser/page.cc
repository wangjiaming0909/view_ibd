#include "page.h"
#include <cassert>
#include <string.h>

using namespace innodb;

Page::Page(const byte *buf, unsigned int page_size, std::streampos offset)
    : page_size_(page_size), offset_(offset), buf_(buf) {}

void Page::dump(std::ostringstream &oss) const {
  oss << "---------------------------------------------\n";
  this->get_fil_header().dump(oss);
}

Page::~Page() {}

void FSPHeaderPage::dump(std::ostringstream &oss) const {
  Page::dump(oss);
  get_fsp_header().dump(oss);
}

const XDES_E *FSPHeaderPage::get_xdes_entry(uint32_t index) {
  if (!xdes_arr_[index].inited()) {
    std::streampos offset = FILHeader::FIL_PAGE_DATA +
                            FSPHeader::FSP_HEADER_SIZE +
                            index * XDES_E::XDES_E_SIZE;
    xdes_arr_[index].init(buf() + offset);
  }
  return &xdes_arr_[index];
}

void Page::init_page(const byte *buf, Page **page) {
  FILHeader header;
  header.init_fil_header(buf);
  Page *p = nullptr;
  switch (header.page_type_) {
  case FIL_PAGE_TYPE_FSP_HDR: {
    p = new FSPHeaderPage(buf, 0, PAGE_SIZE);
    break;
  }
  case FIL_PAGE_TYPE_INODE: {
    p = new INodePage(buf, 0, PAGE_SIZE);
    break;
  }
  case FIL_PAGE_INDEX: {
    p = new IndexPage(buf, 0, PAGE_SIZE);
    break;
  }
  case FIL_PAGE_IBUF_BITMAP: {
    p = new IBufBitMapPage(buf, 0, PAGE_SIZE);
    break;
  }
  case FIL_PAGE_TYPE_SDI: {
    p = new SDIPage(buf, 0, PAGE_SIZE);
    break;
  }
  case FIL_PAGE_TYPE_SYS: {
    p = new SYSPage(buf, 0, PAGE_SIZE);
  }
  default:
    break;
  }
  p->init(buf);
  (*page) = p;
}

void Page::init(const byte *buf) { this->fil_header_.init_fil_header(buf); }

void FSPHeaderPage::init(const byte *buf) {
  Page::init(buf);
  this->fsp_header_.init(buf);
}

void INodePage::init_inode_entries(const byte *buf) {
  uint32_t inode_count = 0;
  INode_E inode{};
  while (inode_count < INodePage::INODE_E_COUNT) {
    inode.init(buf);
    if (!inode.is_valid_inode_entry()) {
      break;
    }
    inode_arr_.push_back(inode);
    buf += INode_E::INODE_ENTRY_SIZE;
    ++inode_count;
  }
}

void INodePage::dump_inode_entries(std::ostringstream &oss) const {
  oss << "INode entries: " << inode_arr_.size() << "\n";
  for (size_t i = 0; i < inode_arr_.size(); ++i) {
    const INode_E &inode = inode_arr_[i];
    inode.dump(oss, Addr{get_fil_header().page_number_offset_,
                         static_cast<uint16_t>(FILHeader::FIL_PAGE_DATA +
                                               ListNode::LIST_NODE_SIZE +
                                               i * INode_E::INODE_ENTRY_SIZE)});
  }
}