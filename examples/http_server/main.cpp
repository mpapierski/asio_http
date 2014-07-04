#include <boost/asio.hpp>
#include <asio_http/http_server.hpp>

using namespace boost::asio::ip;

struct http_request_handler
{
	typedef basic_http_connection<http_request_handler> connection;
	void operator()(connection::pointer ptr)
	{
		std::cout << "Request handler" << std::endl;
		std::ostringstream oss;
		oss << ptr->get_request_url() << '\n';
		for (connection::headers_type::const_iterator it = ptr->get_headers().begin(), end = ptr->get_headers().end();
			it != end; ++it)
		{
			oss << "[" << it->first << "]=[" << it->second << "]" << std::endl;
		}
		ptr->send_response(oss.str());
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