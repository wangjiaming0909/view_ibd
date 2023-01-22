#include "file_space_reader.h"
#include <cassert>
#include <filesystem>
#include <glog/logging.h>

using namespace innodb;

FileSpaceReader::FileSpaceReader(const char *file)
    : file_name_(file), file_opened_(false), ifs_() {
  assert(file);
}
FileSpaceReader::~FileSpaceReader() {
  if (ifs_.is_open())
    ifs_.close();
}

PagePtr FileSpaceReader::get_page(unsigned int index) {
  std::streampos offset{};
  offset = index * PAGE_SIZE;
  PagePtr pg{new Page{PAGE_SIZE}};
  pg->set_page_no(index);
  auto *buf = pg->get_buf();

  if (0 > read(offset, (char*)buf, PAGE_SIZE)) {
      return nullptr;
  }
  return pg;
}

long FileSpaceReader::read(std::streampos offset, char *buf,
                           std::streamsize size) {
  if (!ifs_.is_open() && 0 != open_file()) {
    return -1;
  }
  std::streamsize bytes_read = 0;
  try {
    auto &s = ifs_.seekg(offset);
    bytes_read = s.readsome(buf, size);
  } catch (std::ios_base::failure &e) {
    LOG(ERROR) << "file read err " << e.code() << " " << e.what();
    return -1;
  }

  return bytes_read;
}

int FileSpaceReader::open_file() {
  if (!std::filesystem::exists(std::filesystem::path(file_name_))) {
    LOG(ERROR) << "File Space reader open file error, file not exists "
               << file_name_;
    return -1;
  }
  ifs_.open(file_name_, std::ios::in);
  if (!ifs_.is_open()) {
    LOG(ERROR) << "file " << file_name_ << " isn't opened";
    return -1;
  }
  return 0;
}
