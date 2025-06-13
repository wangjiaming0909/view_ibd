#include "file_space_reader.h"
#include <cassert>
#include <filesystem>
#include <glog/logging.h>
#include <iostream>

using namespace innodb;

FileSpaceReader::FileSpaceReader(const char *file)
    : file_name_(file), file_opened_(false), ifs_() {
  assert(file);
}
FileSpaceReader::~FileSpaceReader() {
  if (ifs_.is_open())
    ifs_.close();
}

Page *FileSpaceReader::get_page(unsigned int index) {
  if (index >= pages_.size() || pages_[index] == nullptr) {
    // read page and set into pages_
    std::streampos offset{};
    offset = index * PAGE_SIZE;
    Page *page = nullptr;
    unsigned char *buf = (unsigned char *)calloc(1, PAGE_SIZE);
    if (0 > read_page(offset, buf, PAGE_SIZE)) {
      LOG(ERROR) << "read page error at index: " << index;
      free(buf);
      return nullptr;
    }
    Page::init_page((const byte *)buf, &page);
    pages_.resize(index + 1, nullptr);
    pages_[index] = page;
    return page;
  } else {
    return pages_[index];
  }
}

long FileSpaceReader::read_page(std::streampos offset, unsigned char *buf,
                                std::streamsize size) {
  if (!ifs_.is_open() && 0 != open_file()) {
    return -1;
  }
  std::streamsize bytes_read = 0;
  try {
    auto &s = ifs_.seekg(offset);
    bytes_read = s.readsome((char *)buf, size);
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

const FSPHeaderPage *FileSpaceReader::get_fsp_header_page() const {
  return static_cast<const FSPHeaderPage *>(get_page(FSP_HEADER_PAGE_NUM));
}

uint32_t FileSpaceReader::get_page_count() {
  auto fsp_header_page = get_fsp_header_page();
  if (!fsp_header_page) {
    LOG(ERROR) << "Fail to get fsp header page";
    return 0;
  }
  return fsp_header_page->fsp_header_.fsp_size_;
}

void FileSpaceReader::dump_space() {
  using namespace std;
  std::ostringstream oss;
  oss << "space file: " << this->file_name_ << std::endl;
  printf("%s", oss.str().c_str());
  oss.str("");

  const auto *fsp_header_page = get_fsp_header_page();
  oss << "fsp header page: " << "\n";
  fsp_header_page->dump(oss);
  printf("%s", oss.str().c_str());
  oss.str("");
  for (uint32_t i = 1; i < 5; ++i) {
    auto pg = get_page(i);
    if (!pg) {
      LOG(ERROR) << "Fail to get page: " << i;
      continue;
    }
    oss << "page: " << i << std::endl;
    pg->dump(oss);
    auto s = oss.str();
    std::cout << s << std::endl;
    oss.str("");
  }
  oss << "============================================" << endl;
  auto dump_xdes_entry = [&](const XDES_E &xdes_e, Addr addr) {
    xdes_e.dump(oss, addr);
  };
  oss << "\n-----------------------------------\nfull frag extents: " << endl;
  traverse_xdes_list(fsp_header_page->get_full_frag_list_base_node(),
                     dump_xdes_entry);
  printf("%s", oss.str().c_str());
  oss.str("");
  oss << "\n---------------------------------------\nfree frag extents: "
      << endl;
  traverse_xdes_list(fsp_header_page->get_free_frag_list_base_node(),
                     dump_xdes_entry);
  printf("%s", oss.str().c_str());
  oss.str("");
  oss << "\n----------------------------------------\nfree extents: " << endl;
  traverse_xdes_list(fsp_header_page->get_free_list_base_node(),
                     dump_xdes_entry);
  printf("%s\n", oss.str().c_str());
  oss.str("");

  auto dump_inode_entries = [&](const INodePage &inode_page, Addr addr) {
    oss << "inode page at: ";
    addr.dump(oss);
    oss << endl;
    for (size_t i = 0; i < inode_page.inode_arr_.size(); ++i) {
      const INode_E &inode_entry = inode_page.inode_arr_.at(i);
      if (inode_entry.full_list_base_node_.list_length_ > 0) {
        oss << "xdes full list with size: "
            << inode_entry.full_list_base_node_.list_length_ << ": ";
        traverse_xdes_list(inode_entry.full_list_base_node_, dump_xdes_entry);
        oss << endl;
      }
      if (inode_entry.not_full_list_base_node_.list_length_ > 0) {
        oss << "------------------------------------------------------" << endl
            << "xdes not full list with size: "
            << inode_entry.not_full_list_base_node_.list_length_ << ": ";
        traverse_xdes_list(inode_entry.not_full_list_base_node_,
                           dump_xdes_entry);
        oss << endl;
      }
    }
  };
  oss << "\n -------------------------------------------------\nfull inode "
         "list: "
      << endl;
  traverse_inode_list(fsp_header_page->get_full_inodes_list_base_node(),
                      dump_inode_entries);
  printf("%s", oss.str().c_str());
  oss.str("");
  oss << "\n ------------------------------------------------\nfree inode "
         "list: "
      << endl;
  traverse_inode_list(fsp_header_page->get_free_inodes_list_base_node(),
                      dump_inode_entries);
  printf("%s", oss.str().c_str());
  oss.str("");
}

void FileSpaceReader::traverse_xdes_list(const ListBaseNode &base_node,
                                         traverse_xdes_entry_func func) {
  Addr cur{};
  if (base_node.list_length_ > 0) {
    cur.page_number_ = base_node.first_page_number_;
    cur.offset_ = base_node.first_offset_;
    while (cur.valid()) {
      auto pg = get_page(cur.page_number_);
      uint32_t entry_num = (cur.offset_ - FILHeader::FIL_PAGE_DATA -
                            FSPHeader::FSP_HEADER_SIZE) /
                           XDES_E::XDES_E_SIZE;
      const XDES_E *xdes_entry = pg->get_xdes_entry(entry_num);
      func(*xdes_entry, cur);
      cur.page_number_ = xdes_entry->list_node_for_xdes_e_.next_page_number_;
      cur.offset_ = xdes_entry->list_node_for_xdes_e_.next_offset_;
    }
  }
}
void FileSpaceReader::traverse_inode_list(
    const ListBaseNode &base_node,
    std::function<void(const INodePage &, Addr addr)> func) {
  Addr cur{};
  if (base_node.list_length_ > 0) {
    cur.page_number_ = base_node.first_page_number_;
    cur.offset_ = base_node.first_offset_;
    while (cur.valid()) {
      auto pg = get_page(cur.page_number_);
      const INodePage *inode_page = static_cast<INodePage *>(pg);
      func(*inode_page, cur);
      cur.page_number_ =
          inode_page->list_node_for_INODE_page_list_.next_page_number_;
      cur.offset_ = inode_page->list_node_for_INODE_page_list_.next_offset_;
    }
  }
}
