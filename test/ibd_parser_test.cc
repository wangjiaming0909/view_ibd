#include "table_reader.h"
#include "gtest/gtest.h"
#include <chrono>
#include <thread>

TEST(parser, index_page) {
  innodb::MySQLDataReader reader("/root/codes/mysql-8.0.42/build/data");
  [[maybe_unused]] auto *sbtest1_reader =
      reader.get_table_reader("test", "sbtest1");
  [[maybe_unused]] auto ibdata1_reader = reader.get_table_reader("", "ibdata1");

  using namespace std::chrono_literals;
  std::this_thread::sleep_for(100000s);
  // innodb::FileSpaceReader
  // reader("/root/codes/mysql-8.0.42/build/data/test/sbtest1.ibd");
  //  reader.dump_space();
}
