#pragma once
#include "defines.h"
#include <sstream>
#include <stdint.h>
#include <unordered_map>

namespace innodb {

enum PAGE_TYPE {
  FIL_PAGE_TYPE_ALOCATED = 0,
  FIL_PAGE_TYPE_UNUSED = 1,
  FIL_PAGE_UNDO_LOG = 2,
  FIL_PAGE_INODE = 3,
  FIL_PAGE_IBUF_FREE_LIST = 4,
  FIL_PAGE_IBUF_BITMAP = 5,
  FIL_PAGE_TYPE_SYS = 6,
  FIL_PAGE_TYPE_TRX_SYS = 7,
  FIL_PAGE_TYPE_FSP_HDR = 8,
  FIL_PAGE_TYPE_XDES = 9,
  FIL_PAGE_TYPE_UNKNOWN = 13,
  FIL_PAGE_SDI = 17853,
  FIL_PAGE_RTREE = 17854,
  FIL_PAGE_INDEX = 17855
};

class Page;

struct FILHeader {
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

  static uint32_t check_sum(const std::byte* p) {return mach_read_from_4(p + FIL_PAGE_SPACE_OR_CHKSUM);}
  static uint32_t page_number_offset(const std::byte*p) {return mach_read_from_4(p + FIL_PAGE_OFFSET);}
  static uint32_t previous_page(const std::byte*p) {return mach_read_from_4(p + FIL_PAGE_PREV);}
  static uint32_t next_page(const std::byte*p) {return mach_read_from_4(p + FIL_PAGE_NEXT);}
  static uint64_t last_mod_page_lsn(const std::byte*p) {return mach_read_from_8(p + FIL_PAGE_LSN);}
  static uint16_t page_type(const std::byte*p) {return mach_read_from_2(p + FIL_PAGE_TYPE);}
  static uint64_t flush_lsn(const std::byte*p) {return mach_read_from_8(p + FIL_PAGE_FILE_FLUSH_LSN);}
  static uint32_t space_id(const std::byte*p) { return mach_read_from_4(p + FIL_PAGE_SPACE_ID);}
  static void dump(const std::byte* b, std::ostringstream &oss);
};

extern const std::unordered_map<uint16_t, std::string> PAGE_TYPE_STR;
std::string get_page_type_str(uint16_t page_type);

struct FSPHeader {
  uint32_t space_id;
  uint32_t unused;
  uint32_t highest_page_number_in_file;
  uint32_t highest_page_number_initialized;
  uint32_t flags;
  uint32_t no_pages_in_free_frag;
  unsigned char list_base_node_for_free_list[16];
  unsigned char list_base_node_for_free_frag_list[16];
  unsigned char list_base_node_for_full_frag_list[16];
  uint64_t next_unused_seg_id;
  unsigned char list_base_node_for_full_inodes_list[16];
  unsigned char list_base_node_for_free_inodes_list[16];
  static constexpr unsigned char FSPHEADER_SIZE = 150 - 38;
  void dump(std::ostringstream &oss) const;
};

struct XDES_E {
  uint64_t fi_seg_id;
  unsigned char list_node_for_xdes[16];
  uint32_t state;
  unsigned char page_state_bit_map[16];
  static constexpr unsigned char XDES_E_SIZE = 40;
};
struct INode_E {
  uint64_t fseg_id;
  uint32_t n_of_used_pgs_in_not_null_list;
};

struct IndexHeader {

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


