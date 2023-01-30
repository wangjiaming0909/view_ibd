#include "buffer_pool.h"
#include "file_space_reader.h"
#include "page.h"
#include "gtest/gtest.h"
#include <filesystem>
#include "parser.h"
#include "table.h"

void dump_page(innodb::FileSpaceReader &reader, int i) {
  innodb::Page* pg = nullptr;
  if (!pg) {
    LOG(ERROR) << "Fail to got pg: " << i;
    return;
  }
  std::ostringstream oss;
  pg->dump(oss);
  printf("%s", oss.str().c_str());
}

TEST(parser, index_page) {
  innodb::FileSpaceReader reader(
      "/home/wjm/mysqldata/data/master/test/sbtest1.ibd");
      //"/media/wjm/A/tmp/sbtest1.ibd");
      //"/mnt/tmp/mysql.ibd");
  for (int i = 0; i < 6; ++i) {
    dump_page(reader, i);
    printf("----------------------------------\n");
  }
}

TEST(parser, scan_dir) {
  std::filesystem::path data_dir = "/root/mysqldata/data/master";
  Parser parser{data_dir.c_str()};
  ASSERT_EQ(0, parser.init());
  ASSERT_FALSE(parser.is_defined_dir((data_dir / "not_existing_folder").c_str()));
  ASSERT_TRUE(parser.is_defined_dir((data_dir / "mysql").c_str()));
  ASSERT_TRUE(parser.is_defined_dir((data_dir / "information_schema").c_str()));

  ASSERT_TRUE(parser.tbs_.size() > 0);
  ASSERT_TRUE(parser.tbs_.find("test.t1") != parser.tbs_.end());
}

TEST(buffer_pool, init) {
  auto pool = innodb::buffer_pool_t{};
  ASSERT_TRUE(pool.inited);
  ASSERT_EQ(pool.FREE_LIST.size(),
            innodb::buffer_pool_t::BUFFER_POOL_SIZE / (1024 * 16) - 1);
  ASSERT_EQ(nullptr, pool.LRU_old_);
  ASSERT_EQ(0, pool.LRU_old_len_);
  ASSERT_EQ(0, pool.LRU.size());

  for (auto &pg : pool.FREE_LIST) {
    ulint pointer = (ulint)pg.get_buf();
    ASSERT_EQ(0, pointer & 0x3fff);
  }
}

TEST(buffer_pool, get_page) {
  auto pool = innodb::buffer_pool_t{};
  innodb::PageID pg_id{0, 0};
  auto pg = pool.get_page(pg_id, innodb::page_fetch_t::NORMAL);
  ASSERT_EQ(nullptr, pg);
}

TEST(dd, t1) {
  Parser parser{"/root/mysqldata/data/master"};
  ASSERT_EQ(0, parser.init());
  auto *t1 = parser.get_table("test", "t1");
  ASSERT_TRUE(t1 != nullptr);
  auto clust_idx = t1->get_cluster_idx();
  ASSERT_TRUE(clust_idx != nullptr);
}
