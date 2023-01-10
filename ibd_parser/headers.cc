#include "headers.h"
#include <arpa/inet.h>
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

void FILHeader::dump(const std::byte*b, std::ostringstream &oss) {
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

void innodb::FSPHeader::dump(const std::byte*pg, std::ostringstream &oss) {
  using namespace std;
  oss << "FSPHeader: space_id: " << space_id(pg) << "\t"
      << "fsp_size in page: " << fsp_size(pg) << "\t"
      << "fsp_free_limit: " << fsp_free_limit(pg) << "\t"
      << "flags: " << space_flags(pg) << "\t"
      << "frag_n_used: " << frag_n_used(pg) << endl;
}

void IndexHeader::dump(const std::byte*b, std::ostringstream &oss) {
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

void FSEG_HEADER::dump(std::ostringstream &oss) const {
  using namespace std;
  oss << "FSEG_HEADER: leaf page inode spid: " << e(leaf_pg_inode_space_id)
      << "\t"
      << "leaf_pg_inode_pg_num: " << e(leaf_pg_inode_pg_num) << "\t"
      << "leaf_pg_inode_offset: " << e(leaf_pg_inode_offset) << "\t"
      << "internal_inode_space_id: " << e(internal_inode_space_id) << "\t"
      << "internal inode pg num: " << e(unternal_inode_pg_num) << "\t"
      << "internal inode offset: " << e(internal_inode_offset) << endl;
}

const char *innodb::get_rec_type(uint8_t rec_t)
{
    const char* p;
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
