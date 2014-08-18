#if !defined(ASIO_HTTP_HTTP_SERVER_CONNECTION_H_INCLUDED_)
#error "Invalid include order"
#endif

template <typename SocketType>
basic_http_connection<SocketType>::basic_http_connection(boost::asio::io_service& io_service,
		SocketType * handler)
	: handler_(handler)
	, socket_(io_service)
	, parser_()
	, header_state_(HEADER_START)
{
	// Initialize parser
	http_parser_init(&parser_, HTTP_REQUEST);
	parser_.data = this;
	std::memset(&settings_, 0, sizeof(settings_));
	settings_.on_url = &basic_http_connection::on_url;
	settings_.on_message_begin = &basic_http_connection::on_message_begin;
	settings_.on_header_field = &basic_http_connection::on_header_field;
	settings_.on_header_value = &basic_http_connection::on_header_value;
	settings_.on_headers_complete = &basic_http_connection::on_headers_complete;
	settings_.on_body = &basic_http_connection::on_body;
	settings_.on_message_complete = &basic_http_connection::on_message_complete;
}

template <typename SocketType>
basic_http_connection<SocketType>::~basic_http_connection()
{
	HTTP_SERVER_DEBUG_OUTPUT("~basic_http_connection\n");
}

template <typename SocketType>
void basic_http_connection<SocketType>::start()
{
	socket_.async_read_some(buffer_.prepare(8),
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
	conn->headers_.clear();
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
	conn->request_body_.append(at, at + length);
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
	conn->socket_.get_io_service().post(
		boost::bind(&basic_http_connection::process_request, conn->shared_from_this()));
	return 0;
}

template <typename SocketType>
void basic_http_connection<SocketType>::handler(const boost::system::error_code& error, std::size_t bytes_transferred)
{
	if (!error && bytes_transferred)
	{
		const char * data = boost::asio::buffer_cast<const char *>(buffer_.data());
		std::size_t nsize = http_parser_execute(&parser_, &settings_, data, bytes_transferred);
		if (nsize != bytes_transferred)
		{
			HTTP_SERVER_DEBUG_OUTPUT("http parser execute fail %lu/%lu\n", nsize, bytes_transferred);
			socket_.close();
			return;
		}
		buffer_.consume(nsize);
		start();
	}
	else
	{
		socket_.close();
	}
}

template <typename SocketType>
void basic_http_connection<SocketType>::handle_write(const boost::system::error_code& error,
	size_t bytes_transferred)
{
	if (error)
	{
		HTTP_SERVER_DEBUG_OUTPUT("Unable to handle request: %s [Errno %d]\n",
			error.message().c_str(),
			error.value());
		return;
	}
	HTTP_SERVER_DEBUG_OUTPUT("Response sent with %lu bytes\n", bytes_transferred);
}

template <typename SocketType>
void basic_http_connection<SocketType>::send_response(int status_code, std::string message)
{
	std::string status_text;
#define HTTP_STATUS_CODE(code, descr, type) \
		case (code): \
			status_text = (descr); \
			break;
	switch (status_code)
	{
		HTTP_STATUS_CODE_MAP(HTTP_STATUS_CODE)
		default:
			status_text = "UNKNOWN";
			break;
	}
#undef HTTP_STATUS_CODE

	std::ostream o(&outgoing_buffer_);
	o << "HTTP/1.1 " << status_code << " " << status_text << "\r\n"
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

template <typename SocketType>
void basic_http_connection<SocketType>::process_request()
{
	(*handler_)(this->shared_from_this());
	// Re-initialize parser
	http_parser_init(&parser_, HTTP_REQUEST);
	parser_.data = this;
}
