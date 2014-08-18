#if 0
#define BOOST_TEST_MODULE example
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#endif
#include <boost/asio.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/array.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <asio_http/http_server.hpp>
#include <asio_http/http_client.hpp>
#include "json/json.h"

using namespace boost::asio::ip;

#define HTTP_404_TEMPLATE "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">\n" \
	"<title>404 Not Found</title>\n" \
	"<h1>Not Found</h1>\n" \
	"<p>The requested URL was not found on the server.  If you entered the URL manually please check your spelling and try again.</p>\n"


#define HTTP_405_TEMPLATE "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">\n" \
	"<title>405 Method Not Allowed</title>\n" \
	"<h1>Method Not Allowed</h1>\n" \
	"<p>The method is not allowed for the requested URL.</p>\n" \

struct http_request_handler
{
	typedef basic_http_connection<http_request_handler> connection;
	Json::Value get_json_data(const connection::pointer & ptr)
	{
		Json::Value result;
		result["url"] = ptr->get_request_url();
		Json::Value headers(Json::objectValue);
		std::ostringstream oss;
		for (connection::headers_type::const_iterator it = ptr->get_headers().begin(), end = ptr->get_headers().end();
			it != end; ++it)
		{
			headers[it->first] = it->second;
		}
		result["headers"] = headers;
		result["origin"] = boost::lexical_cast<std::string>(ptr->get_socket().local_endpoint().address());
		return result;
	}
	void operator()(const connection::pointer & ptr)
	{
		std::cout << "Request URL: " << ptr->get_request_url() << std::endl;
		if (ptr->get_request_url() == "/get")
		{
			if (ptr->get_request_method() != HTTP_GET)
			{
				ptr->send_response(405, "Method Not Allowed");
				return;
			}
			std::cout << "Request handler" << std::endl;
			Json::Value result = get_json_data(ptr);
			ptr->send_response(200, result.toStyledString());
			return;
		}
		if (ptr->get_request_url() == "/post")
		{
			if (ptr->get_request_method() != HTTP_POST)
			{
				ptr->send_response(405, "Method Not Allowed");
				return;
			}
			Json::Value result = get_json_data(ptr);
			result["data"] = ptr->get_request_body();
			ptr->send_response(200, result.toStyledString());
			return;
		}
		ptr->send_response(404, HTTP_404_TEMPLATE);
	}
};

struct F
{
	F()
		: io_service()
		, server(io_service, tcp::endpoint(tcp::v4(), 0))
		, client(boost::asio::use_service<http_client>(io_service))
	{
		std::ostringstream oss;
		oss << "http://127.0.0.1:" << server.get_acceptor().local_endpoint().port();
		base_url = oss.str();
	}
	~F()
	{
	}
	boost::asio::io_service io_service;
	typedef http_server<http_request_handler> server_type;
	server_type server;
	http_client & client;
	std::string base_url;
};

struct client_body_handler
{
	typedef void result_type;
	std::string & str_;
	client_body_handler(std::string & str)
		: str_(str)
	{
	}
	void operator()(const boost::system::error_code & ec, const boost::asio::const_buffer & buffer)
	{
		const char * data = boost::asio::buffer_cast<const char *>(buffer);
		std::size_t size = boost::asio::buffer_size(buffer);
		std::string chunk(data, data + size);
		std::cout << "chunk[" << chunk << "]" << std::endl;
		str_ += chunk;
	}
};

struct client_done_handler
{
	typedef void result_type;
	F::server_type & server_;
	boost::system::error_code & ec_;
	client_done_handler(F::server_type & server, boost::system::error_code & ec)
		: server_(server)
		, ec_(ec)
	{
	}
	void operator()(const boost::system::error_code & ec)
	{
		std::cout << "done handler " << ec.message() << std::endl;
		ec_ = ec;
		server_.stop_accept();
	}
};

typedef	http_client_connection<
	http_client::protocol_type,
	client_body_handler,
	client_done_handler> client_connection;

