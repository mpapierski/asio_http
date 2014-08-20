CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (tests)

FIND_PACKAGE (Threads)
FIND_PACKAGE (Boost REQUIRED COMPONENTS
	system
	thread)
FIND_PACKAGE (PythonInterp REQUIRED)

INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIRS})
ADD_EXECUTABLE (test_server
	test_server.cpp
	jsoncpp.cpp)
TARGET_LINK_LIBRARIES (test_server
	asio_http
	${Boost_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT})
ADD_TEST (NAME test_server
	COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_server.py $<TARGET_FILE:test_server>)

ADD_EXECUTABLE (test_url_parser
	test_url_parser.cpp)
TARGET_LINK_LIBRARIES (test_url_parser
	asio_http
	${Boost_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT})
ADD_TEST (NAME test_url_parser
	COMMAND test_url_parser)
#ADD_EXECUTABLE (http_parser_test
#	test.c)
#TARGET_LINK_LIBRARIES (http_parser_test
#	asio_http)
#ADD_TEST (http_parser
#	http_parser_test)
