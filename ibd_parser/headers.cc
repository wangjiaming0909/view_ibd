#include "headers.h"
#include "file_space_reader.h"
#include <arpa/inet.h>
#include <cassert>

using namespace innodb;

const std::unordered_map<uint16_t, std::string> innodb::PAGE_TYPE_STR = {
    {FIL_PAGE_TYPE_UNUSED, "FIL_PAGE_TYPE_UNUSED"},
    {FIL_PAGE_UNDO_LOG, "FIL_PAGE_UNDO_LOG"},
    {FIL_PAGE_TYPE_INODE, "FIL_PAGE_INODE"},
    {FIL_PAGE_IBUF_FREE_LIST, "FIL_PAGE_IBUF_FREE_LIST"},
    {FIL_PAGE_TYPE_ALOCATED, "FIL_PAGE_TYPE_ALLOCATED"},
    {FIL_PAGE_IBUF_BITMAP, "FIL_PAGE_IBUF_BITMAP"},
    {FIL_PAGE_TYPE_SYS, "FIL_PAGE_TYPE_SYS"},
    {FIL_PAGE_TYPE_TRX_SYS, "FIL_PAGE_TYPE_TRX_SYS"},
    {FIL_PAGE_TYPE_FSP_HDR, "FIL_PAGE_TYPE_FSP_HDR"},
    {FIL_PAGE_TYPE_XDES, "FIL_PAGE_TYPE_XDES"},
    {FIL_PAGE_TYPE_UNKNOWN, "FIL_PAGE_TYPE_UNKNOWN"},
    {FIL_PAGE_TYPE_SDI, "FIL_PAGE_SDI"},
    {FIL_PAGE_RTREE, "FIL_PAGE_RTREE"},
    {FIL_PAGE_INDEX, "FIL_PAGE_INDEX"}};

void FILHeader::dump(std::ostringstream &oss) const {
  using namespace std;
  oss << "FilHeader:: check sum: " << check_sum_ << "\t"
      << "page number offset: " << page_number_offset_ << "\t"
      << "previous page: " << previous_page_ << "\t"
      << "next page: " << next_page_ << "\t"
      << "page type: " << get_page_type_str(page_type_) << "\t\t"
      << "flush lsn: " << flush_lsn_ << "\t"
      << "space id: " << space_id_ << endl;
}

std::string innodb::get_page_type_str(uint16_t page_type) {
  auto it = PAGE_TYPE_STR.find(page_type);
  if (it == PAGE_TYPE_STR.end())
    return std::string("not found type: ") + std::to_string(page_type);
  return it->second;
}

void innodb::FSPHeader::dump(std::ostringstream &oss) const {
  using namespace std;
  oss << "FSPHeader: space_id: " << space_id_ << "\t"
      << "fsp_size in page: " << fsp_size_ << "\t"
      << "fsp_free_limit: " << fsp_free_limit_ << "\t"
      << "flags: " << space_flags_ << "\t"
      << "frag_n_used: " << frag_n_used_ << endl;
  oss << "free list: ";
  free_list_base_node_.dump(oss);
  oss << endl << "free frag list: ";
  free_frag_list_base_node_.dump(oss);
  oss << endl << "full frag list: ";
  full_frag_list_base_node_.dump(oss);
  oss << endl << "next unused segment id: " << next_unused_segment_id_;
  oss << endl << "free inode list: ";
  free_inodes_list_base_node_.dump(oss);
  oss << endl << "full inode list: ";
  full_inodes_list_base_node_.dump(oss);
  oss << endl;
}

void IndexHeader::dump(std::ostringstream &oss) const {
  using namespace std;
  oss << "IndexHeader: dir_slots: " << page_n_dir_slots_ << "\t"
      << "heap_top_pos: " << heap_top_pos_ << "\t"
      << "n_of_heap_recs_or_ft_fg: " << n_of_heap_recs_or_ft_fg_ << "\t"
      << "pg direction: " << pg_direction_ << "\t"
      << "n_of_records: " << n_of_recs_ << "\t"
      << "max_trx_id: " << max_trx_id_ << "\t"
      << "page_level: " << page_level_ << "\t"
      << "index id: " << index_id_ << endl;
}

const char *innodb::get_rec_type(uint8_t rec_t) {
  const char *p;
  switch (rec_t) {
  case REC_STATUS_ORDINARY:
    p = "REC_STATUS_ORDINARY";
    break;
  case REC_STATUS_NODE_PTR:
    p = "REC_STATUS_NODE_PTR";
    break;
  case REC_STATUS_INFIMUM:
    p = "REC_STATUS_INFIMUM";
    break;
  case REC_STATUS_SUPREMUM:
    p = "REC_STATUS_SUPREMUM";
    break;
  default:
    p = "unknown rec type";
  }
  return p;
}

