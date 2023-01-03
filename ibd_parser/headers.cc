#include "headers.h"
#include "defines.h"
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

void FILHeader::dump(std::ostringstream &oss) const {
  using namespace std;
  oss << "FilHeader:: check sum: " << e(check_sum) << "\t"
      << "page number offset: " << e(page_number_offset) << "\t"
      << "previous page: " << e(previous_page) << "\t"
      << "next page: " << e(next_page) << "\t"
      << "page type: " << get_page_type_str(e(page_type)) << "\t\t"
      << "flush lsn: " << e(flush_lsn) << "\t"
      << "space id: " << e(space_id) << endl;
}

std::string innodb::get_page_type_str(uint16_t page_type) {
  auto it = PAGE_TYPE_STR.find(page_type);
  if (it == PAGE_TYPE_STR.end())
    return std::string("not found type: ") + std::to_string(page_type);
  return it->second;
}

void innodb::FSPHeader::dump(std::ostringstream &oss) const {
  using namespace std;
  oss << "FSPHeader: space_id" << e(space_id) << "\t"
      << "highest_pg_n_in_f: " << e(highest_page_number_in_file) << "\t"
      << "higest_pg_n_inited: " << highest_page_number_initialized << "\t"
      << "flags: " << e(flags) << "\t"
      << "no_pg_in_free_frag: " << e(no_pages_in_free_frag) << "\t"
      << "next_unnsed_seg_id: " << e(next_unused_seg_id) << endl;
}

void IndexHeader::dump(std::ostringstream &oss) const {
  using namespace std;
  oss << "IndexHeader: dir_slots: " << e(n_of_dir_slots) << "\t"
      << "heap_top_pos: " << e(heap_top_pos) << "\t"
      << "n_of_heap_recs_or_ft_fg: " << e(n_of_heap_recs_or_ft_fg) << "\t"
      << "pg direction: " << e(pg_direction) << "\t"
      << "n_of_records: " << e(n_of_recs) << "\t"
      << "max_trx_id: " << e(max_trx_id) << "\t"
      << "page_level: " << e(page_level) << "\t"
      << "index id: " << e(index_id) << endl;
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

void IndexSystemRecord_INFIMUM::dump(std::ostringstream &oss) const {
  using namespace std;
  oss << "INFIMUM: info flag: " << int(e(info_flag())) << "\t"
      << "num of recs owned: " << int((num_of_recs_owned())) << "\t"
      << "order: " << int(e(order())) << "\t"
      << "rec_type: " << int(e(rec_type())) << "\t"
      << "next rec offset: " << e(next_rec_offset) << "\t"
      << "text: " << infimum << endl;
}

void IndexSystemRecord_SUPREMUM::dump(std::ostringstream &oss) const {
  using namespace std;
  oss << "SUPREMUM: info flag: " << int(e(info_flag())) << "\t"
      << "num of recs owned: " << int((num_of_recs_owned())) << "\t"
      << "order: " << int(e(order())) << "\t"
      << "rec_type: " << int(e(rec_type())) << "\t"
      << "next rec offset: " << e(next_rec_offset) << "\t"
      << "text: " << supremum << endl;
}
