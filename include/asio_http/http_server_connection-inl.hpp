#if !defined(ASIO_HTTP_HTTP_SERVER_CONNECTION_H_INCLUDED_)
#error "Invalid include order"
#endif

template <typename SocketType>
basic_http_connection<SocketType>::basic_http_connection(boost::asio::io_service& io_service)
	: socket_(io_service)
	, parser_()
	, header_state_(HEADER_START)
{
	// Initialize parser
	http_parser_init(&parser_, HTTP_REQUEST);
	parser_.data = this;
	std::memset(&settings_, 0, sizeof(settings_));
	settings_.on_url = &basic_http_connection::on_url;
	settings_.on_message_begin = &basic_http_connection::on_message_begin;
	settings_.on_status_complete = &basic_http_connection::on_status_complete;
	settings_.on_header_field = &basic_http_connection::on_header_field;
	settings_.on_header_value = &basic_http_connection::on_header_value;
	settings_.on_headers_complete = &basic_http_connection::on_headers_complete;
	settings_.on_body = &basic_http_connection::on_body;
	settings_.on_message_complete = &basic_http_connection::on_message_complete;
}

template <typename SocketType>
void basic_http_connection<SocketType>::start()
{
	boost::asio::async_read(socket_, buffer_, boost::asio::transfer_at_least(64),
		boost::bind(&basic_http_connection<SocketType>::handler,
			this->shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

template <typename SocketType>
int basic_http_connection<SocketType>::on_message_begin(http_parser * parser)
{
	basic_http_connection * conn = static_cast<basic_http_connection *>(parser->data);
	conn->request_url_.clear();
	return 0;
}
template <typename SocketType>
int basic_http_connection<SocketType>::on_status_complete(http_parser * parser)
{
	basic_http_connection * conn = static_cast<basic_http_connection *>(parser->data);
	return 0;
}
template <typename SocketType>
int basic_http_connection<SocketType>::on_header_field(http_parser * parser, const char * at, size_t length)
{
	basic_http_connection * conn = static_cast<basic_http_connection *>(parser->data);
	if (conn->header_state_ == HEADER_START)
	{
		conn->header_field_.append(at, at + length);
	}
	else if (conn->header_state_ == HEADER_VALUE)
	{
		conn->headers_.insert(std::make_pair(conn->header_field_, conn->header_value_));
		conn->header_field_.clear();
		conn->header_value_.clear();
		conn->header_field_.append(at, at + length);
	}
	else if (conn->header_state_ == HEADER_FIELD)
	{
		conn->header_field_.append(at, at + length);
	}
	conn->header_state_ = HEADER_FIELD;
	return 0;
}
template <typename SocketType>
int basic_http_connection<SocketType>::on_header_value(http_parser * parser, const char * at, size_t length)
{
	basic_http_connection * conn = static_cast<basic_http_connection *>(parser->data);
	if (conn->header_state_ == HEADER_FIELD)
	{
		conn->header_value_.clear();
		conn->header_value_.append(at, at + length);
	}
	else if (conn->header_state_ == HEADER_VALUE)
	{
		conn->header_value_.append(at, at + length);
	}
	conn->header_state_ = HEADER_VALUE;
	return 0;
}
template <typename SocketType>
int basic_http_connection<SocketType>::on_headers_complete(http_parser * parser)
{
	basic_http_connection * conn = static_cast<basic_http_connection *>(parser->data);
	if (conn->header_state_ == HEADER_VALUE)
	{
		conn->headers_.insert(std::make_pair(conn->header_field_, conn->header_value_));
	}
	return 0;
}
template <typename SocketType>
int basic_http_connection<SocketType>::on_body(http_parser * parser, const char * at, size_t length)
{
	basic_http_connection * conn = static_cast<basic_http_connection *>(parser->data);
	return 0;
}

template <typename SocketType>
int basic_http_connection<SocketType>::on_url(http_parser* parser, const char *at, size_t length)
{
	basic_http_connection * conn = static_cast<basic_http_connection *>(parser->data);
	conn->request_url_.append(at, at + length);
	return 0;
}

template <typename SocketType>
int basic_http_connection<SocketType>::on_message_complete(http_parser * parser)
{
	basic_http_connection * conn = static_cast<basic_http_connection *>(parser->data);
	SocketType()(conn->shared_from_this());
	return 0;
}

template <typename SocketType>
void basic_http_connection<SocketType>::handler(const boost::system::error_code& error, std::size_t bytes_transferred)
{
	if (!error && bytes_transferred)
	{
		const char * data = boost::asio::buffer_cast<const char *>(buffer_.data());
		std::size_t nsize = http_parser_execute(&parser_, &settings_, data, bytes_transferred);
		// std::cout << "nsize = " << nsize << std::endl;
		if (nsize != bytes_transferred)
		{
			// std::cout << "http parser execute fail " << nsize << "/" << bytes_transferred << std::endl;
			socket_.close();
			return;
		}
		buffer_.consume(nsize);
	}
	else
	{
		socket_.close();
	}
}

template <typename SocketType>
void basic_http_connection<SocketType>::handle_write(const boost::system::error_code& error,
	size_t /*bytes_transferred*/)
{
	if (error)
	{
		// std::cerr << "Unable to handle request: " << error.message() << " [Errno " << error.value() << "]" << std::endl;
		return;
	}
	// std::cout << "Response sent" << std::endl;
	start();
}

template <typename SocketType>
void basic_http_connection<SocketType>::send_response(std::string message)
{
	std::ostream o(&outgoing_buffer_);
	o << "HTTP/1.1 200 OK\r\n"
		<< "Content-Length: " << message.length() << "\r\n";
	if (http_should_keep_alive(&parser_) == 1)
	{
		o << "Connection: keep-alive\r\n";
	}
	o << "\r\n";
	o << message;
	boost::asio::async_write(socket_, outgoing_buffer_,
		boost::bind(&basic_http_connection<SocketType>::handle_write, this->shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
}