void RecordHeader::dump(const byte *rec, std::ostringstream &oss) {
  using namespace std;
  oss << "rec: off: " << (ulint)(rec - (const byte *)align_down(rec, PAGE_SIZE))
      << "\tinfo_bits: " << std::to_string(info_bits(rec)) << "\t"
      << "num_of_recs_owned: " << std::to_string(num_of_recs_owned(rec)) << "\t"
      << "heap_no_new: " << heap_no_new(rec) << "\t"
      << "rec_status: " << get_rec_type(rec_status(rec)) << "\t";

  switch (rec_status(rec)) {
  case REC_STATUS_INFIMUM:
  case REC_STATUS_SUPREMUM:
  case REC_STATUS_ORDINARY:
    oss << "next_offs: " << next_offs(rec) << "\t";
    break;
  case REC_STATUS_NODE_PTR:
    oss << "next_ptr: "
        << next_ptr(rec) - (const byte *)align_down(rec, PAGE_SIZE) << "\t";
    break;
  default:
    break;
  }
  oss << endl;
}

void Records::dump(const byte *pg, std::ostringstream &oss) {
  auto *const infimum = pg + PAGE_NEW_INFIMUM;
  auto *const supremum = pg + PAGE_NEW_SUPREMUM;

  auto *cur = infimum;
  while (cur != supremum) {
    if (RecordHeader::rec_status(cur) == REC_STATUS_ORDINARY ||
        RecordHeader::rec_status(cur) == REC_STATUS_INFIMUM) {
      RecordHeader::dump(cur, oss);
      cur = pg + RecordHeader::next_offs(cur);
    } else {
      RecordHeader::dump(cur, oss);
      assert(RecordHeader::rec_status(cur) == REC_STATUS_NODE_PTR);
      cur = RecordHeader::next_ptr(cur);
    }
  }
  RecordHeader::dump(supremum, oss);
}

void IndexPageDirectory::dump(const byte *pg, std::ostringstream &oss) {
  ulint n_slot = IndexHeader::n_of_dir_slots(pg);
  oss << "page directory: n of slots: " << n_slot << "\t";
  for (ulint i = 0; i < n_slot; ++i) {
    oss << get_slot_rec(get_nth_slot(pg, i)) - pg << "\t";
  }
  oss << std::endl;
}

const ListNode *XDesEntryList::get_xdes_entry_list_node(FileSpaceReader *reader,
                                                        Addr &addr) const {
  auto *page = reader->get_page(addr.page_number_);
  if (!page) {
    return nullptr; // Error handling: page not found
  }
  uint32_t entry_num =
      (addr.offset_ - FILHeader::FIL_PAGE_DATA - FSPHeader::FSP_HEADER_SIZE) /
      XDES_E::XDES_E_SIZE;
  auto *xdes_entry = page->get_xdes_entry(entry_num);
  if (!xdes_entry) {
    return nullptr; // Error handling: xdes entry not found
  }
  const_cast<XDES_E *>(xdes_entry)->list_node_for_xdes_e_.addr.page_number_ =
      addr.page_number_;
  const_cast<XDES_E *>(xdes_entry)->list_node_for_xdes_e_.addr.offset_ =
      addr.offset_;
  return &xdes_entry->list_node_for_xdes_e_;
}
const ListNode *XDesEntryList::first(FileSpaceReader *reader) const {
  if (list_length_ == 0) {
    return nullptr;
  }
  Addr addr(first_page_number_, first_offset_);
  if (!addr.valid()) {
    return nullptr; // Error handling: invalid address
  }
  return get_xdes_entry_list_node(reader, addr);
}
const ListNode *XDesEntryList::last(FileSpaceReader *reader) const {
  if (list_length_ == 0) {
    return nullptr;
  }
  Addr addr(last_page_number_, last_offset_);
  if (!addr.valid()) {
    return nullptr; // Error handling: invalid address
  }
  return get_xdes_entry_list_node(reader, addr);
}

