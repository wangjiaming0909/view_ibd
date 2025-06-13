#pragma once
#include "cstring"
#include "defines.h"
#include <bitset>
#include <sstream>
#include <stdint.h>
#include <unordered_map>
#include <vector>

namespace innodb {
class FileSpaceReader;
class Page;
struct XDES_E;
struct INode_E;
struct Addr;

enum PAGE_TYPE {
  FIL_PAGE_TYPE_ALOCATED = 0,
  FIL_PAGE_TYPE_UNUSED = 1,
  FIL_PAGE_UNDO_LOG = 2,
  FIL_PAGE_TYPE_INODE = 3,
  FIL_PAGE_IBUF_FREE_LIST = 4,
  FIL_PAGE_IBUF_BITMAP = 5,
  FIL_PAGE_TYPE_SYS = 6,
  FIL_PAGE_TYPE_TRX_SYS = 7,
  FIL_PAGE_TYPE_FSP_HDR = 8,
  FIL_PAGE_TYPE_XDES = 9,
  FIL_PAGE_TYPE_UNKNOWN = 13,
  FIL_PAGE_TYPE_SDI = 17853,
  FIL_PAGE_RTREE = 17854,
  FIL_PAGE_INDEX = 17855
};

constexpr ulint FIL_ADDR_SIZE = 6;
constexpr ulint FLST_BASE_NODE_SIZE = 4 + 2 * FIL_ADDR_SIZE;

struct ListNode {
  uint32_t prev_page_number_;
  uint16_t prev_offset_;
  uint32_t next_page_number_;
  uint16_t next_offset_;
  static constexpr size_t LIST_NODE_SIZE = 4 + 2 + 4 + 2;
  void init(const byte *p) {
    prev_page_number_ = prev_page_number(p);
    prev_offset_ = prev_offset(p);
    next_page_number_ = next_page_number(p);
    next_offset_ = next_offset(p);
  }
  ListNode() = default;
  static uint32_t prev_page_number(const byte *p) {
    return mach_read_from_4(p);
  }

  static uint16_t prev_offset(const byte *p) { return mach_read_from_2(p + 4); }

  static uint32_t next_page_number(const byte *p) {
    return mach_read_from_4(p + 6);
  }
  static uint16_t next_offset(const byte *p) {
    return mach_read_from_2(p + 6 + 4);
  }

  void dump(std::ostringstream &oss) const {
    oss << "ListNode: prev_page_number: " << prev_page_number_ << "\t"
        << "prev_offset: " << prev_offset_ << "\t"
        << "next_page_number: " << next_page_number_ << "\t"
        << "next_offset: " << next_offset_ << std::endl;
  }
  virtual const ListNode *next(FileSpaceReader *reader) const = 0;
  virtual const XDES_E *xdes(FileSpaceReader *) const { return nullptr; }
  virtual const std::vector<INode_E> *inode_arr(FileSpaceReader *) const {
    return nullptr;
  }
};

struct ListBaseNode {
  uint32_t list_length_;
  uint32_t first_page_number_;
  uint16_t first_offset_;
  uint32_t last_page_number_;
  uint16_t last_offset_;

  void init(const byte *p) {
    list_length_ = list_length(p);
    first_page_number_ = first_page_number(p);
    first_offset_ = first_offset(p);
    last_page_number_ = last_page_number(p);
    last_offset_ = last_offset(p);
  }
  static uint32_t list_length(const byte *p) { return mach_read_from_4(p); }

  static uint32_t first_page_number(const byte *p) {
    return mach_read_from_4(p + 4);
  }

  static uint16_t first_offset(const byte *p) {
    return mach_read_from_2(p + 4 + 4);
  }

  static uint32_t last_page_number(const byte *p) {
    return mach_read_from_4(p + 2 + 4 + 4);
  }

  static uint16_t last_offset(const byte *p) {
    return mach_read_from_2(p + 2 + 4 + 4 + 4);
  }

  void dump(std::ostringstream &oss) const {
    if (list_length_ > 0) {
      oss << "\nListBaseNode: list_length: " << list_length_ << "\t"
          << "first_page_number: " << first_page_number_ << "\t"
          << "first_offset: " << first_offset_ << "\t"
          << "last_page_number: " << last_page_number_ << "\t"
          << "last_offset: " << last_offset_ << "\n";
    } else {
      oss << "(empty list)" << "\n";
    }
  }
  virtual const ListNode *first(FileSpaceReader *reader) const = 0;
  virtual const ListNode *last(FileSpaceReader *reader) const = 0;
};

struct Addr {
  uint32_t page_number_;
  uint16_t offset_;

