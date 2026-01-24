#include "ResponseBlender.hpp"
#include "DefaultResponse.hpp"
#include "Constants.hpp"
#include "Utils.hpp"
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>

namespace libhttp {

	ResponseBlender::ResponseBlender()
		: _statusCode(200), _headers(), _body() {}

	ResponseBlender::ResponseBlender(int statusCode) { setDefaultPage(statusCode); }

	ResponseBlender::~ResponseBlender() {}

	// Setters
	void ResponseBlender::setStatus(int statusCode) {
		_statusCode = statusCode;
	}
	void ResponseBlender::setHeader(const std::string& key, const std::string& value) {
		_headers.insert(std::make_pair(key, value));
	}
	void ResponseBlender::setBody(const std::string& body) {
		_body = body;
	}

	// Getters
	int ResponseBlender::getStatus() const { return _statusCode; }
	std::multimap<std::string, std::string> ResponseBlender::getHeaders() const { return _headers; }
	std::string ResponseBlender::getBody() const { return _body; }

	std::string ResponseBlender::toString() const {
		std::string response;
		response += HTTP_1_0;
		std::stringstream ss;
		ss << _statusCode;
		response += " " + ss.str() + " " + getStatusMessage(_statusCode) + "\r\n";
		for (std::multimap<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
			response += it->first + ": " + it->second + "\r\n";
		}
		response += "\r\n";
		response += _body;
		return response;
	}

	/**
	 * setDefaultPage() - Creates default HTML error page
	 *
	 * Used when no custom error_page is configured.
	 */
	void ResponseBlender::setDefaultPage(int status_code)
	{
		
		std::string message = getStatusMessage(status_code);

		std::ostringstream html;
		html << "<!DOCTYPE html>\n"
			 << "<html>\n"
			 << "<head>\n"
			 << "  <title>" << status_code << " " << message << "</title>\n"
			 << "</head>\n"
			 << "<body>\n"
			 << "  <h1>" << status_code << " " << message << "</h1>\n"
			 << "  <hr>\n"
			 << "  <p>webserv/1.0</p>\n"
			 << "</body>\n"
			 << "</html>\n";

		std::string body = html.str();

		setStatus(status_code);
		setHeader("Content-Type", "text/html");

		std::ostringstream ss;
		ss << body.size();
		setHeader("Content-Length", ss.str());

		setBody(body);
	}

	/**
	 * setAutoindexPage() - Generates directory listing
	 *
	 * Lists files and subdirectories in HTML format when:
	 * - No index file found
	 * - autoindex is enabled
	 */
	void ResponseBlender::setAutoindexPage(const std::string& directory_path,
								const std::string& uri)
	{
		// Open directory
		DIR* dir = opendir(directory_path.c_str());
		if (dir == NULL)
			return setDefaultPage(500);

		// Build HTML directory listing
		std::ostringstream html;
		html << "<!DOCTYPE html>\n"
			 << "<html>\n"
			 << "<head>\n"
			 << "  <title>Index of " << uri << "</title>\n"
			 << "  <style>\n"
			 << "    body { font-family: monospace; }\n"
			 << "    table { border-collapse: collapse; }\n"
			 << "    th, td { text-align: left; padding: 8px; }\n"
			 << "    tr:hover { background-color: #f5f5f5; }\n"
			 << "  </style>\n"
			 << "</head>\n"
			 << "<body>\n"
			 << "  <h1>Index of " << uri << "</h1>\n"
			 << "  <table>\n"
			 << "    <tr><th>Name</th><th>Size</th></tr>\n";

		// Add parent directory link if not root
		if (uri != "/")
		{
			html << "    <tr><td><a href=\"../\">../</a></td><td>-</td></tr>\n";
		}

		// Read directory entries
		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL)
		{
			std::string name = entry->d_name;

			// Skip . and ..
			if (name == "." || name == "..")
			{
				continue;
			}

			// Get file info
			std::string full_path = directory_path + "/" + name;
			struct stat st;

			if (stat(full_path.c_str(), &st) == 0)
			{
				std::string display_name = name;
				std::string size_str;

				if (S_ISDIR(st.st_mode))
				{
					display_name += "/";
					size_str = "-";
				}
				else
				{
					std::ostringstream ss;
					ss << st.st_size;
					size_str = ss.str();
				}

				// Ensure URI ends with / for proper relative links
				std::string base_uri = uri;
				if (!base_uri.empty() && base_uri[base_uri.length() - 1] != '/')
				{
					base_uri += "/";
				}

				html << "    <tr>"
					 << "<td><a href=\"" << base_uri << name << "\">"
					 << display_name << "</a></td>"
					 << "<td>" << size_str << "</td>"
					 << "</tr>\n";
			}
		}

		closedir(dir);

		html << "  </table>\n"
			 << "</body>\n"
			 << "</html>\n";

		std::string body = html.str();

		setStatus(200);
		setHeader("Content-Type", "text/html");

		std::ostringstream ss;
		ss << body.size();
		setHeader("Content-Length", ss.str());

		setBody(body);
	}

}

// Stream insertion operator
std::ostream &operator<<(std::ostream &os, const libhttp::ResponseBlender &response)
{
	os << response.toString();
	return os;
}