const ListNode *XDesEntryListNode::next(FileSpaceReader *reader) const {
  Addr addr(next_page_number_, next_offset_);
  if (!addr.valid()) {
    return nullptr; // Error handling: invalid address
  }
  auto *page = reader->get_page(addr.page_number_);
  if (!page) {
    return nullptr; // Error handling: page not found
  }
  int32_t entry_num =
      (addr.offset_ - FILHeader::FIL_PAGE_DATA - FSPHeader::FSP_HEADER_SIZE) /
      XDES_E::XDES_E_SIZE;
  auto *xdes_entry = page->get_xdes_entry(entry_num);
  if (!xdes_entry) {
    return nullptr; // Error handling: xdes entry not found
  }
  const_cast<XDES_E *>(xdes_entry)->list_node_for_xdes_e_.addr.page_number_ =
      addr.page_number_;
  const_cast<XDES_E *>(xdes_entry)->list_node_for_xdes_e_.addr.offset_ =
      addr.offset_;
  return &xdes_entry->list_node_for_xdes_e_;
}
const XDES_E *XDesEntryListNode::xdes(FileSpaceReader *reader) const {
  if (!addr.valid()) {
    return nullptr; // Error handling: invalid address
  }
  auto *page = reader->get_page(addr.page_number_);
  if (!page) {
    return nullptr; // Error handling: page not found
  }
  int32_t entry_num =
      (addr.offset_ - FILHeader::FIL_PAGE_DATA - FSPHeader::FSP_HEADER_SIZE) /
      XDES_E::XDES_E_SIZE;
  auto *xdes_entry = page->get_xdes_entry(entry_num);
  if (!xdes_entry) {
    return nullptr; // Error handling: xdes entry not found
  }
  return xdes_entry;
}

const ListNode *INodeEntryList::first(FileSpaceReader *reader) const {
  if (list_length_ == 0) {
    return nullptr;
  }
  Addr addr(first_page_number_, first_offset_);
  if (!addr.valid()) {
    return nullptr; // Error handling: invalid address
  }
  auto *page = reader->get_page(addr.page_number_);
  if (!page) {
    return nullptr; // Error handling: page not found
  }
  auto *inode_page = static_cast<const INodePage *>(page);
  const_cast<INodePage *>(inode_page)
      ->list_node_for_INODE_page_list_.addr.page_number_ = addr.page_number_;
  const_cast<INodePage *>(inode_page)
      ->list_node_for_INODE_page_list_.addr.offset_ = addr.offset_;
  return &inode_page->list_node_for_INODE_page_list_;
}
const ListNode *INodeEntryList::last(FileSpaceReader *reader) const {
  if (list_length_ == 0) {
    return nullptr;
  }
  Addr addr(last_page_number_, last_offset_);
  if (!addr.valid()) {
    return nullptr; // Error handling: invalid address
  }
  auto *page = reader->get_page(addr.page_number_);
  if (!page) {
    return nullptr; // Error handling: page not found
  }
  auto *inode_page = static_cast<const INodePage *>(page);
  return &inode_page->list_node_for_INODE_page_list_;
}

const ListNode *INodeEntryListNode::next(FileSpaceReader *reader) const {
  Addr addr(next_page_number_, next_offset_);
  if (!addr.valid()) {
    return nullptr; // Error handling: invalid address
  }
  auto *page = reader->get_page(addr.page_number_);
  if (!page) {
    return nullptr; // Error handling: page not found
  }
  auto *inode_page = static_cast<const INodePage *>(page);
  const_cast<INodePage *>(inode_page)
      ->list_node_for_INODE_page_list_.addr.page_number_ = addr.page_number_;
  const_cast<INodePage *>(inode_page)
      ->list_node_for_INODE_page_list_.addr.offset_ = addr.offset_;
  return &inode_page->list_node_for_INODE_page_list_;
}
const std::vector<INode_E> *
INodeEntryListNode::inode_arr(FileSpaceReader *reader) const {
  if (!addr.valid()) {
    return nullptr; // Error handling: invalid address
  }
  auto *page = reader->get_page(addr.page_number_);
  if (!page) {
    return nullptr; // Error handling: page not found
  }
  auto *inode_page = static_cast<const INodePage *>(page);
  return &inode_page->inode_arr_;
}

const std::vector<INode_E> *
FSEG_HEADER::leaf_inode(FileSpaceReader *reader) const {
  Addr addr(leaf_page_inode_addr_.page_number_, leaf_page_inode_addr_.offset_);
  if (!addr.valid()) {
    return nullptr;
  }
  auto *pg = reader->get_page(addr.page_number_);
  if (!pg) {
    return nullptr; // Error handling: page not found
  }
  auto *inode_page = static_cast<INodePage *>(pg);
  return &inode_page->inode_arr_;
}
const std::vector<INode_E> *
FSEG_HEADER::external_inode(FileSpaceReader *reader) const {
  Addr addr(internal_page_inode_addr_.page_number_,
            internal_page_inode_addr_.offset_);
  if (!addr.valid()) {
    return nullptr;
  }
  auto *pg = reader->get_page(addr.page_number_);
  if (!pg) {
    return nullptr; // Error handling: page not found
  }
  auto inode_page = static_cast<INodePage *>(pg);
  return &inode_page->inode_arr_;
}