  Addr() : page_number_(0), offset_(0) {}
  Addr(uint32_t page_number, uint16_t offset)
      : page_number_(page_number), offset_(offset) {}

  bool valid() const { return page_number_ != UINT32_MAX; }

  void dump(std::ostringstream &oss) const {
    oss << "Addr: page_number: " << page_number_ << "\t"
        << "offset: " << offset_ << "\t";
  }
};

struct XDesEntryListNode : public ListNode {
  Addr addr;
  XDesEntryListNode() = default;

  void dump(std::ostringstream &oss) const {
    oss << "XDesEntryListNode: ";
    ListNode::dump(oss);
  }
  const ListNode *next(FileSpaceReader *reader) const override;
  const XDES_E *xdes(FileSpaceReader *reader) const override;
};

struct XDesEntryList : public ListBaseNode {
  int a;
  XDesEntryList() = default;

  void dump(std::ostringstream &oss) const {
    oss << "XDesEntryList: ";
    ListBaseNode::dump(oss);
  }
  const ListNode *get_xdes_entry_list_node(FileSpaceReader *reader,
                                           Addr &addr) const;
  const ListNode *first(FileSpaceReader *reader) const override;
  const ListNode *last(FileSpaceReader *reader) const override;
};

struct INodeEntryListNode : public ListNode {
  int a;
  Addr addr;
  INodeEntryListNode() = default;
  void dump(std::ostringstream &oss) const {
    oss << "INodeListNode: ";
    ListNode::dump(oss);
  }
  const ListNode *next(FileSpaceReader *reader) const override;
  const std::vector<INode_E> *inode_arr(FileSpaceReader *reader) const override;
};

struct INodeEntryList : public ListBaseNode {
  int a;
  INodeEntryList() = default;

  void dump(std::ostringstream &oss) const {
    oss << "INodeEntryList: ";
    ListBaseNode::dump(oss);
  }
  const ListNode *first(FileSpaceReader *reader) const override;
  const ListNode *last(FileSpaceReader *reader) const override;
};

struct FILHeader {
  void init_fil_header(const byte *buf) {
    check_sum_ = check_sum(buf);
    page_number_offset_ = page_number_offset(buf);
    previous_page_ = previous_page(buf);
    next_page_ = next_page(buf);
    last_mod_page_lsn_ = last_mod_page_lsn(buf);
    page_type_ = page_type(buf);
    flush_lsn_ = flush_lsn(buf);
    space_id_ = space_id(buf);
  }
  uint32_t check_sum_;
  uint32_t page_number_offset_;
  uint32_t previous_page_;
  uint32_t next_page_;
  uint64_t last_mod_page_lsn_;
  uint16_t page_type_;
  uint64_t flush_lsn_;
  uint32_t space_id_;
  static constexpr uint8_t FIL_PAGE_SPACE_OR_CHKSUM = 8;
  static constexpr uint8_t FIL_PAGE_OFFSET = 4;
  static constexpr uint8_t FIL_PAGE_PREV = 8;
  static constexpr uint8_t FIL_PAGE_SRV_VERSION = 8;
  static constexpr uint8_t FIL_PAGE_NEXT = 12;
  static constexpr uint8_t FIL_PAGE_SPACE_VERSION = 12;
  static constexpr uint8_t FIL_PAGE_LSN = 16;
  static constexpr uint8_t FIL_PAGE_TYPE = 24;
  static constexpr uint8_t FIL_PAGE_FILE_FLUSH_LSN = 26;
  static constexpr uint8_t FIL_PAGE_SPACE_ID = 34;

  static constexpr uint8_t FIL_PAGE_DATA = 38;
  static constexpr uint8_t FIL_PAGE_DATA_END = 8; // size of the page tailer

