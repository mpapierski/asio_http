#define BOOST_TEST_MODULE example
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/scoped_ptr.hpp>
#include <asio_http/http_server.hpp>
#include "json/json.h"

using namespace boost::asio::ip;

#define HTTP_404_TEMPLATE "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">\n" \
	"<title>404 Not Found</title>\n" \
	"<h1>Not Found</h1>\n" \
	"<p>The requested URL was not found on the server.  If you entered the URL manually please check your spelling and try again.</p>\n"

struct http_request_handler
{
	typedef basic_http_connection<http_request_handler> connection;
	void operator()(connection::pointer ptr)
	{
		if (ptr->get_request_url() == "/get")
		{
			std::cout << "Request handler" << std::endl;
			Json::Value result;
			result["url"] = ptr->get_request_url();
			Json::Value headers;
			std::ostringstream oss;
			for (connection::headers_type::const_iterator it = ptr->get_headers().begin(), end = ptr->get_headers().end();
				it != end; ++it)
			{
				headers[it->first] = it->second;
			}
			result["headers"] = headers;
			ptr->send_response(200, result.toStyledString());
			return;
		}
		ptr->send_response(404, HTTP_404_TEMPLATE);
	}
};

struct F
{
	F()
		: io_service()
		, server(io_service, tcp::endpoint(tcp::v4(), 0))
	{
	}
	~F()
	{
	}
	boost::asio::io_service io_service;
	typedef http_server<http_request_handler> server_type;
	server_type server;
};

BOOST_FIXTURE_TEST_SUITE( s, F )

BOOST_AUTO_TEST_CASE( test_get1 )
{
	io_service.run();
}

BOOST_AUTO_TEST_SUITE_END()
