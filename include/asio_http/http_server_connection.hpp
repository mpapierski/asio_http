#if !defined(ASIO_HTTP_HTTP_SERVER_CONNECTION_H_INCLUDED_)
#define ASIO_HTTP_HTTP_SERVER_CONNECTION_H_INCLUDED_

#include <string>
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
	http_parser_settings settings_;
private:
	tcp_connection(boost::asio::io_service& io_service);
	void handle_write(const boost::system::error_code& /*error*/,
					  size_t /*bytes_transferred*/);
	boost::asio::ip::tcp::socket socket_;
	/*
	 * HTTP stuff
	 */
	boost::shared_ptr<http_parser> parser_;
	/**
	 * HTTP parser encountered something that appears to be URL.
	 * This callback might be called multiple times, but in our
	 * case this gets called just once.
	 */
	static int on_url(http_parser* parser, const char *at, size_t length);
	/**
	 * Temporary socket data is stored here.
	 */
	boost::asio::streambuf buffer_;
	/**
	 * Received HTTP header.
	 */
	void handler(const boost::system::error_code& e, std::size_t size);
	/**
	 * Send HTTP response.
	 */
	void send_response(std::string message);
};

#endif /* ASIO_HTTP_HTTP_SERVER_CONNECTION_H_INCLUDED_ */