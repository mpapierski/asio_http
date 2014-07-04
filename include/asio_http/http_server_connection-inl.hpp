#if !defined(ASIO_HTTP_HTTP_SERVER_CONNECTION_H_INCLUDED_)
#error "Invalid include order"
#endif

template <typename SocketType>
basic_http_connection<SocketType>::basic_http_connection(boost::asio::io_service& io_service)
	: socket_(io_service)
	, parser_()
{
	// Initialize parser
	http_parser_init(&parser_, HTTP_REQUEST);
	parser_.data = this;
	std::memset(&settings_, 0, sizeof(settings_));
	settings_.on_url = &basic_http_connection::on_url;
	settings_.on_message_complete = &basic_http_connection::on_message_complete;
}

template <typename SocketType>
void basic_http_connection<SocketType>::start()
{
	// Read chunk
	auto buf = buffer_.prepare(8);
	socket_.async_read_some(buf,
	  boost::bind(&basic_http_connection<SocketType>::handler,
			this->shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

template <typename SocketType>
int basic_http_connection<SocketType>::on_url(http_parser* parser, const char *at, size_t length)
{
	basic_http_connection * conn = static_cast<basic_http_connection *>(parser->data);
	std::string url(at, at + length);
	std::cout << "url=" << url << std::endl;
	return 0;
}

template <typename SocketType>
int basic_http_connection<SocketType>::on_message_complete(http_parser * parser)
{
	basic_http_connection * conn = static_cast<basic_http_connection *>(parser->data);
	std::cout << "message complete" << std::endl;
	return 1;
}

template <typename SocketType>
void basic_http_connection<SocketType>::handler(const boost::system::error_code& error, std::size_t bytes_transferred)
{
	std::cout << "bytes bytes_transferred = " << bytes_transferred << std::endl;
	
	if (!error && bytes_transferred)
	{
		const char * data = boost::asio::buffer_cast<const char *>(buffer_.data());

		std::size_t nsize = http_parser_execute(&parser_, &settings_, data, bytes_transferred);
		assert(nsize == bytes_transferred);
		if (nsize != bytes_transferred)
		{
			std::cout << "http parser execute fail " << nsize << "/" << bytes_transferred << std::endl;
			socket_.close();
			return;
		}
		buffer_.consume(nsize);
		int is_final = http_body_is_final(&parser_);
		int is_keep_alive = http_should_keep_alive(&parser_);
		std::cout << "consumed " << nsize << " bytes" <<std::endl;
		start();
	}
	else
	{
		socket_.close();
	}
}

template <typename SocketType>
void basic_http_connection<SocketType>::handle_write(const boost::system::error_code& /*error*/,
	size_t /*bytes_transferred*/)
{
	
}

template <typename SocketType>
void basic_http_connection<SocketType>::send_response(std::string message)
{
	message.append("\r\n\r\n");
	boost::asio::async_write(socket_, boost::asio::buffer(message),
		boost::bind(&basic_http_connection<SocketType>::handle_write, this->shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
}