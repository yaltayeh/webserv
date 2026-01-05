#include "Constants.hpp"

namespace libhttp {

    static std::map<std::string, std::string> initMimeTypes() {
        std::map<std::string, std::string> m;
        // Text
        m[".txt"] = "text/plain";
        m[".html"] = "text/html";
        m[".htm"] = "text/html";
        m[".css"] = "text/css";
        m[".js"] = "application/javascript";
        m[".json"] = "application/json";
        m[".xml"] = "application/xml";
        m[".csv"] = "text/csv";
        m[".md"] = "text/markdown";
        
        // Images
        m[".png"] = "image/png";
        m[".jpg"] = "image/jpeg";
        m[".jpeg"] = "image/jpeg";
        m[".gif"] = "image/gif";
        m[".webp"] = "image/webp";
        m[".svg"] = "image/svg+xml";
        m[".ico"] = "image/x-icon";
        m[".bmp"] = "image/bmp";
        m[".tiff"] = "image/tiff";
        
        // Audio
        m[".mp3"] = "audio/mpeg";
        m[".wav"] = "audio/wav";
        m[".ogg"] = "audio/ogg";
        m[".flac"] = "audio/flac";
        m[".aac"] = "audio/aac";
        m[".m4a"] = "audio/mp4";
        
        // Video
        m[".mp4"] = "video/mp4";
        m[".avi"] = "video/x-msvideo";
        m[".webm"] = "video/webm";
        m[".ogg"] = "video/ogg";
        m[".mov"] = "video/quicktime";
        m[".mkv"] = "video/x-matroska";
        
        // Application / binary
        m[".pdf"] = "application/pdf";
        m[".zip"] = "application/zip";
        m[".gz"] = "application/gzip";
        m[".tar"] = "application/x-tar";
        m[".7z"] = "application/x-7z-compressed";
        m[".rar"] = "application/vnd.rar";
        m[".exe"] = "application/octet-stream";
        m[".bin"] = "application/octet-stream";
        
        // Fonts
        m[".woff"] = "font/woff";
        m[".woff2"] = "font/woff2";
        m[".ttf"] = "font/ttf";
        m[".otf"] = "font/otf";
        m[".eot"] = "application/vnd.ms-fontobject";
        
        // Scripts / others
        m[".sh"] = "application/x-sh";
        m[".bat"] = "application/x-msdownload";
        m[".ps"] = "application/postscript";
        
        // Archives / compressed
        m[".rar"] = "application/vnd.rar";
        m[".tar.gz"] = "application/gzip";
        m[".tgz"] = "application/gzip";

        return m;
    }

    const std::map<std::string, std::string> MIME_TYPES = initMimeTypes();

}