  static uint32_t check_sum(const byte *p) {
    return mach_read_from_4(p + FIL_PAGE_SPACE_OR_CHKSUM);
  }
  static uint32_t page_number_offset(const byte *p) {
    return mach_read_from_4(p + FIL_PAGE_OFFSET);
  }
  static uint32_t previous_page(const byte *p) {
    return mach_read_from_4(p + FIL_PAGE_PREV);
  }
  static uint32_t next_page(const byte *p) {
    return mach_read_from_4(p + FIL_PAGE_NEXT);
  }
  static uint64_t last_mod_page_lsn(const byte *p) {
    return mach_read_from_8(p + FIL_PAGE_LSN);
  }
  static uint16_t page_type(const byte *p) {
    return mach_read_from_2(p + FIL_PAGE_TYPE);
  }
  static uint64_t flush_lsn(const byte *p) {
    return mach_read_from_8(p + FIL_PAGE_FILE_FLUSH_LSN);
  }
  static uint32_t space_id(const byte *p) {
    return mach_read_from_4(p + FIL_PAGE_SPACE_ID);
  }
  void dump(std::ostringstream &oss) const;
};

extern const std::unordered_map<uint16_t, std::string> PAGE_TYPE_STR;
std::string get_page_type_str(uint16_t page_type);

struct FSPHeader {
  uint32_t space_id_;
  uint32_t unused_;
  uint32_t fsp_size_;
  uint32_t fsp_free_limit_;
  uint32_t space_flags_;
  uint32_t frag_n_used_;
  XDesEntryList free_list_base_node_;
  XDesEntryList free_frag_list_base_node_;
  XDesEntryList full_frag_list_base_node_;
  uint64_t next_unused_segment_id_;
  INodeEntryList full_inodes_list_base_node_;
  INodeEntryList free_inodes_list_base_node_;

  void init(const byte *buf) {
    space_id_ = mach_read_from_4(buf + FSP_HEADER_OFFSET + FSP_SPACE_ID);
    unused_ = mach_read_from_4(buf + FSP_HEADER_OFFSET + FSP_NOT_USED);
    fsp_size_ = mach_read_from_4(buf + FSP_HEADER_OFFSET + FSP_SIZE);
    fsp_free_limit_ =
        mach_read_from_4(buf + FSP_HEADER_OFFSET + FSP_FREE_LIMIT);
    space_flags_ = mach_read_from_4(buf + FSP_HEADER_OFFSET + FSP_SPACE_FLAGS);
    frag_n_used_ = mach_read_from_4(buf + FSP_HEADER_OFFSET + FSP_FRAG_N_USED);
    free_list_base_node_.init(buf + FSP_HEADER_OFFSET +
                              FSP_FREE_LIST_BASE_NODE);
    free_frag_list_base_node_.init(buf + FSP_HEADER_OFFSET +
                                   FSP_FREE_FRAG_LIST_BASE_NODE);
    full_frag_list_base_node_.init(buf + FSP_HEADER_OFFSET +
                                   FSP_FULL_FRAG_LIST_BASE_NODE);
    next_unused_segment_id_ =
        mach_read_from_8(buf + FSP_HEADER_OFFSET + FSP_NEXT_UNUSED_SEGMENT_ID);
    full_inodes_list_base_node_.init(buf + FSP_HEADER_OFFSET +
                                     FSP_FULL_INODES_LIST_BASE_NODE);
    free_inodes_list_base_node_.init(buf + FSP_HEADER_OFFSET +
                                     FSP_FREE_INODES_LIST_BASE_NODE);
  }

  static constexpr uint8_t FSP_SPACE_ID = 0;
  static constexpr uint8_t FSP_NOT_USED = 4;
  static constexpr uint8_t FSP_SIZE = 8; // cur size of the space in pages
  static constexpr uint8_t FSP_FREE_LIMIT = 12;
  static constexpr uint8_t FSP_SPACE_FLAGS = 16;
  static constexpr uint8_t FSP_FRAG_N_USED = 20;
  static constexpr uint8_t FSP_FREE_LIST_BASE_NODE = 24;
  static constexpr uint8_t FSP_FREE_FRAG_LIST_BASE_NODE = 40;
  static constexpr uint8_t FSP_FULL_FRAG_LIST_BASE_NODE =
      FSP_FREE_FRAG_LIST_BASE_NODE + 16;
  static constexpr uint8_t FSP_NEXT_UNUSED_SEGMENT_ID =
      FSP_FULL_FRAG_LIST_BASE_NODE + 16;
  static constexpr uint8_t FSP_FULL_INODES_LIST_BASE_NODE =
      FSP_NEXT_UNUSED_SEGMENT_ID + 8;
  static constexpr uint8_t FSP_FREE_INODES_LIST_BASE_NODE =
      FSP_FULL_INODES_LIST_BASE_NODE + 16;
  static constexpr uint8_t FSP_HEADER_OFFSET = FILHeader::FIL_PAGE_DATA;
  static constexpr auto FSP_HEADER_SIZE = 32 + 5 * FLST_BASE_NODE_SIZE;

