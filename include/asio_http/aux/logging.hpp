#if !defined(ASIO_HTTP_AUX_LOGGING_INCLUDED_H_)
#define ASIO_HTTP_AUX_LOGGING_INCLUDED_H_

/*
 * Internal logging function that just prints data to stdout.
 * TODO: Make this method pluggable into instance.
 */
#if !defined(NDEBUG) || defined(HTTP_SERVER_VERBOSE)
#define HTTP_SERVER_DEBUG_OUTPUT(message, ...)                               \
	do                                                                       \
	{                                                                        \
		std::fprintf(stdout, message, ##__VA_ARGS__);                    \
		std::fflush(stdout);                                                 \
	} while (0)
#else
#define HTTP_SERVER_DEBUG_OUTPUT(...) do {} while (0)
#endif

#endif /* ASIO_HTTP_AUX_LOGGING_INCLUDED_H_ */