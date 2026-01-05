#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <string>
#include <map>

namespace libhttp {

	// HTTP Methods
	enum Method {
		GET,
		POST,
		DELETE
	};

	// Status Codes
    enum Status {
        OK = 200,
        CREATED = 201,
        NO_CONTENT = 204,
        BAD_REQUEST = 400,
        FORBIDDEN = 403,
        NOT_FOUND = 404,
        INTERNAL_SERVER_ERROR = 500,
        NOT_IMPLEMENTED = 501
    };

	// Other / Keys
    const std::string HTTP_1_0 = "HTTP/1.0";
	const std::string ContentType = "Content-Type";
	const std::string ContentLength = "Content-Length";
	const std::string Date = "Date";
	const std::string Server = "Server";

	// content types
	const std::map<std::string, std::string> MIME_TYPES = {
		// Text
		{".txt",  "text/plain"},
		{".html", "text/html"},
		{".htm",  "text/html"},
		{".css",  "text/css"},
		{".js",   "application/javascript"},
		{".json", "application/json"},
		{".xml",  "application/xml"},
		{".csv",  "text/csv"},
		{".md",   "text/markdown"},
		
		// Images
		{".png",  "image/png"},
		{".jpg",  "image/jpeg"},
		{".jpeg", "image/jpeg"},
		{".gif",  "image/gif"},
		{".webp", "image/webp"},
		{".svg",  "image/svg+xml"},
		{".ico",  "image/x-icon"},
		{".bmp",  "image/bmp"},
		{".tiff", "image/tiff"},
		
		// Audio
		{".mp3",  "audio/mpeg"},
		{".wav",  "audio/wav"},
		{".ogg",  "audio/ogg"},
		{".flac", "audio/flac"},
		{".aac",  "audio/aac"},
		{".m4a",  "audio/mp4"},
		
		// Video
		{".mp4",  "video/mp4"},
		{".avi",  "video/x-msvideo"},
		{".webm", "video/webm"},
		{".ogg",  "video/ogg"},
		{".mov",  "video/quicktime"},
		{".mkv",  "video/x-matroska"},
		
		// Application / binary
		{".pdf",  "application/pdf"},
		{".zip",  "application/zip"},
		{".gz",   "application/gzip"},
		{".tar",  "application/x-tar"},
		{".7z",   "application/x-7z-compressed"},
		{".rar",  "application/vnd.rar"},
		{".exe",  "application/octet-stream"},
		{".bin",  "application/octet-stream"},
		
		// Fonts
		{".woff",  "font/woff"},
		{".woff2", "font/woff2"},
		{".ttf",   "font/ttf"},
		{".otf",   "font/otf"},
		{".eot",   "application/vnd.ms-fontobject"},
		
		// Scripts / others
		{".sh",    "application/x-sh"},
		{".bat",   "application/x-msdownload"},
		{".ps",    "application/postscript"},
		
		// Archives / compressed
		{".rar",  "application/vnd.rar"},
		{".tar.gz","application/gzip"},
		{".tgz",  "application/gzip"}
	};


	// Helper functions for later
	// std::string getMimeType(const std::string& path);

}

#endif