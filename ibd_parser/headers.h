#pragma once
#include "defines.h"
#include <sstream>
#include <stdint.h>
#include <unordered_map>


#define PAGE_SIZE 16384
#define PAGE_BTR_SEG_LEAF 36

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

constexpr ulint FIL_ADDR_SIZE =6;
constexpr ulint FLST_BASE_NODE_SIZE = 4 + 2 * FIL_ADDR_SIZE;

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
    static constexpr uint8_t FSP_SPACE_ID = 0;
    static constexpr uint8_t FSP_NOT_USED = 4;
    static constexpr uint8_t FSP_SIZE = 8; // cur size of the space in pages
    static constexpr uint8_t FSP_FREE_LIMIT = 12;
    static constexpr uint8_t FSP_SPACE_FLAGS = 16;
    static constexpr uint8_t FSP_FRAG_N_USED = 20;
    static constexpr uint8_t FSP_FREE = 24;
    static constexpr uint8_t FSP_HEADER_OFFSET = FILHeader::FIL_PAGE_DATA;
    static constexpr auto FSP_HEADER_SIZE = 32+5*FLST_BASE_NODE_SIZE;

        static uint32_t space_id(const std::byte* pg) {
            return mach_read_from_4(pg + FSP_HEADER_OFFSET + FSP_SPACE_ID);
        }
  static uint32_t unused;
  static uint32_t fsp_size(const std::byte*pg)  {
    return mach_read_from_4(pg + FSP_HEADER_OFFSET + FSP_SIZE);
  }
  static uint32_t fsp_free_limit(const std::byte*pg) {
      return mach_read_from_4(pg + FSP_HEADER_OFFSET + FSP_FREE_LIMIT);
  }
  static uint32_t space_flags (const std::byte*pg) {
      return mach_read_from_4(pg + FSP_HEADER_OFFSET + FSP_SPACE_FLAGS);
  }
  static uint32_t frag_n_used(const std::byte *pg) {
      return mach_read_from_4(pg + FSP_HEADER_OFFSET + FSP_FRAG_N_USED);
  }
  static void dump(const std::byte *pg, std::ostringstream &oss);
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
  static constexpr uint8_t FSEG_HDR_SPACE = 0;
  static constexpr uint8_t FSEG_HDR_PAGE_NO = 4;
  static constexpr uint8_t FSEG_HDR_OFFSET = 8;
  static constexpr uint8_t FSEG_HEADER_SIZE = 10;
  static constexpr uint8_t FSEG_PAGE_DATA = FILHeader::FIL_PAGE_DATA;
  static const std::byte* fseg_header(const std::byte *pg) {
      return pg + FSPHeader::FSP_HEADER_OFFSET + PAGE_BTR_SEG_LEAF;
  }
  static uint32_t fseg_space(const std::byte *pg) {
    return mach_read_from_4(fseg_header(pg) + FSEG_HDR_SPACE);
  }
  static uint32_t fseg_hdr_page_no(const std::byte *pg) {
    return mach_read_from_4(fseg_header(pg) + FSEG_HDR_PAGE_NO);
  }
  static uint16_t fseg_hdr_offset(const std::byte* pg){
    return mach_read_from_2(fseg_header(pg) + FSEG_HDR_OFFSET);
  }
  static void dump(const std::byte *pg, std::ostringstream& oss);
};

const char* get_rec_type(uint8_t rec_t);

enum rec_type {
    REC_STATUS_ORDINARY = 0,
    REC_STATUS_NODE_PTR = 1,
    REC_STATUS_INFIMUM = 2,
    REC_STATUS_SUPREMUM = 3
};

constexpr uint8_t PAGE_HEADER = FSEG_HEADER::FSEG_PAGE_DATA;
constexpr uint8_t PAGE_DATA = PAGE_HEADER + 36 + 2 * FSEG_HEADER::FSEG_HEADER_SIZE;
constexpr uint8_t REC_N_EXTRA_BYTES = 5;
constexpr auto PAGE_NEW_INFIMUM = PAGE_DATA + REC_N_EXTRA_BYTES;
constexpr auto PAGE_NEW_SUPREMUM = PAGE_DATA + 2 * REC_N_EXTRA_BYTES + 8;
constexpr auto PAGE_NEW_SUPREMUM_END = PAGE_NEW_SUPREMUM + 8;

struct Records {
    static void dump(const std::byte*pg, std::ostringstream& oss);
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

  static inline uint8_t info_bits(const std::byte*rec) {
    return rec_get_bit_field_1(rec, REC_NEW_INFO_BITS, REC_INFO_BITS_MASK, REC_INFO_BITS_SHIFT);
  }
  static inline uint8_t num_of_recs_owned(const std::byte*rec) {
    return rec_get_bit_field_1(rec, REC_NEW_N_OWNED, REC_N_OWNED_MASK, REC_N_OWNED_SHIFT);
  }

  static inline uint16_t heap_no_new(const std::byte*rec) {
    return rec_get_bit_field_2(rec, REC_NEW_HEAP_NO, REC_HEAP_NO_MASK, REC_HEAP_NO_SHIFT);
  }
  static inline uint8_t rec_status(const std::byte*rec) {
    return rec_get_bit_field_1(rec, REC_NEW_STATUS, REC_NEW_STAUTS_MASK, REC_NEW_STATUS_SHIFT);
  }
  static inline uint16_t next_offs(const std::byte*rec) {
    ulint field_value = mach_read_from_2(rec - REC_NEXT);

    if (field_value == 0) return 0;
    return align_offset(rec + field_value, PAGE_SIZE);
  }

  static inline std::byte* next_ptr(const std::byte*rec) {
    ulint field_value = mach_read_from_2(rec - REC_NEXT);
    if (field_value == 0) return nullptr;

    return ((std::byte*)align_down(rec, PAGE_SIZE) + align_offset(rec + field_value, PAGE_SIZE));
  }

  static void dump(const std::byte *rec, std::ostringstream &oss);
};

struct IndexPageDirectory{};

} // namespace innodb
