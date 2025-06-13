#pragma once
#include "page.h"
#include <fstream>
#include <functional>
#include <string>

namespace innodb {

/// @brief
class FileSpaceReader {
public:
  static constexpr int32_t FSP_HEADER_PAGE_NUM = 0;
  FileSpaceReader(const char *file);
  ~FileSpaceReader();

  /// @brief get the specified page
  /// @param index the index of the page
  /// @return nullptr if reader got error, other the pageptr is returned
  Page* get_page(unsigned int index);

  const Page* get_page(unsigned int index) const {
    return const_cast<FileSpaceReader*>(this)->get_page(index);
  }

  const FSPHeaderPage* get_fsp_header_page() const ;

  uint32_t get_page_count();

  void dump_space();

  using traverse_xdes_entry_func = std::function<void(const XDES_E &, Addr addr)>;
  void traverse_xdes_list(const ListBaseNode &base_node,
                          traverse_xdes_entry_func func);

  using traverse_inode_entry_func = std::function<void(const INode_E &, Addr addr)>;
  void traverse_inode_list(
      const ListBaseNode &base_node,
      std::function<void(const INodePage &, Addr addr)> func);

private:
  /// @brief open the file
  /// @return -1 when got error, check errno, 0 for succeed.
  int open_file();

  /// @brief read data from the opened file, call open_file first
  /// @param offset the offset to read from
  /// @param buf the buffer to store the data read
  /// @param size the size to read
  /// @return return the bytes read, -1 for error, check errno
  long read_page(std::streampos offset, unsigned char *buf,
                 std::streamsize size = PAGE_SIZE);

private:
  std::string file_name_;
  bool file_opened_;
  std::ifstream ifs_;
  std::vector<Page*> pages_;

  std::vector<XDES_E> full_frag_extents_;
  std::vector<XDES_E> free_frag_extents_;
  std::vector<XDES_E> free_extents_;
};
} // namespace innodb