  static uint16_t n_of_dir_slots(const std::byte*b) {return mach_read_from_2(b + PAGE_HEADER + PAGE_N_DIR_SLOTS);}
  static uint16_t heap_top_pos(const std::byte* b) {return mach_read_from_2(b + PAGE_HEADER + PAGE_HEAP_TOP);}
  static uint16_t n_of_heap_recs_or_ft_fg(const std::byte*b){return mach_read_from_2(b + PAGE_HEADER + PAGE_N_HEAP);}
  static uint16_t first_garbage_rec_offset(const std::byte*b) {return mach_read_from_2(b + PAGE_HEADER + PAGE_FREE);}
  static uint16_t garbage_space;
  static uint16_t last_insert_pos;
  static uint16_t pg_direction(const std::byte*b) {return mach_read_from_2(b + PAGE_HEADER + PAGE_DIRECTION);}
  static uint16_t n_of_inserts_in_pg_direction;
  static uint16_t n_of_recs(const std::byte* b) {return mach_read_from_2(b + PAGE_HEADER + PAGE_N_RECS);}
  static uint64_t max_trx_id (const std::byte*b) {return mach_read_from_8(b + PAGE_HEADER + PAGE_MAX_TRX_ID);}
  static uint16_t page_level(const std::byte*b) {return mach_read_from_2(b + PAGE_HEADER + PAGE_LEVEL);}
  static uint64_t index_id(const std::byte*b) {return mach_read_from_8(b + PAGE_HEADER + PAGE_INDEX_ID);}
  static constexpr unsigned int INDEX_HEADER_SIZE = 74 - 38;
  static void dump(const std::byte*b, std::ostringstream &oss);
};

struct FSEG_HEADER {
  uint32_t leaf_pg_inode_space_id;
  uint32_t leaf_pg_inode_pg_num;
  uint16_t leaf_pg_inode_offset;
  uint32_t internal_inode_space_id;
  uint32_t unternal_inode_pg_num;
  uint16_t internal_inode_offset;
  static constexpr unsigned int FSEG_HEADER_SIZE = 94 - 74;
  void dump(std::ostringstream& oss) const;
};

const char* get_rec_type(uint8_t rec_t);

enum rec_type {
    REC_STATUS_ORDINARY = 0,
    REC_STATUS_NODE_PTR = 1,
    REC_STATUS_INFIMUM = 2,
    REC_STATUS_SUPREMUM = 3
};

struct RecordHeader {
    uint8_t buf[5];
    uint8_t info_flag() const {return buf[0] >> 4;}
    uint8_t num_of_recs_owned() const {return buf[0] & 0xfUL;}
    uint16_t order() const {return reinterpret_cast<const uint16_t*>(buf+1)[0] >> 3;}
    uint8_t rec_type() const { return buf[2] & 0x7UL;}
    uint16_t next_rec_offset() const {return reinterpret_cast<const uint16_t*>(buf+3)[0];}
};

struct IndexSystemRecord_INFIMUM{
  uint8_t info_flag() const {return header.info_flag();}
  uint8_t num_of_recs_owned() const {return header.num_of_recs_owned();}
  uint16_t order() const {return header.order();}
  uint8_t rec_type() const { return header.rec_type();}
  uint16_t next_rec_offset() const {return header.next_rec_offset();}
  RecordHeader header;
  char infimum[8];
  static constexpr unsigned int INDEX_SYSTEM_RECORD_SIZE = 107 - 94;
  void dump(std::ostringstream& oss) const;
};
static const int i = sizeof(IndexSystemRecord_INFIMUM);

struct IndexSystemRecord_SUPREMUM {
  uint8_t info_flag() const {return header.info_flag();}
  uint8_t num_of_recs_owned() const {return header.num_of_recs_owned();}
  uint16_t order() const {return header.order();}
  uint8_t rec_type() const { return header.rec_type();}
  uint16_t next_rec_offset() const {return header.next_rec_offset();}
  RecordHeader header;

  char supremum[8];
  static constexpr unsigned int INDEX_SYSTEM_RECORD_SIZE = 120 - 107;
  void dump(std::ostringstream& oss) const;
};

struct UserRecord{
  uint8_t info_flag() const {return header.info_flag();}
  uint8_t num_of_recs_owned() const {return header.num_of_recs_owned();}
  uint16_t order() const {return header.order();}
  uint8_t rec_type() const { return header.rec_type();}
  uint16_t next_rec_offset() const {return header.next_rec_offset();}
  RecordHeader header;
};

struct IndexPageDirectory{};

} // namespace innodb
