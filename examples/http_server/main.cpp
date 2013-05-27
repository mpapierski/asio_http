#include <boost/asio.hpp>
#include <asio_http/http_server.hpp>

using namespace boost::asio::ip;

int
main(int argc, char * argv[])
{
	boost::asio::io_service io_svc;
	http_server server(io_svc, 	tcp::endpoint(tcp::v4(), 8080));
	io_svc.run();
	return 0;
}