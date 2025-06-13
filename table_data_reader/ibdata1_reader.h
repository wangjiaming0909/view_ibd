#pragma once
#include <string>
#include <cassert>
#include "file_space_reader.h"

namespace innodb {
class Ibdata1Reader {
    std::string file_name_;
    FileSpaceReader fsp_reader_;
    static constexpr int32_t DATA_DICT_FILE_SPACE_ID = 0;
    static constexpr int32_t FSP_DICT_HDR_PAGE_NO = 7;
public:
  Ibdata1Reader(const char *file) : file_name_(file), fsp_reader_(file) {
    init();
  }

private:
  void init();
};
}