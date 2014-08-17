#include <iostream>
#include <boost/asio.hpp>
#include <asio_http/http_server.hpp>

using namespace boost::asio::ip;

struct http_request_handler
{
	typedef basic_http_connection<http_request_handler> connection;
	void operator()(const connection::pointer & ptr)
	{
		ptr->send_response(200, "Hello world!");
	}
};

int
main(int argc, char * argv[])
{
	boost::asio::io_service io_svc;
	http_server<http_request_handler> server(io_svc, tcp::endpoint(tcp::v4(), 5000));
	io_svc.run();
	return 0;
}