  static uint32_t space_id(const byte *pg) {
    return mach_read_from_4(pg + FSP_HEADER_OFFSET + FSP_SPACE_ID);
  }
  static uint32_t unused;
  static uint32_t fsp_size(const byte *pg) {
    return mach_read_from_4(pg + FSP_HEADER_OFFSET + FSP_SIZE);
  }
  static uint32_t fsp_free_limit(const byte *pg) {
    return mach_read_from_4(pg + FSP_HEADER_OFFSET + FSP_FREE_LIMIT);
  }
  static uint32_t space_flags(const byte *pg) {
    return mach_read_from_4(pg + FSP_HEADER_OFFSET + FSP_SPACE_FLAGS);
  }
  static uint32_t frag_n_used(const byte *pg) {
    return mach_read_from_4(pg + FSP_HEADER_OFFSET + FSP_FRAG_N_USED);
  }
  static const byte *list_base_node_for_free_list(const byte *pg) {
    return pg + FSP_HEADER_OFFSET + FSP_FREE_LIST_BASE_NODE;
  }

  static const byte *list_base_node_for_free_frag_list(const byte *pg) {
    return pg + FSP_HEADER_OFFSET + FSP_FREE_FRAG_LIST_BASE_NODE;
  }

  static const byte *list_base_node_for_full_frag_list(const byte *pg) {
    return pg + FSP_HEADER_OFFSET + FSP_FULL_FRAG_LIST_BASE_NODE;
  }

  static uint64_t next_unused_segment_id(const byte *pg) {
    return mach_read_from_8(pg + FSP_HEADER_OFFSET +
                            FSP_NEXT_UNUSED_SEGMENT_ID);
  }

  static const byte *list_base_node_for_full_inodes_list(const byte *pg) {
    return pg + FSP_HEADER_OFFSET + FSP_FULL_INODES_LIST_BASE_NODE;
  }
  static const byte *list_base_node_for_free_inodes_list(const byte *pg) {
    return pg + FSP_HEADER_OFFSET + FSP_FREE_INODES_LIST_BASE_NODE;
  }

  void dump(std::ostringstream &oss) const;
};

struct XDES_E {
  uint64_t fi_seg_id;
  XDesEntryListNode list_node_for_xdes_e_;
  uint32_t state;
  byte page_state[16];
  static constexpr unsigned char XDES_E_SIZE = 40;
  XDES_E() : fi_seg_id(UINT64_MAX), list_node_for_xdes_e_(), state(0) {}
  bool inited() const { return fi_seg_id != UINT64_MAX; }
  void init(const byte *buf) {
    fi_seg_id = mach_read_from_8(buf + 0);
    list_node_for_xdes_e_.init(buf + 8);
    state = mach_read_from_4(buf + 20);
    memcpy(page_state, (buf + 24), sizeof(page_state));
  }
  void dump_page_state_bitmap(std::ostringstream &oss) const {
    oss << "Page state bitmap: ";
    for (unsigned int i = 0; i < sizeof(page_state); ++i) {
      oss << std::bitset<8>{(uint8_t)page_state[i]} << " ";
    }
    oss << std::dec << std::endl;
  }
  void dump(std::ostringstream &oss, Addr addr) const {
    oss << "XDES_E at ";
    addr.dump(oss);
    oss << "\nfi_seg_id: " << fi_seg_id << ", state: " << state
        << ", list_node: ";
    list_node_for_xdes_e_.dump(oss);
    dump_page_state_bitmap(oss);
  }
  bool is_page_free(uint32_t page_num) const {
    if (page_num >= 64) {
      return false; // Invalid page number
    }
    byte b = page_state[page_num / 4];
    return ((char)b & (1 << (page_num * 2))) == 0;
  }
};

struct INode_E {
  void init(const byte *buf) {
    fseg_id = mach_read_from_8(buf + 0);
    n_of_used_pgs_in_not_full_list = mach_read_from_4(buf + 8);
    free_list_base_node_.init(buf + 12);
    not_full_list_base_node_.init(buf + 28);
    full_list_base_node_.init(buf + 44);
    magic_number_ = mach_read_from_4(buf + MAGIC_NUMBER_OFFSET);
    frag_array_.clear();
    for (size_t i = 0; i < FRAG_ARRAY_SIZE; ++i) {
      frag_array_.push_back(
          mach_read_from_4(buf + MAGIC_NUMBER_OFFSET + 4 + i * 4));
    }
  }
  void dump(std::ostringstream &oss, Addr addr) const {
    oss << "INode_E at ";
    addr.dump(oss);
    if (full_list_base_node_.list_length_ == 0 &&
        n_of_used_pgs_in_not_full_list == 0) {
      oss << " (empty inode entry)";
    } else {
      oss << "\nfseg_id: " << fseg_id << ", n_of_used_pgs_in_not_full_list: "
          << n_of_used_pgs_in_not_full_list
          << ", magic_number: " << magic_number_ << "\n";
      oss << "xdes free_list: ";
      free_list_base_node_.dump(oss);
      oss << "xdes not_full_list: ";
      not_full_list_base_node_.dump(oss);
      oss << "xdes full_list: ";
      full_list_base_node_.dump(oss);
      oss << "frag_array: ";
      for (const auto &frag : frag_array_) {
        oss << frag << " ";
      }
    }
    oss << std::endl;
  }
  bool is_valid_inode_entry() const { return magic_number_ == MAGIC_NUMBER; }
  uint64_t fseg_id;
  uint32_t n_of_used_pgs_in_not_full_list;
  XDesEntryList free_list_base_node_;
  XDesEntryList not_full_list_base_node_;
  XDesEntryList full_list_base_node_;
  uint32_t magic_number_;
  std::vector<int32_t> frag_array_;

