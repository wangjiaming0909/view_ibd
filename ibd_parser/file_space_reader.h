#pragma once
#include "page.h"
#include <fstream>
#include <string>

namespace innodb {

/// @brief
class FileSpaceReader {
public:
  FileSpaceReader(const char *file);
  ~FileSpaceReader();

  /// @brief get the specified page
  /// @param index the index of the page
  /// @return nullptr if reader got error, other the pageptr is returned
  PagePtr get_page(unsigned int index);

private:
  /// @brief open the file
  /// @return -1 when got error, check errno, 0 for succeed.
  int open_file();

  /// @brief read data from the opened file, call open_file first
  /// @param offset the offset to read from
  /// @param buf the buffer to store the data read
  /// @param size the size to read
  /// @return return the bytes read, -1 for error, check errno
  long read(std::streampos offset, char *buf, std::streamsize size);

private:
  std::string file_name_;
  bool file_opened_;
  std::ifstream ifs_;
};
} // namespace innodb
