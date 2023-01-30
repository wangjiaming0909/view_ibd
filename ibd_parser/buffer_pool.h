#pragma once
#include "page.h"
#include <functional>
#include <list>
#include <mutex>
#include <unordered_map>

namespace innodb {

class FileSpaceReader;

enum class page_fetch_t { NORMAL, SCAN };

#define BUF_LRU_OLD_MIN_LEN 64

struct buffer_pool_t {
  static constexpr ulint BUFFER_POOL_SIZE =
      1024 * 1024 * 100; // 100M = 1024 * 16 * 6400
  static buffer_pool_t &instance();
  /// @brief get the page from cache, read it if not exists, and add it to cache
  /// after read, whichi could pop old pages.
  /// @param[in] mode fetch mode, NORMAL or SCAN
  /// @return null if the page_id is not added into BufferPool
  Page *get_page(PageID page_id, page_fetch_t mode);

  /// @brief register a ibd file into the fil_name_ids_, read the PageID and
  /// reigster it into fil_id_readers_
  /// @param fil_name the fil_name to read
  /// @return succeed return 0, else err is returned
  int add_fil(const char *fil_name);

PRIVATE:
  buffer_pool_t();
  ~buffer_pool_t();
  int init_pool();
  bool inited = false;

  std::unordered_map<SpaceID, FileSpaceReader *> fil_id_readers_;
  std::unordered_map<std::string, SpaceID> fil_name_ids_;

  /// Pool Impl
PRIVATE:
  /// @brief try get the page from cache, which follow the LRU algorithm
  Page *find_pg_in_cache(PageID id);
  Page *read_pg(PageID id);
  /// @brief try to find free page from free list
  /// if free list is empty, should try to evict one from the LRU list, and
  /// retry to get one free page
  /// @note if one page is returned, then it has been remove from free list
  Page *find_free_page();

  /// @brief free page, this page will be removed from LRU list and added
  /// into free list
  /// @return 0 if succeed, -1 if the page is fixed by someone else
  int free_one_page(Page* pg);

  /// @brief try to scan the list and evict one page from LRU list to free list
  /// @param[in] scan_all true means to scan from the LRU HEAD, else scan from
  /// the old list HEAD
  /// @return 0 if succeed
  int evict_one_page(bool scan_all);

  /// @brief after got one free page, try to init it for read from disk, the
  /// page will be add into LRU list
  /// @param[in,out] pg the page to be inited
  /// @param[in] id the id will be assigned to the page
  /// @return 0 for succeed
  /// @note assume that we have acquired the page lock
  int init_page_for_read(Page *pg, PageID id);

  /// @brief read data into the page, do the actual io
  int read_page(Page* pg, FileSpaceReader* reader);

  /// @brief check if the page is behind the LRU_old_, try to make it young if
  /// not doing scanning
  /// @param[in] mode fetch mode
  void page_make_young_if_needed(Page *pg, page_fetch_t mode);

  /// @brief move the pg to the head of LRU list
  void page_make_young(Page *pg);

  /// @brief add page into lru list
  /// @param[in] page
  /// @param[in] old add to lru HEAD or old HEAD
  void lru_add_page(Page *page, bool old);

  /// @brief get the reader for this PageID
  /// @return nullptr if not found
  FileSpaceReader* get_reader(PageID id);

  byte *raw_buffer_;
  byte *buffer_;

  page_list_t LRU;
  page_list_t FREE_LIST;
  Page* LRU_old_;
  ulint LRU_old_len_;

  lock_t lru_lock_;
  lock_t free_list_lock_;
  lock_t hash_lock_;

  std::unordered_map<PageID, Page *, PageID_Hash> page_buffer_map_;

  static ulint LRU_old_ratio_;
};

} // namespace innodb
