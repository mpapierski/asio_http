#if !defined(ASIO_HTTP_HTTP_CLIENT_INCLUDED_H_)
#error "Invalid include order"
#endif

template <typename Protocol>
boost::asio::io_service::id basic_http_client<Protocol>::id;

template <typename Protocol>
basic_http_client<Protocol>::basic_http_client(boost::asio::io_service & io_service)
	: boost::asio::io_service::service(io_service)
{
}

template <typename Protocol>
void basic_http_client<Protocol>::shutdown_service()
{
}

template <typename Protocol>
template <typename BodyHandler, typename DoneHandler>
boost::shared_ptr<
	http_client_connection<
		Protocol,
		BodyHandler,
		DoneHandler
	>
> basic_http_client<Protocol>::create_request(const std::string & url, BodyHandler body_handler, DoneHandler done_handler)
{
	typedef http_client_connection<Protocol, BodyHandler, DoneHandler> result_type;
	return boost::make_shared<result_type>(
		boost::ref(get_io_service()),
		url,
		body_handler,
		done_handler);
}