#if !defined(ASIO_HTTP_HTTP_SERVER_INCLUDED_H_)
#error "Invalid include order"
#endif

template <typename RequestHandler>
http_server<RequestHandler>::http_server(boost::asio::io_service & io_svc,
		boost::asio::ip::tcp::endpoint endpoint_,
		RequestHandler handler)
	: io_svc_(io_svc)
	, acceptor_(io_svc_, endpoint_)
	, request_handler_(handler)
{
	start_accept();
}

template <typename RequestHandler>
void http_server<RequestHandler>::start_accept()
{
    typename connection_type::pointer new_connection =
		connection_type::create(io_svc_, &request_handler_);
	acceptor_.async_accept(new_connection->get_socket(),
		boost::bind(&http_server<RequestHandler>::handle_accept, this, new_connection,
		boost::asio::placeholders::error));
}

template <typename RequestHandler>
void http_server<RequestHandler>::stop_accept()
{
	acceptor_.cancel();
}

template <typename RequestHandler>
void http_server<RequestHandler>::handle_accept(typename connection_type::pointer new_connection,
	const boost::system::error_code& error)
{
	std::cout << "new server connection: " << error.message() << std::endl;
    if (!error)
    {
		new_connection->start();
		start_accept();
    }
}

template <typename RequestHandler>
void http_server<RequestHandler>::handle_request(typename connection_type::pointer connection)
{

}