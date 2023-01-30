#include "dd_cache.h"
#include "dd_properties.h"
#include <cassert>

namespace innodb {
namespace dd {

DD_Cache::DD_Cache() : table_cache_() {
  init_dd_properties();
}

DD_Cache &DD_Cache::instance() {
  static DD_Cache *instance = new DD_Cache();
  return *instance;
}

void DD_Cache::add_to_cache(DD_Abstract_Table* tb) {
  assert(tb);
  table_cache_.emplace(tb->tb_full_name().c_str(), tb);
}

void DD_Cache::init_dd_properties() {
  auto *dd_prop = new DD_Properties();
  dd_prop->load_properties();
  add_to_cache(dd_prop);
}

} // namespace dd
} // namespace innodb
