#include <string>

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

class Http
{
private:
public:
	static MethodMap methodMap[];
	static HttpMethod getHttpMethod(std::string methodName);
};
