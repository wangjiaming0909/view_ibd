#pragma once
#include "headers.h"
#include <map>
#include <memory>
#include <string>
#include <glog/logging.h>

namespace innodb {

enum class PageType {
  UNKNOWN,
  FSP_HDR,
  IBUF_BITMAP,
  INODE,
  XDES_HDR,
  DATA_PAGE,
  INDEX_PAGE
};

class Page;

struct FSPHeaderPage {
  FSPHeader header;
  XDES_E xdes[256];
  void dump(std::ostringstream &oss) const;
};

struct IBufBitMapPage {
  struct IBUFBITMAP {
    int free_space : 2;
    int buffered_flag : 1;
    int change_buf_fg : 1;
  };
  IBUFBITMAP bitmap[16384];
};

struct INodePage {
  unsigned char list_node_for_inode_page_list[16];
  INode_E inodes[85];
};

struct IndexPage {
  static void dump(const byte* page, std::ostringstream &oss);
};

class Page {
public:
  Page(unsigned int page_size, std::streampos offset);
  ~Page();
  inline std::string get_type() const {
    return get_page_type_str(FILHeader::page_type((byte*)buf_));
  }
  unsigned char *get_buf() { return buf_; }

  template <typename T> const T *get(unsigned int offset) const {
    LOG(INFO) << typeid(T).name()
              << " offset: " << offset_ + offset;
    return reinterpret_cast<T *>(buf_ + offset);
  }

  std::string get_str(unsigned int offset, unsigned int size) {
    return std::string(buf_[offset], size);
  }

  const FILHeader &get_fil_header() const { return *((FILHeader *)buf_); }
  void dump(std::ostringstream &oss) const;

private:
  unsigned int page_size_; // bytes
  std::streampos offset_;
  unsigned char *buf_;
  unsigned char *buf_allocated_;
};

using PagePtr = std::unique_ptr<Page>;

} // namespace innodb
