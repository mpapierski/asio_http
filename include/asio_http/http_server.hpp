#if !defined(ASIO_HTTP_HTTP_SERVER_INCLUDED_H_)
#define ASIO_HTTP_HTTP_SERVER_INCLUDED_H_

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <asio_http/http_server_connection.hpp>

/**
 * HTTP server implementation
 */
class http_server
{
private:
	boost::asio::io_service & io_svc_;
	/**
	 * It waits for sockets
	 */

	boost::asio::ip::tcp::acceptor acceptor_;
public:
	http_server(boost::asio::io_service & io_svc,
				boost::asio::ip::tcp::endpoint endpoint_);
	~http_server();
	/**
	 * Start asynchronous accept.
	 */
	void start_accept();
	/**
	 * New client connected
	 */
	void handle_accept(tcp_connection::pointer new_connection,
					   const boost::system::error_code& error);
};

#endif /* ASIO_HTTP_HTTP_SERVER_INCLUDED_H_ */