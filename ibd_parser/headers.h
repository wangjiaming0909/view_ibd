#pragma once
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

struct FILHeader {
  uint32_t check_sum;
  uint32_t page_number_offset;
  uint32_t previous_page;
  uint32_t next_page;
  uint64_t last_mod_page_lsn;
  uint16_t page_type;
  uint64_t flush_lsn;
  uint32_t space_id;
  static constexpr unsigned char FILHEADER_SIZE = 38;
  void dump(std::ostringstream &oss) const;
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
  uint16_t n_of_dir_slots;
  uint16_t heap_top_pos;
  uint16_t n_of_heap_recs_or_ft_fg;
  uint16_t first_garbage_rec_offset;
  uint16_t garbage_space;
  uint16_t last_insert_pos;
  uint16_t pg_direction;
  uint16_t n_of_inserts_in_pg_direction;
  uint16_t n_of_recs;
  uint64_t max_trx_id;
  uint16_t page_level;
  uint64_t index_id;
  static constexpr unsigned int INDEX_HEADER_SIZE = 74 - 38;
  void dump(std::ostringstream &oss) const;
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

struct IndexSystemRecord_INFIMUM {
  uint8_t info_flag() const {return (buf >> 4) << 4;}
  uint8_t num_of_recs_owned() const {return buf << 4 >> 4;}
  uint16_t order() const {return (order_and_rec_type >> 3) << 3;}
  uint8_t rec_type() const {
    const uint8_t *p = reinterpret_cast<const uint8_t*>(&order_and_rec_type);
    p += 1;
    return *p << 5;
  }
  //int info_flag : 4;
  //int num_of_recs_owned : 4;
  uint8_t buf;
  //int order : 13;
  //int rec_type : 3;
  uint16_t order_and_rec_type;
  uint16_t next_rec_offset;
  char infimum[8];
  static constexpr unsigned int INDEX_SYSTEM_RECORD_SIZE = 107 - 94;
  void dump(std::ostringstream& oss) const;
} __attribute__((packed));
static const int i = sizeof(IndexSystemRecord_INFIMUM);

struct IndexSystemRecord_SUPREMUM {
  uint8_t info_flag() const {return buf >> 4 << 4;}
  uint8_t num_of_recs_owned() const {return (buf << 4 >>4);}
  uint16_t order() const{return order_and_rec_type >> 3 << 3;}
  uint8_t rec_type() const {
    const uint8_t *p = reinterpret_cast<const uint8_t*>(&order_and_rec_type);
    p += 1;
    return *p << 5;
  }
  uint8_t buf;
  uint16_t order_and_rec_type;
  uint16_t next_rec_offset;
  char supremum[8];
  static constexpr unsigned int INDEX_SYSTEM_RECORD_SIZE = 120 - 107;
  void dump(std::ostringstream& oss) const;
} __attribute__((packed));

struct UserRecord{};

struct IndexPageDirectory{};

} // namespace innodb
