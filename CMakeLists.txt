CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (asio_http)

FIND_PACKAGE (Boost REQUIRED)
INCLUDE_DIRECTORIES (
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
	${Boost_INCLUDE_DIRS})

SET (asio_http_server_SOURCES
	src/http_parser.c
	src/http_server.cpp
	src/http_server_connection.cpp)
ADD_LIBRARY (asio_http
	${asio_http_server_SOURCES})

ENABLE_TESTING ()
ADD_SUBDIRECTORY (tests)
