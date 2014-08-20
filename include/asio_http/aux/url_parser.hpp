#if !defined(ASIO_HTTP_AUX_HTTP_PARSER_INCLUDED_H_)
#define ASIO_HTTP_AUX_HTTP_PARSER_INCLUDED_H_

#include <string>
#include "http_parser.h"

struct url_parser
{
	http_parser_url url_;
	std::string input_;
	bool is_connect_;
	url_parser(const std::string & input, bool is_connect = false)
		: url_()
		, input_(input)
		, is_connect_(is_connect)
	{
		if (::http_parser_parse_url(input_.c_str(), input_.size(), is_connect_, &url_) != 0)
		{
			throw std::runtime_error("Unable to parser url");
		}
	}
	std::string get_schema() const
	{
		assert(url_.field_set & (1 << UF_SCHEMA));
		uint16_t off = url_.field_data[UF_SCHEMA].off, len = url_.field_data[UF_SCHEMA].len;
		return input_.substr(off, len);
	}
	std::string get_host() const
	{
		assert(url_.field_set & (1 << UF_HOST));
		uint16_t off = url_.field_data[UF_HOST].off, len = url_.field_data[UF_HOST].len;
		return input_.substr(off, len);
	}
	int get_port() const
	{
		assert(url_.field_set & (1 << UF_PORT));
		return url_.port;
	}
	std::string get_path() const
	{
		assert(url_.field_set & (1 << UF_PATH));
		uint16_t off = url_.field_data[UF_PATH].off, len = url_.field_data[UF_PATH].len;
		return input_.substr(off, len);
	}
	std::string get_query() const
	{
		assert(url_.field_set & (1 << UF_QUERY));
		uint16_t off = url_.field_data[UF_QUERY].off, len = url_.field_data[UF_QUERY].len;
		return input_.substr(off, len);
	}
	std::string get_fragment() const
	{
		assert(url_.field_set & (1 << UF_FRAGMENT));
		uint16_t off = url_.field_data[UF_FRAGMENT].off, len = url_.field_data[UF_FRAGMENT].len;
		return input_.substr(off, len);
	}
	std::string get_userinfo() const
	{
		assert(url_.field_set & (1 << UF_USERINFO));
		uint16_t off = url_.field_data[UF_USERINFO].off, len = url_.field_data[UF_USERINFO].len;
		return input_.substr(off, len);
	}
};

#endif /* ASIO_HTTP_AUX_HTTP_PARSER_INCLUDED_H_ */