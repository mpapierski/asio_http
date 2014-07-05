CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (tests)

FIND_PACKAGE (Boost REQUIRED COMPONENTS
	system
	thread)
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIRS})
ADD_EXECUTABLE (test_server
	test_server.cpp
	jsoncpp.cpp)
TARGET_LINK_LIBRARIES (test_server
	asio_http
	${Boost_LIBRARIES})
#ADD_EXECUTABLE (http_parser_test
#	test.c)
#TARGET_LINK_LIBRARIES (http_parser_test
#	asio_http)
#ADD_TEST (http_parser
#	http_parser_test)
