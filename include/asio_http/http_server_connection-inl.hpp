#if !defined(ASIO_HTTP_HTTP_SERVER_CONNECTION_H_INCLUDED_)
#error "Invalid include order"
#endif

template <typename SocketType>
basic_http_connection<SocketType>::basic_http_connection(boost::asio::io_service& io_service)
	: socket_(io_service)
	, parser_(boost::make_shared<http_parser>())
{
	// Initialize parser
	http_parser_init(parser_.get(), HTTP_REQUEST);
	parser_->data = this;
	std::memset(&settings_, 0, sizeof(settings_));
	settings_.on_url = &basic_http_connection::on_url;
}

template <typename SocketType>
void basic_http_connection<SocketType>::start()
{
	// Read whole HTTP request
	boost::asio::async_read_until(socket_, buffer_, "\r\n\r\n",
	  boost::bind(&basic_http_connection<SocketType>::handler,
			this->shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

template <typename SocketType>
int basic_http_connection<SocketType>::on_url(http_parser* parser, const char *at, size_t length)
{
	basic_http_connection * conn = static_cast<basic_http_connection *>(parser->data);
	//std::vector<char> url(at, at + length + 1);
	//url[length] = '\0';
	//std::cout << "URL: " << url.data() << std::endl;
	return 0;
}

template <typename SocketType>
void basic_http_connection<SocketType>::handler(const boost::system::error_code& error, std::size_t bytes_transferred)
{
	if (!error && bytes_transferred && this->shared_from_this())
	{
		std::istream is(&buffer_);
		std::vector<char> vec(bytes_transferred, '\0');
		is.read(vec.data(), vec.size());
		std::size_t nsize = http_parser_execute(parser_.get(), &settings_, vec.data(), bytes_transferred);
		if (nsize != bytes_transferred)
		{
			std::cout << "http parser execute fail " << nsize << "/" << bytes_transferred << std::endl;
			socket_.close();
			return;
		}
		// Not sure if http_parser should be told about eof.
		// There is no \r\n\r\n in the stream (or is it?)
		std::size_t eof = http_parser_execute(parser_.get(), &settings_, vec.data(), 0);
		assert(eof == 0 && "Unable to reach EOF");
		this->send_response("hello world");
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