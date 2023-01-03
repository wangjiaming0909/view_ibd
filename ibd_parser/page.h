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
  const IndexHeader *get_index_header(const Page &pg) const;
  const FSEG_HEADER* get_fseg_header(const Page& pg) const;
  const IndexSystemRecord_INFIMUM* get_infimum(const Page&pg) const;
  const IndexSystemRecord_SUPREMUM *get_supremum(const Page &pg) const;
  IndexHeader index_header;
  FSEG_HEADER fseg_header;
  IndexSystemRecord_INFIMUM infimum;
  IndexSystemRecord_SUPREMUM supremum;
  static void dump(const Page& page, std::ostringstream &oss);
  static const IndexPage* get_index_page(const Page& page);
};

class IIndexPage {
  const IndexHeader& get_index_header() const;
  const FSEG_HEADER& get_fseg_header() const;
  const IndexSystemRecord_INFIMUM& get_infimum() const;
  const IndexSystemRecord_SUPREMUM get_supremum() const;
};

class Page {
public:
  Page(unsigned int page_size, std::streampos offset);
  ~Page();
  inline std::string get_type() const {
    return get_page_type_str(get_fil_header().page_type);
  }
  char *get_buf() { return buf_; }

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
  char *buf_;
};

using PagePtr = std::unique_ptr<Page>;

} // namespace innodb
