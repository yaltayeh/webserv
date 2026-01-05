#include "HttpHeaders.hpp"
#include "Http.hpp"
#include <sstream>
#include <string>

class Request
{
private:
	HttpMethod	method;
	HttpHeaders	headers;
	std::string body;

protected:
	void setMethod(std::string methodName);
	void setMethod(HttpMethod method);

public:
	Request();
	~Request();
	HttpMethod		getMethod();
	static Request	parseRequest(std::string rawHttp);
	static Request	parseRequest(std::istream &is);
};
