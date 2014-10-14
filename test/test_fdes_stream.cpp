/**
 * Verify fdes_stream works as expected
 */

#include <cppunit/extensions/HelperMacros.h>
#include <fcntl.h>
#include "fdes_stream.hpp"
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

class test_fdes_stream : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(test_fdes_stream);
  CPPUNIT_TEST(test_fdes);
  CPPUNIT_TEST_SUITE_END();

public:

  void test_fdes()
  {
    std::string filename("blarg.txt");
    int fd = open(filename.c_str(), O_RDWR | O_CREAT, 0755);
    // Normally you wouldn't new it but I want to close it out and re-open
    // it to read back my text in a minute.
    fr::socket::fdes_stream *blarg = new fr::socket::fdes_stream(fd);
    std::string expected("Blarg!");
    std::string actual;
    std::ostream &blarg_out = blarg->get_ostream();
    blarg_out << expected << std::endl;
    delete(blarg);
    fd = open(filename.c_str(), O_RDWR);
    blarg = new fr::socket::fdes_stream(fd);
    std::istream &blarg_in = blarg->get_istream();
    blarg_in >> actual;
    CPPUNIT_ASSERT(expected == actual);
    delete(blarg);
    unlink(filename.c_str());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(test_fdes_stream);
