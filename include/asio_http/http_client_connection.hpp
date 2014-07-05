#if !defined(ASIO_HTTP_HTTP_CLIENT_CONNECTION_INCLUDED_H_)
#define ASIO_HTTP_HTTP_CLIENT_CONNECTION_INCLUDED_H_

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>

template <typename T, typename BodyHandler, typename DoneHandler>
struct http_client_connection
	: boost::enable_shared_from_this<
		http_client_connection<
			T,
			BodyHandler,
			DoneHandler
		>
	>
{
	typedef boost::shared_ptr<http_client_connection> pointer;
	http_client_connection(boost::asio::io_service & io_service,
		std::string url,
		BodyHandler body_handler, DoneHandler done_handler);
	~http_client_connection();
	void start();
	void resolve_handler(const boost::system::error_code& ec,
		boost::asio::ip::tcp::resolver::iterator i);
	void connect_handler(const boost::system::error_code& ec,
		boost::asio::ip::tcp::resolver::iterator i);
	void start_read();
	void write_handler(const boost::system::error_code& error,
		std::size_t bytes_transferred);
	void read_handler(const boost::system::error_code& error,
		std::size_t bytes_transferred);
	static int on_body(http_parser * parser, const char * at, size_t length);
	static int on_message_complete(http_parser * parser);
	boost::asio::io_service & io_service_;
	boost::asio::ip::tcp::resolver resolver_;
	T socket_;
	std::string url_;
	http_parser_url parsed_url_;
	BodyHandler body_handler_;
	DoneHandler done_handler_;
	boost::asio::streambuf request_buffer_;
	boost::asio::streambuf response_buffer_;
	http_parser_settings settings_;
	http_parser parser_;
};

#include "http_client_connection-inl.hpp"

#endif