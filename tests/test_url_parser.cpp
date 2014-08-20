#define BOOST_TEST_MODULE url_parser
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <asio_http/aux/url_parser.hpp>

BOOST_AUTO_TEST_CASE( test_parse_correct_url )
{
	url_parser url("http://www.github.com");
}

BOOST_AUTO_TEST_CASE( test_parse_and_validate_memory )
{
	BOOST_REQUIRE_EQUAL(url_parser("http://www.github.com").get_schema(), "http");
}

BOOST_AUTO_TEST_CASE( test_parse_incorrect_url )
{
	BOOST_REQUIRE_THROW(url_parser("http:///"), std::runtime_error);
}

struct F
{
	url_parser url;
	F()
		: url("http://www.github.com:666/query?key=foo&value=bar")
	{}
	~F()
	{}
};

BOOST_FIXTURE_TEST_SUITE( s, F )

BOOST_AUTO_TEST_CASE (test_correct_schema)
{
	BOOST_REQUIRE_EQUAL(url.get_schema(), "http");
}

BOOST_AUTO_TEST_CASE (test_correct_host)
{
	BOOST_REQUIRE_EQUAL(url.get_host(), "www.github.com");
}

BOOST_AUTO_TEST_CASE (test_correct_port)
{
	BOOST_REQUIRE_EQUAL(url.get_port(), 666);
}

BOOST_AUTO_TEST_CASE (test_correct_path)
{
	BOOST_REQUIRE_EQUAL(url.get_path(), "/query");
}

BOOST_AUTO_TEST_CASE (test_correct_query)
{
	BOOST_REQUIRE_EQUAL(url.get_query(), "key=foo&value=bar");
}

BOOST_AUTO_TEST_CASE (test_correct_fragment)
{
	BOOST_REQUIRE_EQUAL(url.get_fragment(), "");
}

BOOST_AUTO_TEST_CASE (test_correct_userinfo)
{
	BOOST_REQUIRE_EQUAL(url.get_userinfo(), "");
}

BOOST_AUTO_TEST_SUITE_END()