  static constexpr uint8_t MAGIC_NUMBER_OFFSET = 60;
  static constexpr uint8_t INODE_ENTRY_SIZE = 192;
  static constexpr uint32_t MAGIC_NUMBER = 97937874;
  static constexpr uint32_t FRAG_ARRAY_SIZE = 32;
};

struct IndexHeader {
  uint8_t page_n_dir_slots_;
  uint16_t heap_top_pos_;
  uint16_t n_of_heap_recs_or_ft_fg_;
  uint16_t first_garbage_rec_offset_;
  uint16_t pg_direction_;
  uint16_t n_of_inserts_in_pg_direction_;
  uint16_t n_of_recs_;
  uint64_t max_trx_id_;
  uint16_t page_level_;
  uint64_t index_id_;
  void init(const byte *buf) {
    page_n_dir_slots_ = mach_read_from_1(buf + PAGE_HEADER + PAGE_N_DIR_SLOTS);
    heap_top_pos_ = mach_read_from_2(buf + PAGE_HEADER + PAGE_HEAP_TOP);
    n_of_heap_recs_or_ft_fg_ =
        mach_read_from_2(buf + PAGE_HEADER + PAGE_N_HEAP);
    first_garbage_rec_offset_ = mach_read_from_2(buf + PAGE_HEADER + PAGE_FREE);
    pg_direction_ = mach_read_from_2(buf + PAGE_HEADER + PAGE_DIRECTION);
    n_of_inserts_in_pg_direction_ =
        mach_read_from_2(buf + PAGE_HEADER + PAGE_N_DIRECTION);
    n_of_recs_ = mach_read_from_2(buf + PAGE_HEADER + PAGE_N_RECS);
    max_trx_id_ = mach_read_from_8(buf + PAGE_HEADER + PAGE_MAX_TRX_ID);
    page_level_ = mach_read_from_2(buf + PAGE_HEADER + PAGE_LEVEL);
    index_id_ = mach_read_from_8(buf + PAGE_HEADER + PAGE_INDEX_ID);
  }
  static constexpr uint8_t PAGE_N_DIR_SLOTS = 0;
  static constexpr uint8_t PAGE_HEAP_TOP = 2;
  static constexpr uint8_t PAGE_N_HEAP = 4;
  static constexpr uint8_t PAGE_FREE = 6;
  static constexpr uint8_t PAGE_GARBAGE = 8;
  static constexpr uint8_t PAGE_DIRECTION = 12;
  static constexpr uint8_t PAGE_N_DIRECTION = 14;
  static constexpr uint8_t PAGE_N_RECS = 16;
  static constexpr uint8_t PAGE_MAX_TRX_ID = 18;
  static constexpr uint8_t PAGE_HEADER_PRIV_END = 26;
  static constexpr uint8_t PAGE_LEVEL = 26;
  static constexpr uint8_t PAGE_INDEX_ID = 28;
  static constexpr uint8_t PAGE_HEADER = FILHeader::FIL_PAGE_DATA;

