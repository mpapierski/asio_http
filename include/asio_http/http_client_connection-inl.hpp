#if !defined(ASIO_HTTP_HTTP_CLIENT_INCLUDED_H_)
#error "Invalid include order"
#endif

template <typename Protocol, typename BodyHandler, typename DoneHandler>
http_client_connection<Protocol, BodyHandler, DoneHandler>::http_client_connection(boost::asio::io_service & io_service,
		std::string url,
		BodyHandler body_handler, DoneHandler done_handler)
	: io_service_(io_service)
	, resolver_(io_service_)
	, socket_(io_service_)
	, url_(url)
	, parsed_url_()
	, body_handler_(body_handler)
	, done_handler_(done_handler)
{
	std::cout << url_ << std::endl;
	int result = http_parser_parse_url(url_.c_str(), url_.size(), 0, &parsed_url_);
	assert(result == 0);
}


template <typename Protocol, typename BodyHandler, typename DoneHandler>
void http_client_connection<Protocol, BodyHandler, DoneHandler>::start()
{
	std::string port = parsed_url_.port
		? url_.substr(
				parsed_url_.field_data[UF_PORT].off,
				parsed_url_.field_data[UF_PORT].len)
		: "80";
	std::string addr = url_.substr(
		parsed_url_.field_data[UF_HOST].off,
		parsed_url_.field_data[UF_HOST].len);
	boost::asio::ip::tcp::resolver::query q(addr, port);
	resolver_.async_resolve(q, 
		boost::bind(&http_client_connection::resolve_handler, this->shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::iterator));
}

template <typename Protocol, typename BodyHandler, typename DoneHandler>
void http_client_connection<Protocol, BodyHandler, DoneHandler>::resolve_handler(const boost::system::error_code& ec,
	boost::asio::ip::tcp::resolver::iterator i)
{
	if (!ec)
	{
		boost::asio::async_connect(socket_, i,
			connect_condition(),
			boost::bind(&http_client_connection::connect_handler, this->shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::iterator));
	}
}

template <typename Protocol, typename BodyHandler, typename DoneHandler>
void http_client_connection<Protocol, BodyHandler, DoneHandler>::connect_handler(
	const boost::system::error_code& ec,
	boost::asio::ip::tcp::resolver::iterator i)
{
	if (ec)
	{
		// An error occurred.
		io_service_.post(boost::bind(done_handler_, ec));
		return;
	}
	else
	{
		std::cout << "Connected to: " << i->endpoint() << std::endl;
	}
}
