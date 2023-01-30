#pragma once
#include "data.h"
#include "headers.h"
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <glog/logging.h>
#include <boost/intrusive/list.hpp>

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
  PageID(SpaceID sp_id = 0, PageNO pg_no = 0)
      : space_id(sp_id), page_no(pg_no) {}
  SpaceID space_id;
  PageNO page_no;
  std::string dump() const {
    return std::to_string(space_id) + std::string(":") +
           std::to_string(page_no);
  }
};

bool operator==(const PageID&l, const PageID&r);

struct PageID_Hash {
  size_t operator()(const PageID &pg_id) const;
};

using page_list_mem_hook = boost::intrusive::list_member_hook<
    boost::intrusive::constant_time_size<true>>;

class buf_page_t {
public:
  buf_page_t(byte* buf);
  byte* get_buf() {return buf_;}
  bool fix_page();
  bool unfix_page();
  void lock_page() {page_lock_.lock();}
  void unlock_page() {page_lock_.unlock();}
  bool locked_by_me() const {return true;}
  void set_old(bool old) {old_ = old;}
  PageID get_page_id() const { return page_id_; }
  page_list_mem_hook list_hook;
  PAGE_TYPE page_type() const;
  record_ptr_t first_rec();
PROTECTED:
  byte* buf_;
  std::atomic_bool fixed = false;
  PageID page_id_;
  lock_t page_lock_;
  bool old_;
};

class Page : public buf_page_t {
public:
  friend class buffer_pool_t;
  Page(unsigned int page_size, byte* d);
  ~Page();
  void dump(std::ostringstream &oss) const;

private:
  unsigned int page_size_; // bytes
};

using page_list_hook_options =
    boost::intrusive::member_hook<buf_page_t, page_list_mem_hook, &buf_page_t::list_hook>;

using page_list_t = boost::intrusive::list<buf_page_t, page_list_hook_options>;

using PagePtr = std::unique_ptr<Page>;

} // namespace innodb