  static uint16_t n_of_dir_slots(const byte *pg) {
    return mach_read_from_2(pg + PAGE_HEADER + PAGE_N_DIR_SLOTS);
  }
  static uint16_t heap_top_pos(const byte *pg) {
    return mach_read_from_2(pg + PAGE_HEADER + PAGE_HEAP_TOP);
  }
  static uint16_t n_of_heap_recs_or_ft_fg(const byte *b) {
    return mach_read_from_2(b + PAGE_HEADER + PAGE_N_HEAP);
  }
  static uint16_t first_garbage_rec_offset(const byte *b) {
    return mach_read_from_2(b + PAGE_HEADER + PAGE_FREE);
  }
  static uint16_t garbage_space;
  static uint16_t last_insert_pos;
  static uint16_t pg_direction(const byte *b) {
    return mach_read_from_2(b + PAGE_HEADER + PAGE_DIRECTION);
  }
  static uint16_t n_of_inserts_in_pg_direction;
  static uint16_t n_of_recs(const byte *b) {
    return mach_read_from_2(b + PAGE_HEADER + PAGE_N_RECS);
  }
  static uint64_t max_trx_id(const byte *b) {
    return mach_read_from_8(b + PAGE_HEADER + PAGE_MAX_TRX_ID);
  }
  static uint16_t page_level(const byte *b) {
    return mach_read_from_2(b + PAGE_HEADER + PAGE_LEVEL);
  }
  static uint64_t index_id(const byte *b) {
    return mach_read_from_8(b + PAGE_HEADER + PAGE_INDEX_ID);
  }
  static constexpr unsigned int INDEX_HEADER_SIZE = 74 - 38;
  void dump(std::ostringstream &oss) const;
};

struct FSEG_HEADER {
  uint32_t leaf_page_inode_space_id_;
  Addr leaf_page_inode_addr_;
  uint32_t internal_page_inode_space_id_;
  Addr internal_page_inode_addr_;

  void init(const byte *pg) {
    leaf_page_inode_space_id_ =
        mach_read_from_4(fseg_header(pg) + FSEG_HDR_LEAF_SPACE);
    leaf_page_inode_addr_.page_number_ =
        mach_read_from_4(fseg_header(pg) + FSEG_HDR_LEAF_PAGE_NO);
    leaf_page_inode_addr_.offset_ =
        mach_read_from_2(FSEG_HDR_LEAF_OFFSET + fseg_header(pg));

    internal_page_inode_space_id_ =
        mach_read_from_4(fseg_header(pg) + FSEG_HDR_INTERNAL_PAGE_NO);
    internal_page_inode_addr_.page_number_ =
        mach_read_from_4(fseg_header(pg) + FSEG_HDR_INTERNAL_PAGE_NO);
    internal_page_inode_addr_.offset_ =
        mach_read_from_2(fseg_header(pg) + FSEG_HDR_INTERNAL_OFFSET);
  }

