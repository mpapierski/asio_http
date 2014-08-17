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
	, settings_()
	, parser_()
{
	std::cout << url_ << std::endl;
	int result = http_parser_parse_url(url_.c_str(), url_.size(), 0, &parsed_url_);
	assert(result == 0);
	http_parser_init(&parser_, HTTP_RESPONSE);
	parser_.data = this;
	settings_.on_body = &http_client_connection::on_body;
	settings_.on_message_complete = &http_client_connection::on_message_complete;
	settings_.on_status = &http_client_connection::on_status;
}

template <typename Protocol, typename BodyHandler, typename DoneHandler>
http_client_connection<Protocol, BodyHandler, DoneHandler>::~http_client_connection()
{
	HTTP_SERVER_DEBUG_OUTPUT("~http_client_connection\n");
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
	if (ec)
	{
		io_service_.post(boost::bind(done_handler_, ec));
		return;
	}
	assert(i != boost::asio::ip::tcp::resolver::iterator());
	boost::asio::ip::tcp::endpoint ep = *i;
	socket_.async_connect(ep,
		boost::bind(&http_client_connection::connect_handler, this->shared_from_this(),
			boost::asio::placeholders::error,
			++i));
}

template <typename Protocol, typename BodyHandler, typename DoneHandler>
void http_client_connection<Protocol, BodyHandler, DoneHandler>::connect_handler(
	const boost::system::error_code& ec,
	boost::asio::ip::tcp::resolver::iterator i)
{
	if (!ec)
	{
		// An error occurred.
		HTTP_SERVER_DEBUG_OUTPUT("connected to %s:%d", socket_.local_endpoint().address().to_string().c_str(), socket_.local_endpoint().port());
		std::string path = url_.substr(
			parsed_url_.field_data[UF_PATH].off,
			url_.size());
		std::ostream o(&request_buffer_);
		o << "GET " << path << " HTTP/1.1\r\n\r\n";
		boost::asio::async_write(socket_, request_buffer_,
			boost::bind(&http_client_connection::write_handler, this->shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		return;
	}
	else if (i != boost::asio::ip::tcp::resolver::iterator())
	{
		HTTP_SERVER_DEBUG_OUTPUT("Trying to connect to %s\n", i->endpoint().address().to_string().c_str());
		socket_.close();
		boost::asio::ip::tcp::endpoint ep = *i;
		socket_.async_connect(ep,
			boost::bind(&http_client_connection::connect_handler, this->shared_from_this(),
				boost::asio::placeholders::error,
				++i));
	}
	else
	{
		io_service_.post(boost::bind(done_handler_, ec));
	}
}

template <typename Protocol, typename BodyHandler, typename DoneHandler>
void http_client_connection<Protocol, BodyHandler, DoneHandler>::start_read()
{
	HTTP_SERVER_DEBUG_OUTPUT("Reading...\n");
	socket_.async_read_some(response_buffer_.prepare(1024),
		boost::bind(&http_client_connection::read_handler,
			this->shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

template <typename Protocol, typename BodyHandler, typename DoneHandler>
void http_client_connection<Protocol, BodyHandler, DoneHandler>::write_handler(
	const boost::system::error_code& error,
	std::size_t bytes_transferred)
{
	HTTP_SERVER_DEBUG_OUTPUT("Write %lu: %s\n",
		bytes_transferred,
		error.message().c_str());
	if (error)
	{
		io_service_.post(boost::bind(done_handler_, error));
		return;
	}
	request_buffer_.consume(bytes_transferred);
	start_read();
}

template <typename Protocol, typename BodyHandler, typename DoneHandler>
void http_client_connection<Protocol, BodyHandler, DoneHandler>::read_handler(
	const boost::system::error_code& error,
	std::size_t bytes_transferred)
{
	HTTP_SERVER_DEBUG_OUTPUT("Received data %s [%lu bytes]\n", error.message().c_str(), bytes_transferred);
	if (!error && bytes_transferred)
	{
		const char * data = boost::asio::buffer_cast<const char *>(response_buffer_.data());
		std::size_t nsize = http_parser_execute(&parser_, &settings_, data, bytes_transferred);
		// std::cout << "nsize = " << nsize << std::endl;
		if (nsize != bytes_transferred)
		{
			HTTP_SERVER_DEBUG_OUTPUT("http parser execute fail %lu/%lu", nsize, bytes_transferred);
			socket_.close();
			return;
		}
		response_buffer_.consume(nsize);
		start_read();
	}
	else
	{
		socket_.close();
	}
}

template <typename Protocol, typename BodyHandler, typename DoneHandler>
int http_client_connection<Protocol, BodyHandler, DoneHandler>::on_body(http_parser * parser, const char * at, size_t length)
{
	assert(parser->data);
	http_client_connection * obj = static_cast<http_client_connection *>(parser->data);
	obj->body_handler_(boost::system::error_code(), boost::asio::const_buffer(at, length));
	return 0;
}

template <typename Protocol, typename BodyHandler, typename DoneHandler>
int http_client_connection<Protocol, BodyHandler, DoneHandler>::on_message_complete(http_parser * parser)
{
	assert(parser->data);
	http_client_connection * obj = static_cast<http_client_connection *>(parser->data);
	obj->io_service_.post(boost::bind(obj->done_handler_, boost::system::error_code()));
	obj->socket_.close();
	return 0;
}

template <typename Protocol, typename BodyHandler, typename DoneHandler>
int http_client_connection<Protocol, BodyHandler, DoneHandler>::on_status(http_parser * parser, const char * at, size_t length)
{
	assert(parser->data);
	http_client_connection * obj = static_cast<http_client_connection *>(parser->data);
	obj->status_.append(at, length);
	return 0;
}
