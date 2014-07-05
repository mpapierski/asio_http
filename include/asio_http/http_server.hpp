#if !defined(ASIO_HTTP_HTTP_SERVER_INCLUDED_H_)
#define ASIO_HTTP_HTTP_SERVER_INCLUDED_H_

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <asio_http/http_server_connection.hpp>

/**
 * HTTP server implementation
 */
template <typename RequestHandler>
class http_server
{
private:
	boost::asio::io_service & io_svc_;
	/**
	 * It waits for sockets
	 */

	boost::asio::ip::tcp::acceptor acceptor_;
	RequestHandler request_handler_;
public:
	typedef basic_http_connection<RequestHandler> connection_type;
	http_server(boost::asio::io_service & io_svc,
				boost::asio::ip::tcp::endpoint endpoint_);
	/**
	 * Start asynchronous accept.
	 */
	void start_accept();
	/**
	 * New client connected
	 */
	void handle_accept(typename connection_type::pointer new_connection,
					   const boost::system::error_code& error);
	void handle_request(typename connection_type::pointer connection);
	inline boost::asio::ip::tcp::acceptor & get_acceptor()
	{
		return acceptor_;
	}
};

#include "http_server-inl.hpp"

#endif /* ASIO_HTTP_HTTP_SERVER_INCLUDED_H_ */