  static constexpr uint8_t FSEG_HDR_LEAF_SPACE = 0;
  static constexpr uint8_t FSEG_HDR_LEAF_PAGE_NO = 4;
  static constexpr uint8_t FSEG_HDR_LEAF_OFFSET = 8;
  static constexpr uint8_t FSEG_HDR_INTERNAL_SPACE = 12;
  static constexpr uint8_t FSEG_HDR_INTERNAL_PAGE_NO = 16;
  static constexpr uint8_t FSEG_HDR_INTERNAL_OFFSET = 20;
  static constexpr uint8_t FSEG_HEADER_SIZE = 10;
  static constexpr uint8_t FSEG_PAGE_DATA = FILHeader::FIL_PAGE_DATA;
  static const byte *fseg_header(const byte *pg) {
    return pg + FSPHeader::FSP_HEADER_OFFSET + PAGE_BTR_SEG_LEAF;
  }
  static uint32_t fseg_space(const byte *pg) {
    return mach_read_from_4(fseg_header(pg) + FSEG_HDR_LEAF_SPACE);
  }
  static uint32_t fseg_hdr_page_no(const byte *pg) {
    return mach_read_from_4(fseg_header(pg) + FSEG_HDR_LEAF_PAGE_NO);
  }
  static uint16_t fseg_hdr_offset(const byte *pg) {
    return mach_read_from_2(fseg_header(pg) + FSEG_HDR_LEAF_OFFSET);
  }
  void dump(std::ostringstream &oss) const {
    oss << "FSEG_HEADER: leaf_page_inode_space_id: "
        << leaf_page_inode_space_id_ << ", leaf_page_inode_addr: ";
    leaf_page_inode_addr_.dump(oss);
    oss << ", internal_page_inode_space_id: " << internal_page_inode_space_id_
        << ", internal_page_inode_addr: ";
    internal_page_inode_addr_.dump(oss);
  }
  const std::vector<INode_E> *leaf_inode(FileSpaceReader *reader) const;
  const std::vector<INode_E> *external_inode(FileSpaceReader *reader) const;
};

const char *get_rec_type(uint8_t rec_t);

enum rec_type {
  REC_STATUS_ORDINARY = 0,
  REC_STATUS_NODE_PTR = 1,
  REC_STATUS_INFIMUM = 2,
  REC_STATUS_SUPREMUM = 3
};

constexpr uint8_t PAGE_HEADER = FSEG_HEADER::FSEG_PAGE_DATA;
constexpr uint8_t PAGE_DATA =
    PAGE_HEADER + 36 + 2 * FSEG_HEADER::FSEG_HEADER_SIZE;
constexpr uint8_t REC_N_EXTRA_BYTES = 5;
constexpr auto PAGE_NEW_INFIMUM = PAGE_DATA + REC_N_EXTRA_BYTES;
constexpr auto PAGE_NEW_SUPREMUM = PAGE_DATA + 2 * REC_N_EXTRA_BYTES + 8;
constexpr auto PAGE_NEW_SUPREMUM_END = PAGE_NEW_SUPREMUM + 8;

struct Records {
  static void dump(const byte *pg, std::ostringstream &oss);
};

struct RecordHeader {
  static constexpr uint8_t REC_NEW_INFO_BITS = 5;
  static constexpr auto REC_INFO_BITS_MASK = 0xF0UL;
  static constexpr uint8_t REC_INFO_BITS_SHIFT = 0;

  static constexpr uint8_t REC_NEW_N_OWNED = 5;
  static constexpr auto REC_N_OWNED_MASK = 0xFUL;
  static constexpr uint8_t REC_N_OWNED_SHIFT = 0;

  static constexpr uint8_t REC_NEW_HEAP_NO = 4;
  static constexpr auto REC_HEAP_NO_MASK = 0xFFF8UL;
  static constexpr uint8_t REC_HEAP_NO_SHIFT = 3;

  static constexpr uint8_t REC_NEW_STATUS = 3;
  static constexpr auto REC_NEW_STAUTS_MASK = 0x7UL;
  static constexpr uint8_t REC_NEW_STATUS_SHIFT = 0;

  static constexpr uint8_t REC_NEXT = 2;

  static inline uint8_t info_bits(const byte *rec) {
    return rec_get_bit_field_1(rec, REC_NEW_INFO_BITS, REC_INFO_BITS_MASK,
                               REC_INFO_BITS_SHIFT);
  }
  static inline uint8_t num_of_recs_owned(const byte *rec) {
    return rec_get_bit_field_1(rec, REC_NEW_N_OWNED, REC_N_OWNED_MASK,
                               REC_N_OWNED_SHIFT);
  }

  static inline uint16_t heap_no_new(const byte *rec) {
    return rec_get_bit_field_2(rec, REC_NEW_HEAP_NO, REC_HEAP_NO_MASK,
                               REC_HEAP_NO_SHIFT);
  }
  static inline uint8_t rec_status(const byte *rec) {
    return rec_get_bit_field_1(rec, REC_NEW_STATUS, REC_NEW_STAUTS_MASK,
                               REC_NEW_STATUS_SHIFT);
  }
  static inline uint16_t next_offs(const byte *rec) {
    ulint field_value = mach_read_from_2(rec - REC_NEXT);

    if (field_value == 0)
      return 0;
    return align_offset(rec + field_value, PAGE_SIZE);
  }

  static inline byte *next_ptr(const byte *rec) {
    ulint field_value = mach_read_from_2(rec - REC_NEXT);
    if (field_value == 0)
      return nullptr;

    return ((byte *)align_down(rec, PAGE_SIZE) +
            align_offset(rec + field_value, PAGE_SIZE));
  }

