#pragma once
#include "page.h"
#include <unordered_map>

namespace innodb {

class FileSpaceReader;
namespace design {

struct BufferPool {
  BufferPool &instance();
  // @brief get the page from cache, read it if not exists, and add it to cache
  // after read, whichi could pop old pages.
  // @return null if the page_id is not added into BufferPool
  Page *get_page(PageID page_id);

  // @brief register a ibd file into the fil_name_ids_, read the PageID and
  // reigster it into fil_id_readers_
  // @param fil_name the fil_name to read
  // @return succeed return 0, else err is returned
  int add_fil(const char *fil_name);

private:
  BufferPool();
  ~BufferPool();

  std::unordered_map<SpaceID, FileSpaceReader *> fil_id_readers_;
  std::unordered_map<std::string, SpaceID> fil_name_ids_;

  // Pool Impl
private:
  // @brief try get the page from cache, which follow the LRU algorithm
  Page *get_pg(PageID id);
  void push_page(Page *pg);
  void pop_page(PageID id);
  std::unordered_map<PageID, Page*> cached_pgs_;
};

} // namespace design
} // namespace innodb
