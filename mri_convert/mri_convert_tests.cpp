//
// Created by Ahmed Abou-Aliaa on 29.11.19.
//

#define BOOST_TEST_MODULE MRI_CONVERT_TESTS
#include <boost/test/included/unit_test.hpp>
#include "mri_convert_lib.hpp"

using namespace boost::unit_test::framework;

BOOST_AUTO_TEST_CASE(first_test) { BOOST_TEST(returnAnInt(24) == 24); }

BOOST_AUTO_TEST_CASE(second_test) {
  FILE *stream{};
  BOOST_TEST(usage_message_lib(stream) == false);
}
