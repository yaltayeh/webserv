#include "Http.hpp"
#include <string>

MethodMap Http::methodMap[] = {
#define DEFINE_METHOD(name) {#name, name},
#include "HttpMethods.h"
#undef DEFINE_METHOD
};

HttpMethod Http::getHttpMethod(std::string methodName)
{
	for (unsigned int i = 0; i < sizeof(methodMap) / sizeof(methodMap[0]); i++)
	{
		if (methodMap[i].name == methodName)
			return (methodMap[i].method);
	}
	// throw exe
	return (HttpMethod)-1; 
}
