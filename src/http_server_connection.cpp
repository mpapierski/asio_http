#include <asio_http/http_server_connection.hpp>

// Test
#include <iostream>

struct http_request_match
{
	tcp_connection * conn_;
	http_parser * parser_;
	http_request_match(tcp_connection * conn, http_parser * parser)
		: conn_(conn)
		, parser_(parser)
	{
	}
	template <typename Iterator>
	std::pair<Iterator, bool>
	operator()(Iterator begin, Iterator end)
	{
		// Test code (match first whitespace)
		Iterator i = begin;
		while (i != end)
		{
			if (std::isspace(*i++))
				return std::make_pair(i, true);
		}
		return std::make_pair(i, false);
	}
};

namespace boost{
namespace asio {
	template <> struct is_match_condition<http_request_match>
    : public boost::true_type {};
} // namespace asio
}
tcp_connection::tcp_connection(boost::asio::io_service& io_service)
	: socket_(io_service)
	, parser_(boost::make_shared<http_parser>())
{
	// Initialize parser
	http_parser_init(parser_.get(), HTTP_REQUEST);
	parser_->data = this;
	http_parser_settings settings;
	settings.on_url = &tcp_connection::on_url;
}

void tcp_connection::start()
{
	// Start reading
	boost::asio::streambuf sb;
	boost::asio::async_read_until(socket_, sb, http_request_match(this, parser_.get()),
		boost::bind(&tcp_connection::handler,
			shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

int tcp_connection::on_url(http_parser* parser, const char *at, size_t length)
{
	std::cout << "on_url" << std::endl;
	return 1;
}

void tcp_connection::handler(const boost::system::error_code& e, std::size_t size)
{
	std::cout << "handler" << std::endl;
}