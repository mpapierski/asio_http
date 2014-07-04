#if !defined(ASIO_HTTP_HTTP_SERVER_CONNECTION_H_INCLUDED_)
#define ASIO_HTTP_HTTP_SERVER_CONNECTION_H_INCLUDED_

#include <string>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/bind.hpp>
#include "http_parser.h"

template <typename SocketType>
class basic_http_connection
	: public boost::enable_shared_from_this<basic_http_connection<SocketType> >
{
public:
	typedef boost::shared_ptr<basic_http_connection<SocketType> > pointer;

	static pointer create(boost::asio::io_service& io_service)
	{
		return pointer(new basic_http_connection(io_service));
	}

	boost::asio::ip::tcp::socket& socket()
	{
		return socket_;
	}
	void start();
	http_parser_settings settings_;
private:
	basic_http_connection(boost::asio::io_service& io_service);
	void handle_write(const boost::system::error_code& /*error*/,
		size_t /*bytes_transferred*/);
	boost::asio::ip::tcp::socket socket_;
	/*
	 * HTTP stuff
	 */
	http_parser parser_;
	/**
	 * HTTP parser encountered something that appears to be URL.
	 * This callback might be called multiple times, but in our
	 * case this gets called just once.
	 */
	static int on_url(http_parser* parser, const char *at, size_t length);
	/**
	 * Received complete HTTP request
	 */
	static int on_message_complete(http_parser * parser);
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

#include "http_server_connection-inl.hpp"

typedef basic_http_connection<boost::asio::ip::tcp::socket> http_connection;

#endif /* ASIO_HTTP_HTTP_SERVER_CONNECTION_H_INCLUDED_ */