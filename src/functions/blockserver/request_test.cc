#include <algorithm>
#include <string>

#include <boost/test/tools/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "request.hh"

using namespace std;
using namespace mcmap::server;

char const *test_request[] = {"GET MMP/1.0\r\n"
                              "Coord-X: 10\r\n"
                              "Coord-Z: 20\r\n"
                              "Dimension: overworld\r\n"
                              "\r\n",

                              "GET MMP/1.0\r\n"
                              "Coord-X:10\r\n"
                              "Coord-Z:20\r\n"
                              "Dimension:overworld\r\n"
                              "\r\n",

                              "GET MMP\r\n"
                              "\r\n",

                              "MMP/1.0\r\n"
                              "\r\n",

                              "\r\n"
                              "\r\n",

                              "GET MMP/1.0\r\n"
                              "Coord-X:\r\n"
                              "Coord-Z: \r\n"
                              "\r\n",

                              "GET MMP/1.0\r\n"
                              "Coord-X:\r\n"
                              ": 10\r\n"
                              "\r\n",

                              "GET MMP/1.0\r\n"
                              "Field-01: foo\r\n"
                              "Field-01: foo\r\n"
                              "Field-02: foo\r\n"
                              "Field-03: foo\r\n"
                              "Field-04: foo\r\n"
                              "Field-05: foo\r\n"
                              "Field-06: foo\r\n"
                              "Field-07: foo\r\n"
                              "Field-08: foo\r\n"
                              "Field-09: foo\r\n"
                              "Field-10: foo\r\n"
                              "\r\n",

                              "GET MMP/1.0\r\n"
                              "Coord-Z: 10\r\n"
                              "Coord-Z: 20\r\n"
                              "\r\n",

                              "GET MMP/1.0\n"
                              ""};

class test_reader : public reader {
    size_t off = 0;
    char const *test_data;

public:
    test_reader(int index) { test_data = test_request[index]; }

    ssize_t fill_buffer(char *buf, size_t len, size_t off) {
        if (!test_data[this->off]) return -1;
        size_t N = min(len - off, (size_t)5);
        for (size_t i = 0; i < N; ++i) {
            if (!test_data[this->off]) {
                return i;
            }
            buf[off + i] = test_data[this->off];
            ++(this->off);
        }
        return N;
    }
};

BOOST_AUTO_TEST_CASE(request_normal) {
    reader *reader = new test_reader(0);
    request *req = new request(reader);
    BOOST_TEST(req->parse_all());
    BOOST_TEST(req->get_method() == "GET");
    BOOST_TEST(req->get_protocol() == "MMP");
    BOOST_TEST(req->get_version() == "1.0");
    BOOST_TEST(req->get_field_count() == 3);
    BOOST_TEST(req->get_request_field("Coord-X") == "10");
    BOOST_TEST(req->get_request_field("Coord-Z") == "20");
    BOOST_TEST(req->get_request_field("Dimension") == "overworld");
    delete req;
    delete reader;
}

BOOST_AUTO_TEST_CASE(request_normal_no_space) {
    reader *reader = new test_reader(1);
    request *req = new request(reader);
    BOOST_TEST(req->parse_all());
    BOOST_TEST(req->get_method() == "GET");
    BOOST_TEST(req->get_protocol() == "MMP");
    BOOST_TEST(req->get_version() == "1.0");
    BOOST_TEST(req->get_field_count() == 3);
    BOOST_TEST(req->get_request_field("Coord-X") == "10");
    BOOST_TEST(req->get_request_field("Coord-Z") == "20");
    BOOST_TEST(req->get_request_field("Dimension") == "overworld");
    delete req;
    delete reader;
}

BOOST_AUTO_TEST_CASE(request_illegal_sig_0) {
    reader *reader = new test_reader(2);
    request *req = new request(reader);
    BOOST_TEST(!req->parse_all());
    delete req;
    delete reader;
}

BOOST_AUTO_TEST_CASE(request_illegal_sig_1) {
    reader *reader = new test_reader(3);
    request *req = new request(reader);
    BOOST_TEST(!req->parse_all());
    delete req;
    delete reader;
}

BOOST_AUTO_TEST_CASE(request_illegal_sig_2) {
    reader *reader = new test_reader(4);
    request *req = new request(reader);
    BOOST_TEST(!req->parse_all());
    delete req;
    delete reader;
}

BOOST_AUTO_TEST_CASE(request_empty_field) {
    reader *reader = new test_reader(5);
    request *req = new request(reader);
    BOOST_TEST(req->parse_all());
    BOOST_TEST(req->get_field_count() == 2);
    BOOST_TEST(req->get_request_field("Coord-X") == "");
    BOOST_TEST(req->get_request_field("Coord-Z") == "");
    delete req;
    delete reader;
}

BOOST_AUTO_TEST_CASE(request_illegal_field) {
    reader *reader = new test_reader(6);
    request *req = new request(reader);
    BOOST_TEST(!req->parse_all());
    delete req;
    delete reader;
}

BOOST_AUTO_TEST_CASE(too_many_fields) {
    reader *reader = new test_reader(7);
    request *req = new request(reader);
    BOOST_TEST(!req->parse_all());
    delete req;
    delete reader;
}

BOOST_AUTO_TEST_CASE(duplicate_fields) {
    reader *reader = new test_reader(8);
    request *req = new request(reader);
    BOOST_TEST(req->parse_all());
    BOOST_TEST(req->get_request_field("Coord-Z") == "20");
    delete req;
    delete reader;
}

BOOST_AUTO_TEST_CASE(non_crlf) {
    reader *reader = new test_reader(9);
    request *req = new request(reader);
    BOOST_TEST(!req->parse_all());
    delete req;
    delete reader;
}
