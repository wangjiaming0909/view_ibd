#include "dd_properties.h"
#include <glog/logging.h>

namespace innodb {
namespace dd {

DD_Properties::DD_Properties()
    : DD_Abstract_Table(DD_SPACE, NAME, DD_PROPETIES_PAGE_NO) {
  init();
}

void DD_Properties::init() { auto *clust_idx = new Index();
  clust_idx->set_sp_id(DD_SPACE_ID);
  clust_idx->set_tb(this);
  clust_idx->set_n_uniqs(1);
  clust_idx->set_page_no(DD_PROPETIES_PAGE_NO);
  indexes_.push_back(clust_idx);
}

int DD_Properties::load_properties() {
  Index::Rec_Oper_t oper = [](byte *) {};

  auto clust_idx = indexes_.front();
  auto ret = clust_idx->search(oper, Index::Always_True_Comp);
}

int DD_Properties::get_property(const char* key, std::string& ret) {
  if (properties_map_.empty()) {
    if (0 != load_properties()) {
      return -1;
    }
  }

  auto it = properties_map_.find(key);
  if (it == properties_map_.end()) return -1;
  ret = it->second;
  return 0;
}


} // namespace dd
} // namespace innodb