  static void dump(const byte *rec, std::ostringstream &oss);
};

struct IndexPageDirectory {
  static constexpr uint8_t PAGE_DIR_SLOT_SIZE = 2;
  static constexpr uint8_t PAGE_DIR = FILHeader::FIL_PAGE_DATA_END;
  static inline const byte *get_nth_slot(const byte *pg, ulint n) {
    return pg + PAGE_SIZE - PAGE_DIR - (n + 1) * PAGE_DIR_SLOT_SIZE;
  }
  static inline const byte *get_slot_rec(const byte *slot) {
    return page_align(slot) + mach_read_from_2(slot);
  }
  static inline ulint slot_get_n_owned(const byte *slot) {
    return RecordHeader::num_of_recs_owned(get_slot_rec(slot));
  }
  static void dump(const byte *pg, std::ostringstream &oss);
};

struct DictHeader {
  uint64_t row_id_;
  uint64_t table_id_;
  uint64_t index_id_;
  uint32_t max_space_id_;
  uint32_t min_id_low_;
  uint32_t tables_;
  uint32_t table_ids_;
  uint32_t columns_;
  uint32_t indexes_;
  uint32_t fields_;
  FSEG_HEADER fseg_header_;
  void init(const byte *pg) {
    row_id_ = mach_read_from_8(pg + DICT_HDR_ROW_ID);
    table_id_ = mach_read_from_8(pg + DICT_HDR_TABLE_ID);
    index_id_ = mach_read_from_8(pg + DICT_HDR_INDEX_ID);
    max_space_id_ = mach_read_from_4(pg + DICT_HDR_MAX_SPACE_ID);
    min_id_low_ = mach_read_from_4(pg + DICT_HDR_MIN_ID_LOW);
    tables_ = mach_read_from_4(pg + DICT_HDR_TABLES);
    table_ids_ = mach_read_from_4(pg + DICT_HDR_TABLE_IDS);
    columns_ = mach_read_from_4(pg + DICT_HDR_COLUMNS);
    indexes_ = mach_read_from_4(pg + DICT_HDR_INDEXES);
    fields_ = mach_read_from_4(pg + DICT_HDR_FIELDS);
    // fseg_header_.init(pg);
  }

  static constexpr uint8_t DICT_HDR_ROW_ID = 0;
  static constexpr uint8_t DICT_HDR_TABLE_ID = 8;
  static constexpr uint8_t DICT_HDR_INDEX_ID = 16;
  static constexpr uint8_t DICT_HDR_MAX_SPACE_ID = 24;
  static constexpr uint8_t DICT_HDR_MIN_ID_LOW = 28;
  static constexpr uint8_t DICT_HDR_TABLES = 32;
  static constexpr uint8_t DICT_HDR_TABLE_IDS = 36;
  static constexpr uint8_t DICT_HDR_COLUMNS = 40;
  static constexpr uint8_t DICT_HDR_INDEXES = 44;
  static constexpr uint8_t DICT_HDR_FIELDS = 48;
  static constexpr uint8_t DICT_HDR_FSEG_HEADER = 56;

  static inline uint64_t row_id(const byte *pg) {
    return mach_read_from_8(pg + DICT_HDR_ROW_ID);
  }
  static inline uint64_t table_id(const byte *pg) {
    return mach_read_from_8(pg + DICT_HDR_TABLE_ID);
  }
  static inline uint64_t index_id(const byte *pg) {
    return mach_read_from_8(pg + DICT_HDR_INDEX_ID);
  }
  static inline uint32_t max_space_id(const byte *pg) {
    return mach_read_from_4(pg + DICT_HDR_MAX_SPACE_ID);
  }
  static inline uint32_t min_id_low(const byte *pg) {
    return mach_read_from_4(pg + DICT_HDR_MIN_ID_LOW);
  }
  static inline uint32_t tables(const byte *pg) {
    return mach_read_from_4(pg + DICT_HDR_TABLES);
  }
  static inline uint32_t table_ids(const byte *pg) {
    return mach_read_from_4(pg + DICT_HDR_TABLE_IDS);
  }
  static inline uint32_t columns(const byte *pg) {
    return mach_read_from_4(pg + DICT_HDR_COLUMNS);
  }
  static inline uint32_t indexes(const byte *pg) {
    return mach_read_from_4(pg + DICT_HDR_INDEXES);
  }
  static inline uint32_t fields(const byte *pg) {
    return mach_read_from_4(pg + DICT_HDR_FIELDS);
  }
};
} // namespace innodb
