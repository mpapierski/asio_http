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
	void start();
	void resolve_handler(const boost::system::error_code& ec,
    	boost::asio::ip::tcp::resolver::iterator i);
	struct connect_condition
	{
		template <typename Iterator>
		Iterator operator()(
			const boost::system::error_code& ec,
			Iterator next)
		{
			if (ec) std::cout << "Error: " << ec.message() << std::endl;
			std::cout << "Trying: " << next->endpoint() << std::endl;
			return next;
		}
	}; 
	void connect_handler(const boost::system::error_code& ec,
    	boost::asio::ip::tcp::resolver::iterator i);
	boost::asio::io_service & io_service_;
	boost::asio::ip::tcp::resolver resolver_;
	T socket_;
	std::string url_;
	http_parser_url parsed_url_;
	BodyHandler body_handler_;
	DoneHandler done_handler_;
};

#include "http_client_connection-inl.hpp"

#endif