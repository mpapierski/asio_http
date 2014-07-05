#if !defined(ASIO_HTTP_HTTP_SERVER_DETAIL_STATUS_CODES_HPP_INCLUDED_)
#define ASIO_HTTP_HTTP_SERVER_DETAIL_STATUS_CODES_HPP_INCLUDED_

// http://wiki.splunk.com/Http_status.csv
#define HTTP_STATUS_CODE_MAP(XX) \
	XX(100, "Continue", "Informational") \
	XX(101, "Switching Protocols", "Informational") \
	XX(200, "OK", "Successful") \
	XX(201, "Created", "Successful") \
	XX(202, "Accepted", "Successful") \
	XX(203, "Non-Authoritative Information", "Successful") \
	XX(204, "No Content", "Successful") \
	XX(205, "Reset Content", "Successful") \
	XX(206, "Partial Content", "Successful") \
	XX(300, "Multiple Choices", "Redirection") \
	XX(301, "Moved Permanently", "Redirection") \
	XX(302, "Found", "Redirection") \
	XX(303, "See Other", "Redirection") \
	XX(304, "Not Modified", "Redirection") \
	XX(305, "Use Proxy", "Redirection") \
	XX(307, "Temporary Redirect", "Redirection") \
	XX(400, "Bad Request", "Client Error") \
	XX(401, "Unauthorized", "Client Error") \
	XX(402, "Payment Required", "Client Error") \
	XX(403, "Forbidden", "Client Error") \
	XX(404, "Not Found", "Client Error") \
	XX(405, "Method Not Allowed", "Client Error") \
	XX(406, "Not Acceptable", "Client Error") \
	XX(407, "Proxy Authentication Required", "Client Error") \
	XX(408, "Request Timeout", "Client Error") \
	XX(409, "Conflict", "Client Error") \
	XX(410, "Gone", "Client Error") \
	XX(411, "Length Required", "Client Error") \
	XX(412, "Precondition Failed", "Client Error") \
	XX(413, "Request Entity Too Large", "Client Error") \
	XX(414, "Request-URI Too Long", "Client Error") \
	XX(415, "Unsupported Media Type", "Client Error") \
	XX(416, "Requested Range Not Satisfiable", "Client Error") \
	XX(417, "Expectation Failed", "Client Error") \
	XX(500, "Internal Server Error", "Server Error") \
	XX(501, "Not Implemented", "Server Error") \
	XX(502, "Bad Gateway", "Server Error") \
	XX(503, "Service Unavailable", "Server Error") \
	XX(504, "Gateway Timeout", "Server Error") \
	XX(505, "HTTP Version Not Supported", "Server Error")

#endif