int
main(int argc, char * argv[])
{
	try
	{
		typedef http_server<http_request_handler> server_type;
		boost::asio::io_service io_service;
		server_type srv(io_service, tcp::endpoint(tcp::v4(), 0));
		Json::Value message;
		message["type"] = "listen";
		message["data"]["port"] = srv.get_acceptor().local_endpoint().address().to_string();
		message["data"]["port"] = srv.get_acceptor().local_endpoint().port();
		std::string json_data = Json::FastWriter().write(message);
		std::fprintf(stdout, "%s", json_data.c_str());
		std::fflush(stdout);
		io_service.run();
	}
	catch (std::exception & e)
	{
		Json::Value message;
		message["type"] = "exception";
		message["data"]["what"] = e.what();
		std::cout << Json::FastWriter().write(message);
		return 1;
	}
}

#if 0

BOOST_FIXTURE_TEST_SUITE( s, F )

BOOST_AUTO_TEST_CASE( test_get1 )
{
	std::string data;
	boost::system::error_code ec;
	client_body_handler body(data);
	client_done_handler done(server, ec);
	client_connection::pointer connection = client.create_request(base_url + "/get", body, done);
	connection->start();
	io_service.run();
	Json::Reader reader;
	Json::Value json_data;
	BOOST_REQUIRE(reader.parse(data, json_data));
	BOOST_REQUIRE_EQUAL("/get", json_data["url"].asString());
	BOOST_REQUIRE_EQUAL("127.0.0.1", json_data["origin"].asString());
	BOOST_REQUIRE_EQUAL(200, connection->get_status_code());
	BOOST_REQUIRE_EQUAL("OK", connection->get_status());
}

struct scoped_thread
{
	boost::thread & thread_;
	scoped_thread(boost::thread & thread)
		: thread_(thread)
	{
	}
	~scoped_thread()
	{
		thread_.join();
	}
};

BOOST_AUTO_TEST_CASE( test_multiple_get_requests )
{
	// Checks if server cleans up properly internal structures
	// after two correct requests were processed.
	std::ostringstream data;
	data << "GET /get HTTP/1.1\r\n";
	data << "Header1: Value1\r\n";
	data << "Host: " << server.get_acceptor().local_endpoint() << "\r\n";
	data << "Accept: */*\r\n";
	data << "Referer: \r\n\r\n";
	
	data << "GET /get HTTP/1.1\r\n";
	data << "Header2: Value2\r\n";
	data << "Host: " << server.get_acceptor().local_endpoint() << "\r\n";
	data << "Accept: */*\r\n";
	data << "Referer: \r\n\r\n";

	boost::thread th(boost::bind(&boost::asio::io_service::run, &io_service));

	boost::asio::ip::tcp::socket client(io_service);
	client.connect(server.get_acceptor().local_endpoint());
	std::size_t bytes_written = boost::asio::write(client, boost::asio::buffer(data.str()));
	std::cout << "sent " << bytes_written << std::endl;
	BOOST_REQUIRE_EQUAL(bytes_written, data.str().size());
	std::vector<Json::Value> values;
	for (int i = 1; i <= 2; ++i)
	{
		boost::asio::streambuf response_buffer;
		std::size_t received_bytes = boost::asio::read_until(client, response_buffer, std::string("}"));
		const char * ptr = boost::asio::buffer_cast<const char *>(response_buffer.data());
		std::string response1(ptr, ptr + received_bytes);
		std::string json_str = response1.substr(response1.find_first_of('{'));
		json_str += "}";
		Json::Reader reader;
		Json::Value value;
		BOOST_REQUIRE(reader.parse(json_str, value));
		values.push_back(value);
	}
	io_service.stop();
	th.join();
	BOOST_REQUIRE(values[0]["headers"].isMember("Header1"));
	BOOST_REQUIRE(!values[0]["headers"].isMember("Header2"));
	BOOST_REQUIRE_EQUAL("Value1", values[0]["headers"]["Header1"].asString());

	BOOST_REQUIRE(values[1]["headers"].isMember("Header2"));
	BOOST_REQUIRE(!values[1]["headers"].isMember("Header1"));
	BOOST_REQUIRE_EQUAL("Value2", values[1]["headers"]["Header2"].asString());
}

BOOST_AUTO_TEST_SUITE_END()

#endif