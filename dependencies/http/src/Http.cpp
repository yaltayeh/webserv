#include "Http.hpp"
#include <string>

namespace Http
{
	MethodMap Http::methodMap[3] = {
#define DEFINE_METHOD(name) {#name, name},
#include "HttpMethods.h"
#undef DEFINE_METHOD
	};

	HttpMethod Http::getHttpMethod(std::string methodName)
	{
		for (unsigned int i = 0; i < sizeof(Http::methodMap) / sizeof(Http::methodMap[0]); i++)
		{
			if (methodMap[i].name == methodName)
				return (methodMap[i].method);
		}
		// throw exe
		return (HttpMethod)-1;
	}
}