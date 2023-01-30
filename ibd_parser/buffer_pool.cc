#include "buffer_pool.h"
#include "file_space_reader.h"
#include <atomic>
#include <glog/logging.h>
#include <mutex>
#include <thread>

namespace innodb {

buffer_pool_t &buffer_pool_t::instance() {
  static buffer_pool_t *instance = new buffer_pool_t();
  if (!instance->inited)
    return *(buffer_pool_t *)0;
  return *instance;
}
buffer_pool_t::buffer_pool_t()
    : fil_id_readers_(), fil_name_ids_(), raw_buffer_(0), buffer_(0), LRU(),
      FREE_LIST(), LRU_old_(nullptr), LRU_old_len_(0), lru_lock_(),
      free_list_lock_(), hash_lock_(), page_buffer_map_() {
  if (init_pool() == 0)
    inited = true;
}

ulint buffer_pool_t::LRU_old_ratio_ = 38; // 3/8

buffer_pool_t::~buffer_pool_t() {
  LRU_old_ = nullptr;

  auto it = LRU.begin();
  for (; it != LRU.end();) {
    auto* pg = it.pointed_node();
    it = LRU.erase(it);
    delete pg;
  }

  it = FREE_LIST.begin();
  for (; it != FREE_LIST.end();) {
    auto *pg = it.pointed_node();
    it = FREE_LIST.erase(it);
    delete pg;
  }

  free(raw_buffer_);
}

int buffer_pool_t::init_pool() {
  raw_buffer_ = (byte *)calloc(BUFFER_POOL_SIZE, 1);
  auto p = raw_buffer_ + PAGE_SIZE;
  buffer_ = (byte *)align_down(p, PAGE_SIZE);

  p = buffer_;
  auto *end = raw_buffer_ + BUFFER_POOL_SIZE;
  while (end - p > PAGE_SIZE) {
    auto *pg = new Page(PAGE_SIZE, p);
    FREE_LIST.push_front(*pg);
    p += PAGE_SIZE;
  }

  LRU_old_ = nullptr;
  LRU_old_len_ = 0;
  return 0;
}

FileSpaceReader *buffer_pool_t::get_reader(PageID page_id) {
  auto reader_it = fil_id_readers_.find(page_id.space_id);
  if (reader_it == fil_id_readers_.end()) {
    LOG(ERROR) << "reader is not found for pg: " << page_id.dump();
    return nullptr;
  }
  return reader_it->second;
}

Page *buffer_pool_t::read_pg(PageID page_id) {
  Page *pg = nullptr;

  // try to read it into mem
  auto reader = get_reader(page_id);
  if (!reader)
    return pg;

  // scan from the mid pointer at the first time, then we will scan from the
  // head of LRU list
  auto iterations_n = 0;
  while (!pg) {
    pg = find_free_page();
    if (!pg) {
      if (0 != evict_one_page(iterations_n > 0))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      iterations_n++;
      if (iterations_n > 20) {
        LOG(WARNING) << "fail to find one free page, try to enlarge pool size";
      }
      continue;
    }
  }
  BOOST_ASSERT(pg);
  // from now on we have locked this page

  if (0 != init_page_for_read(pg, page_id) || read_page(pg, reader) != 0) {
    auto ret = free_one_page(pg);
    BOOST_ASSERT(ret == 0);
    pg = nullptr;
  }
  return pg;
}

Page *buffer_pool_t::get_page(PageID page_id, page_fetch_t mode) {
  // try to find the page in cache
  auto *page = find_pg_in_cache(page_id);

  if (!page)
    page = read_pg(page_id);

  if (!page) return nullptr;

  page_make_young_if_needed(page, mode);
  // return the pg
  return page;
}

Page *buffer_pool_t::find_pg_in_cache(PageID id) {
  guard_t<lock_t> _{hash_lock_};
  auto it = page_buffer_map_.find(id);
  if (it != page_buffer_map_.end()) {
    return it->second;
  }
  return nullptr;
}

int buffer_pool_t::add_fil(const char *fil_name) {}

int buffer_pool_t::free_one_page(Page *pg) {
  BOOST_ASSERT(pg);
  BOOST_ASSERT(pg->locked_by_me());

  if (pg->fixed)
    return -1;

  if (pg->list_hook.is_linked()) {
    auto it = LRU.iterator_to(*pg);
    LRU.erase(it);
  }

  FREE_LIST.push_front(*pg);

  pg->unfix_page();
  pg->unlock_page();
  return 0;
}

int buffer_pool_t::read_page(Page *pg, FileSpaceReader *reader) {
  BOOST_ASSERT(reader);
  BOOST_ASSERT(pg);

  return reader->read_page(pg);
}

int buffer_pool_t::evict_one_page(bool scan_all) {

}

int buffer_pool_t::init_page_for_read(Page *pg, PageID id) {
  BOOST_ASSERT(pg);
  BOOST_ASSERT(!pg->fixed);
  BOOST_ASSERT(pg->locked_by_me());

  pg->page_id_ = id;

  pg->fix_page();
  pg->unlock_page();

  lru_add_page(pg, true);

  guard_t<lock_t> _{hash_lock_};
  page_buffer_map_[id] = pg;
  return 0;
}

Page *buffer_pool_t::find_free_page() {
  guard_t<lock_t> _{free_list_lock_};

  if (FREE_LIST.empty())
    return nullptr;
  auto &pg = FREE_LIST.back();
  FREE_LIST.pop_back();

  pg.lock_page();
  return &pg;
}

void buffer_pool_t::lru_add_page(Page *page, bool old) {
  guard_t<lock_t> _{lru_lock_};

  if (!old || LRU.size() < BUF_LRU_OLD_MIN_LEN) {
    LRU.push_front(*page);
  } else {
    BOOST_ASSERT(LRU_old_);
    LRU.insert(++LRU.iterator_to(*LRU_old_), *page);
    LRU_old_len_++;
  }

  if (LRU.size() > BUF_LRU_OLD_MIN_LEN) {
    BOOST_ASSERT(LRU_old_);
    page->set_old(old);
  } else if (LRU.size() == BUF_LRU_OLD_MIN_LEN) {
    for (auto& pg : LRU) {
      pg.set_old(true);
    }
    LRU_old_ = &LRU.front();
    LRU_old_len_ = LRU.size();
  } else {
    page->set_old(LRU_old_ != nullptr);
  }
}

void buffer_pool_t::page_make_young_if_needed(Page *pg, page_fetch_t mode) {
  BOOST_ASSERT(pg);

  guard_t<lock_t> _{lru_lock_};

  if (1)
    page_make_young(pg);
}

void buffer_pool_t::page_make_young(Page *pg) {
  BOOST_ASSERT(pg);

}

} // namespace innodb
