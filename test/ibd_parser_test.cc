#include "file_space_reader.h"
#include "page.h"
#include "gtest/gtest.h"
#include <filesystem>
#include "parser.h"

void dump_page(innodb::FileSpaceReader &reader, int i) {
  auto pg = reader.get_page(i);
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
