#include <string>

#pragma once

namespace Http
{
	enum HttpMethod
	{
#define DEFINE_METHOD(name) name,
#include "HttpMethods.h"
#undef DEFINE_METHOD
	};

	struct MethodMap
	{
		std::string name;
		HttpMethod method;
	};

	MethodMap methodMap[];
	HttpMethod getHttpMethod(std::string methodName);
}
