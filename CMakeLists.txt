CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (asio_http)

INCLUDE_DIRECTORIES (
	"${CMAKE_CURRENT_SOURCE_DIR}/include")

SET (asio_http_server_SOURCES
	src/http_parser.c)
ADD_LIBRARY (asio_http
	${asio_http_server_SOURCES})

ADD_SUBDIRECTORY (examples)
ENABLE_TESTING ()
ADD_SUBDIRECTORY (tests)
