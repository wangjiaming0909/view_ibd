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


using SpaceID = uint64_t;
using PageNO = uint64_t;
using Rec = std::byte;
struct PageID {
  SpaceID space_id;
  PageNO page_no;
};

class Page {
public:
  Page(unsigned int page_size);
  ~Page();
  inline std::string get_type() const {
    return get_page_type_str(FILHeader::page_type((byte*)buf_));
  }
  unsigned char *get_buf() { return buf_; }

  void dump(std::ostringstream &oss) const;
  void set_space_id(SpaceID sp_id) { page_id_.space_id = sp_id; }
  void set_page_no(PageNO pg_no) { page_id_.page_no = pg_no; }

private:
  unsigned int page_size_; // bytes
  unsigned char *buf_;
  unsigned char *buf_allocated_;
  PageID page_id_;
};

using PagePtr = std::unique_ptr<Page>;

} // namespace innodb
