#include "table_loader.h"
#include "dd_cache.h"
#include <glog/logging.h>
#include "dd_properties.h"

namespace innodb {

TableLoader::TableLoader(Table &tb) : tb_(tb) {}

dd::DD_Cache& TableLoader::dd_cache() {
  return dd::DD_Cache::instance();
}

int TableLoader::load() {
  int ret;
  do {
    if (tb_.is_dd_tb()) {
      if (0 != (ret = fill_properties()))
        break;
    } else {
      if (0 != (ret = fill_table()))
        break;
      if (0 != (ret = fill_index()))
        break;
    }
  } while (0);
  return ret;
}

int TableLoader::fill_properties() {
  if (tb_.tb_full_name() == dd::DD_Properties::NAME)
    return 0;
  dd::DD_Properties *dd_properties =
      dd_cache().get_dd_table<dd::DD_Properties>();

  std::string ret;
  dd_properties->get_property("", ret);

}

int TableLoader::fill_table() {
  dd::DD_Tables *dd_tables = dd_cache().get_dd_table<dd::DD_Tables>();
}

int TableLoader::fill_index() {

}

} // namespace innodb
