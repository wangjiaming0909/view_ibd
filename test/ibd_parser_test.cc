#include "page.h"
#include "file_space_reader.h"
#include "gtest/gtest.h"

void dump_page(innodb::FileSpaceReader&reader, int i) {
    auto pg = reader.get_page(i);
    if (!pg) {
      LOG(ERROR) << "Fail to got pg: " << i;
      return;
    }
    auto &fil_header = pg->get_fil_header();
    std::ostringstream oss;
    pg->dump(oss);
    printf("%s", oss.str().c_str());
}

TEST(parser, index_page) {
  innodb::FileSpaceReader reader(
              "/home/wjm/mysqldata/data/master/test/sbtest1.ibd");
      //"/media/wjm/A/tmp/sbtest1.ibd");
  for (int i = 0; i < 16; ++i) {
    dump_page(reader, i);
    printf("----------------------------------\n");
  }
}
