#include "ibdata1_reader.h"
#include <iostream>

namespace innodb {
void Ibdata1Reader::init() {
  // Read and process the FSP header page
  const auto* fsp_header_page = fsp_reader_.get_fsp_header_page();
  if (!fsp_header_page) {
    LOG(ERROR) << "Failed to get FSP header page from ibdata1 file.";
    return;
  }
  const auto *dict_hdr_page = fsp_reader_.get_page(FSP_DICT_HDR_PAGE_NO);
  if (!dict_hdr_page) {
    LOG(ERROR) << "Failed to get dictionary header page from ibdata1 file.";
    return;
  }
  using namespace std;
  ostringstream oss;
  dict_hdr_page->dump(oss);
  cout << "Dictionary Header Page:\n" << oss.str() << endl;
}
}