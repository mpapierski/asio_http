#if !defined(ASIO_HTTP_HTTP_SERVER_CONNECTION_H_INCLUDED_)
#define ASIO_HTTP_HTTP_SERVER_CONNECTION_H_INCLUDED_

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/bind.hpp>
#include "http_parser.h"

class tcp_connection
	: public boost::enable_shared_from_this<tcp_connection>
{
public:
	typedef boost::shared_ptr<tcp_connection> pointer;

	static pointer create(boost::asio::io_service& io_service)
	{
		return pointer(new tcp_connection(io_service));
	}

	boost::asio::ip::tcp::socket& socket()
	{
		return socket_;
	}
	void start();
private:
	tcp_connection(boost::asio::io_service& io_service);
	void handle_write(const boost::system::error_code& /*error*/,
					  size_t /*bytes_transferred*/)
	{
	}
	boost::asio::ip::tcp::socket socket_;
	/*
	 * HTTP stuff
	 */
	boost::shared_ptr<http_parser> parser_;
	static int on_url(http_parser* parser, const char *at, size_t length);
	
	// Test purposes only!
	char buffer_[512 * 1024];
	void handler(const boost::system::error_code& e, std::size_t size);
};

#endif /* ASIO_HTTP_HTTP_SERVER_CONNECTION_H_INCLUDED_ */