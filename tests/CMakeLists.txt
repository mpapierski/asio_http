CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (tests)

ADD_EXECUTABLE (http_parser_test
	test.c)
TARGET_LINK_LIBRARIES (http_parser_test
	asio_http)
ADD_TEST (http_parser
	http_parser_test)