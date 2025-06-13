#pragma once
#include "headers.h"
#include <cassert>
#include <glog/logging.h>
#include <map>
#include <memory>
#include <string>

namespace innodb {

enum class PageType {
  UNKNOWN,
  FSP_HDR,
  IBUF_BITMAP,
  INODE,
  XDES_HDR,
  DATA_PAGE,
  INDEX_PAGE,
  SDI,
};

class Page;

class Page {
public:
  static void init_page(const byte *buf, Page **page);
  Page(const byte *buf, unsigned int page_size, std::streampos offset);
  ~Page();
  virtual PageType get_type() const = 0;
  const byte *get_buf() const { return buf_; }
  virtual void init(const byte *buf);

  const FILHeader &get_fil_header() const { return fil_header_; }
  virtual void dump(std::ostringstream &oss) const;

  virtual const XDES_E *get_xdes_entry(uint32_t) { return nullptr; }
  virtual const INode_E *get_inode_entry(uint32_t) const { return nullptr; }

  const byte *buf() const { return (const byte *)buf_; }

private:
  FILHeader fil_header_;
  unsigned int page_size_; // bytes
  std::streampos offset_;
  const byte *buf_;
  unsigned char *buf_allocated_;
};

using PagePtr = std::unique_ptr<Page>;

struct FSPHeaderPage : public Page {
  FSPHeaderPage(const byte *buf, std::streampos offset = 0,
                unsigned int page_size = PAGE_SIZE)
      : Page(buf, page_size, offset) {
    xdes_arr_.resize(256); // TODO check if page with other PAGE_SIZE has 256
                           // extent entries too.
  }
  void init(const byte *buf) override;
  PageType get_type() const override { return PageType::FSP_HDR; }
  uint32_t page_num() const { return fsp_header_.fsp_size_; }

  void dump(std::ostringstream &oss) const override;
  const FSPHeader &get_fsp_header() const { return fsp_header_; }
  const ListBaseNode &get_free_list_base_node() const {
    return fsp_header_.free_list_base_node_;
  }
  const ListBaseNode &get_free_frag_list_base_node() const {
    return fsp_header_.free_frag_list_base_node_;
  }
  const ListBaseNode &get_full_frag_list_base_node() const {
    return fsp_header_.full_frag_list_base_node_;
  }
  const ListBaseNode &get_full_inodes_list_base_node() const {
    return fsp_header_.full_inodes_list_base_node_;
  }
  const ListBaseNode &get_free_inodes_list_base_node() const {
    return fsp_header_.free_inodes_list_base_node_;
  }
  const XDES_E *get_xdes_entry(uint32_t index) override;
  FSPHeader fsp_header_;
  std::vector<XDES_E> xdes_arr_;
};

struct XDESPage : public Page {
  XDESPage(const byte *buf, std::streampos offset = 0,
           unsigned int page_size = PAGE_SIZE)
      : Page(buf, page_size, offset), xdes_arr_() {}
  void init(const byte *buf) override { Page::init(buf); }
  PageType get_type() const override { return PageType::XDES_HDR; }
  void dump(std::ostringstream &oss) const override { Page::dump(oss); }
  std::vector<XDES_E> xdes_arr_;
};

struct INodePage : public Page {
  INodePage(const byte *buf, std::streampos offset = 0,
            unsigned int page_size = PAGE_SIZE)
      : Page(buf, page_size, offset), list_node_for_INODE_page_list_(),
        inode_arr_() {}
  void init(const byte *buf) override {
    Page::init(buf);
    list_node_for_INODE_page_list_.init(buf + FILHeader::FIL_PAGE_DATA);
    init_inode_entries(buf + FILHeader::FIL_PAGE_DATA +
                       ListNode::LIST_NODE_SIZE);
  }
  PageType get_type() const override { return PageType::INODE; }
  void dump(std::ostringstream &oss) const override {
    Page::dump(oss);
    oss << "ListNode for INODE page list: ";
    list_node_for_INODE_page_list_.dump(oss);
    dump_inode_entries(oss);
  }
  const INode_E *get_inode_entry(uint32_t index) const override {
    if (index >= INODE_E_COUNT) {
      LOG(ERROR) << "Index out of bounds: " << index;
      return nullptr;
    }
    return &inode_arr_[index];
  }
  INodeEntryListNode list_node_for_INODE_page_list_;
  std::vector<INode_E> inode_arr_;
  static constexpr unsigned int INODE_E_COUNT = 85; // 85 entries per page
  static constexpr unsigned int INODE_ENTRY_OFFSET =
      FILHeader::FIL_PAGE_DATA + ListNode::LIST_NODE_SIZE;
  void dump_inode_entries(std::ostringstream &oss) const;

private:
  void init_inode_entries(const byte *buf);
};

struct IndexPage : public Page {
  IndexPage(const byte *buf, std::streampos offset = 0,
            unsigned int page_size = PAGE_SIZE)
      : Page(buf, page_size, offset) {}
  void init(const byte *buf) override {
    Page::init(buf);
    index_header_.init(buf);
    fseg_header_.init(buf);
  }
  void dump(std::ostringstream &oss) const override {
    Page::dump(oss);
    index_header_.dump(oss);
    fseg_header_.dump(oss);
    oss << std::endl;
  }
  PageType get_type() const override { return PageType::INDEX_PAGE; }
  IndexHeader index_header_;
  FSEG_HEADER fseg_header_;
  static constexpr int32_t FSEG_HEADER_OFFSET = FILHeader::FIL_PAGE_DATA + 36;
};

struct IBufBitMapPage : public Page {
  IBufBitMapPage(const byte *buf, std::streampos offset = 0,
                 unsigned int page_size = PAGE_SIZE)
      : Page(buf, page_size, offset) {}
  void init(const byte *buf) override { Page::init(buf); }
  void dump(std::ostringstream &oss) const override { Page::dump(oss); }
  PageType get_type() const override { return PageType::IBUF_BITMAP; }
  struct IBUFBITMAP {
    int free_space : 2;
    int buffered_flag : 1;
    int change_buf_fg : 1;
  };
  IBUFBITMAP bitmap[16384];
};

struct SDIPage : public Page {
  SDIPage(const byte *buf, std::streampos offset = 0,
          unsigned int page_size = PAGE_SIZE)
      : Page(buf, page_size, offset) {}
  void init(const byte *buf) override { Page::init(buf); }
  void dump(std::ostringstream &oss) const override { Page::dump(oss); }
  PageType get_type() const override { return PageType::SDI; }
};

struct SYSPage : public Page {
  DictHeader dict_header_;
  SYSPage(const byte *buf, std::streampos offset = 0,
          unsigned int page_size = PAGE_SIZE)
      : Page(buf, page_size, offset) {}
  void init(const byte *buf) override {
    Page::init(buf);
    dict_header_.init(buf);
  }
  void dump(std::ostringstream &oss) const override { Page::dump(oss); }
  PageType get_type() const override { return PageType::UNKNOWN; }
};

} // namespace innodb
