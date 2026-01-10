#ifndef AUTOINDEXFORMATDIRECTIVE_HPP
#define AUTOINDEXFORMATDIRECTIVE_HPP

#include "IDirective.hpp"
#include <string>

/*
static ngx_conf_enum_t  ngx_http_autoindex_format[] = {
	{ ngx_string("html"), NGX_HTTP_AUTOINDEX_HTML },
	{ ngx_string("json"), NGX_HTTP_AUTOINDEX_JSON },
	{ ngx_string("jsonp"), NGX_HTTP_AUTOINDEX_JSONP },
	{ ngx_string("xml"), NGX_HTTP_AUTOINDEX_XML },
	{ ngx_null_string, 0 }
};
*/
namespace Config
{
	enum auto_index_format
	{
		HTTP_AUTOINDEX_HTML,
		HTTP_AUTOINDEX_JSON,
		HTTP_AUTOINDEX_JSONP,
		HTTP_AUTOINDEX_XML,
	};
	class AutoindexFormatDirective : public IDirective
	{
	public:
		enum auto_index_format format;

	public:
		AutoindexFormatDirective();
		AutoindexFormatDirective(enum auto_index_format format);
		AutoindexFormatDirective(const AutoindexFormatDirective &other);
		AutoindexFormatDirective &operator=(const AutoindexFormatDirective &other);
		~AutoindexFormatDirective();

		static AutoindexFormatDirective *initAutoindexFormatDirective(const std::vector<std::string> &args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
