#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <glog/logging.h>
#include <iomanip>
#include <ostream>

using namespace std;
void custom_glog_prefix(std::ostream &s, const google::LogMessageInfo &l,
                        void *) {
  s << l.severity[0] << " " << setw(4) << 1900 + l.time.year() << setw(2)
    << 1 + l.time.month() << setw(2) << l.time.day() << ' ' << setw(2)
    << l.time.hour() << ':' << setw(2) << l.time.min() << ':' << setw(2)
    << l.time.sec() << "." << setw(6) << l.time.usec() << ' ' << setfill(' ')
    << setw(5) << l.thread_id << setfill('0') << ' ' << l.filename << ':'
    << l.line_number << "]";
}

void setup_logger(const char* argv0) {
  google::InitGoogleLogging(argv0, custom_glog_prefix);
}

int main(int argc, char **argv) {
  fLB::FLAGS_logtostdout = false;
  fLB::FLAGS_logtostderr = false;
  fLS::FLAGS_log_dir = "/tmp";
  google::SetLogDestination(google::GLOG_INFO, "/tmp/INFO_");
  setup_logger(argv[0]);

  ::testing::InitGoogleMock(&argc, argv);
  LOG(INFO) << "start run tests" << endl;
  return RUN_ALL_TESTS();
}
