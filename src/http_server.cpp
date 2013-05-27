#include <asio_http/http_server.hpp>

http_server::http_server(boost::asio::io_service & io_svc,
						 boost::asio::ip::tcp::endpoint endpoint_)
	: io_svc_(io_svc)
	, acceptor_(io_svc_, endpoint_)
{
	start_accept();
}

void http_server::start_accept()
{
    tcp_connection::pointer new_connection =
		tcp_connection::create(io_svc_);
	acceptor_.async_accept(new_connection->socket(),
		boost::bind(&http_server::handle_accept, this, new_connection,
		boost::asio::placeholders::error));
}

void http_server::handle_accept(tcp_connection::pointer new_connection,
								const boost::system::error_code& error)
{
    if (!error)
    {
		new_connection->start();
		start_accept();
    }
}