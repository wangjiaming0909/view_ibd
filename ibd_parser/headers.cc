#include "headers.h"
#include <arpa/inet.h>
#include <cassert>
#include <limits>

using namespace innodb;

const std::unordered_map<uint16_t, std::string> innodb::PAGE_TYPE_STR = {
    {FIL_PAGE_TYPE_UNUSED, "FIL_PAGE_TYPE_UNUSED"},
    {FIL_PAGE_UNDO_LOG, "FIL_PAGE_UNDO_LOG"},
    {FIL_PAGE_INODE, "FIL_PAGE_INODE"},
    {FIL_PAGE_IBUF_FREE_LIST, "FIL_PAGE_IBUF_FREE_LIST"},
    {FIL_PAGE_TYPE_ALOCATED, "FIL_PAGE_TYPE_ALLOCATED"},
    {FIL_PAGE_IBUF_BITMAP, "FIL_PAGE_IBUF_BITMAP"},
    {FIL_PAGE_TYPE_SYS, "FIL_PAGE_TYPE_SYS"},
    {FIL_PAGE_TYPE_TRX_SYS, "FIL_PAGE_TYPE_TRX_SYS"},
    {FIL_PAGE_TYPE_FSP_HDR, "FIL_PAGE_TYPE_FSP_HDR"},
    {FIL_PAGE_TYPE_XDES, "FIL_PAGE_TYPE_XDES"},
    {FIL_PAGE_TYPE_UNKNOWN, "FIL_PAGE_TYPE_UNKNOWN"},
    {FIL_PAGE_SDI, "FIL_PAGE_SDI"},
    {FIL_PAGE_RTREE, "FIL_PAGE_RTREE"},
    {FIL_PAGE_INDEX, "FIL_PAGE_INDEX"}};

void FILHeader::dump(const byte *b, std::ostringstream &oss) {
  using namespace std;
  oss << "FilHeader:: check sum: " << check_sum(b) << "\t"
      << "page number offset: " << page_number_offset(b) << "\t"
      << "previous page: " << previous_page(b) << "\t"
      << "next page: " << next_page(b) << "\t"
      << "page type: " << get_page_type_str(page_type(b)) << "\t\t"
      << "flush lsn: " << flush_lsn(b) << "\t"
      << "space id: " << space_id(b) << endl;
}

std::string innodb::get_page_type_str(uint16_t page_type) {
  auto it = PAGE_TYPE_STR.find(page_type);
  if (it == PAGE_TYPE_STR.end())
    return std::string("not found type: ") + std::to_string(page_type);
  return it->second;
}

void innodb::FSPHeader::dump(const byte *pg, std::ostringstream &oss) {
  using namespace std;
  oss << "FSPHeader: space_id: " << space_id(pg) << "\t"
      << "fsp_size in page: " << fsp_size(pg) << "\t"
      << "fsp_free_limit: " << fsp_free_limit(pg) << "\t"
      << "flags: " << space_flags(pg) << "\t"
      << "frag_n_used: " << frag_n_used(pg) << endl;
}

void IndexHeader::dump(const byte *b, std::ostringstream &oss) {
  using namespace std;
  oss << "IndexHeader: dir_slots: " << n_of_dir_slots(b) << "\t"
      << "heap_top_pos: " << heap_top_pos(b) << "\t"
      << "n_of_heap_recs_or_ft_fg: " << n_of_heap_recs_or_ft_fg(b) << "\t"
      << "pg direction: " << pg_direction(b) << "\t"
      << "n_of_records: " << n_of_recs(b) << "\t"
      << "max_trx_id: " << max_trx_id(b) << "\t"
      << "page_level: " << page_level(b) << "\t"
      << "index id: " << index_id(b) << endl;
}

void FSEG_HEADER::dump(const byte *pg, std::ostringstream &oss) {
  using namespace std;
  oss << "FSEG_HEADER: fseg_space: " << fseg_space(pg) << "\t"
      << "fseg_hdr_page_no: " << fseg_hdr_page_no(pg) << "\t"
      << "fseg_hdr_offset: " << fseg_hdr_offset(pg) << endl;
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

void RecordHeader::dump(const byte *rec, std::ostringstream &oss)
{
  using namespace std;
  oss << "rec: off: " << (ulint)(rec - (const byte*)align_down(rec, PAGE_SIZE))
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
      oss << "next_ptr: " << next_ptr(rec) - (const byte *)align_down(rec, PAGE_SIZE) << "\t";
      break;
  default:
      break;
  }
  oss << endl;
}

void Records::dump(const byte *pg, std::ostringstream &oss)
{
  auto * const infimum = pg + PAGE_NEW_INFIMUM;
  auto * const supremum = pg + PAGE_NEW_SUPREMUM;

  auto *cur = infimum;
  while (cur != supremum) {
    if (RecordHeader::rec_status(cur) == REC_STATUS_ORDINARY || RecordHeader::rec_status(cur) == REC_STATUS_INFIMUM) {
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

void IndexPageDirectory::dump(const byte *pg, std::ostringstream &oss)
{
    ulint n_slot = IndexHeader::n_of_dir_slots(pg);
    oss << "page directory: n of slots: " << n_slot << "\t";
    for (ulint i = 0; i < n_slot; ++i) {
        oss << get_slot_rec(get_nth_slot(pg, i)) - pg << "\t";
    }
    oss << std::endl;
}
