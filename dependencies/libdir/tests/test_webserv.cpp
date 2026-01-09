// Test for complete webserv configuration with all required directives
#include "Config.hpp"
#include "Parser.hpp"
#include "IDirective.hpp"
#include "HttpDirective.hpp"
#include "ServerDirective.hpp"
#include "LocationDirective.hpp"
#include "ServerNameDirective.hpp"
#include "ListenDirective.hpp"
#include "RootDirective.hpp"
#include "ErrorPageDirective.hpp"
#include "ClientMaxBodySizeDirective.hpp"
#include "KeepaliveTimeoutDirective.hpp"
#include "AllowedMethodsDirective.hpp"
#include "ReturnDirective.hpp"
#include "AutoindexDirective.hpp"
#include "IndexDirective.hpp"
#include "UploadEnableDirective.hpp"
#include "UploadStoreDirective.hpp"
#include "CgiPassDirective.hpp"
#include "CgiParamDirective.hpp"
#include "CgiExtensionDirective.hpp"
#include <iostream>
#include <fstream>

void printServerConfig(const Config::ServerDirective* server) {
	std::cout << "  Server Configuration:" << std::endl;
	
	// Server name
	try {
		const Config::ServerNameDirective* serverName = 
			server->getFirstChild<Config::ServerNameDirective>();
		if (serverName) {
			std::cout << "    Server names: ";
			for (size_t i = 0; i < serverName->names.size(); ++i) {
				std::cout << serverName->names[i];
				if (i < serverName->names.size() - 1) std::cout << ", ";
			}
			std::cout << std::endl;
		}
	} catch (const Config::DirectiveNotFound&) {}
	
	// Listen
	try {
		const Config::ListenDirective* listen = 
			server->getFirstChild<Config::ListenDirective>();
		if (listen) {
			std::cout << "    Listen: " << listen->host << ":" << listen->port << std::endl;
		}
	} catch (const Config::DirectiveNotFound&) {}
	
	// Root
	try {
		const Config::RootDirective* root = 
			server->getFirstChild<Config::RootDirective>();
		if (root) {
			std::cout << "    Root: " << root->path << std::endl;
		}
	} catch (const Config::DirectiveNotFound&) {}
	
	// Error pages
	std::vector<Config::ErrorPageDirective*> errorPages = 
		server->getChildren<Config::ErrorPageDirective>();
	if (!errorPages.empty()) {
		std::cout << "    Error pages:" << std::endl;
		for (size_t i = 0; i < errorPages.size(); ++i) {
			std::cout << "      Codes: ";
			for (size_t j = 0; j < errorPages[i]->codes.size(); ++j) {
				std::cout << errorPages[i]->codes[j];
				if (j < errorPages[i]->codes.size() - 1) std::cout << ", ";
			}
			std::cout << " -> " << errorPages[i]->uri << std::endl;
		}
	}
	
	// Client max body size
	try {
		const Config::ClientMaxBodySizeDirective* maxBodySize = 
			server->getFirstChild<Config::ClientMaxBodySizeDirective>();
		if (maxBodySize) {
			std::cout << "    Max body size: " << maxBodySize->size << std::endl;
		}
	} catch (const Config::DirectiveNotFound&) {
		// Not configured at this level
	}
	
	// Index files
	try {
		const Config::IndexDirective* index = 
			server->getFirstChild<Config::IndexDirective>();
		if (index) {
			std::cout << "    Index files: ";
			for (size_t i = 0; i < index->files.size(); ++i) {
				std::cout << index->files[i];
				if (i < index->files.size() - 1) std::cout << ", ";
			}
			std::cout << std::endl;
		}
	} catch (const Config::DirectiveNotFound&) {
		// Not configured at this level
	}
	
	// Autoindex
	try {
		const Config::AutoindexDirective* autoindex = 
			server->getFirstChild<Config::AutoindexDirective>();
		if (autoindex) {
			std::cout << "    Autoindex: " << (autoindex->enabled ? "on" : "off") << std::endl;
		}
	} catch (const Config::DirectiveNotFound&) {
		// Not configured at this level
	}
	
	// CGI extensions
	try {
		const Config::CgiExtensionDirective* cgiExt = 
			server->getFirstChild<Config::CgiExtensionDirective>();
		if (cgiExt) {
			std::cout << "    CGI extensions: ";
			for (size_t i = 0; i < cgiExt->extensions.size(); ++i) {
				std::cout << cgiExt->extensions[i];
				if (i < cgiExt->extensions.size() - 1) std::cout << ", ";
			}
			std::cout << std::endl;
		}
	} catch (const Config::DirectiveNotFound&) {
		// Not configured at this level
	}
}

void printLocationConfig(const Config::LocationDirective* location, int indent = 4) {
	std::string indentStr(indent, ' ');
	std::cout << indentStr << "Location: ";
	if (!location->modifier.empty()) {
		std::cout << location->modifier << " ";
	}
	std::cout << location->uri << std::endl;
	
	// Allowed methods
	try {
		const Config::AllowedMethodsDirective* methods = 
			location->getFirstChild<Config::AllowedMethodsDirective>();
		if (methods) {
			std::cout << indentStr << "  Allowed methods: ";
			for (size_t i = 0; i < methods->methods.size(); ++i) {
				std::cout << methods->methods[i];
				if (i < methods->methods.size() - 1) std::cout << ", ";
			}
			std::cout << std::endl;
		}
	} catch (const Config::DirectiveNotFound&) {}
	
	// Return
	try {
		const Config::ReturnDirective* returnDir = 
			location->getFirstChild<Config::ReturnDirective>();
		if (returnDir) {
			std::cout << indentStr << "  Return: " << returnDir->code;
			if (!returnDir->text.empty()) {
				std::cout << " " << returnDir->text;
			}
			std::cout << std::endl;
		}
	} catch (const Config::DirectiveNotFound&) {}
	
	// Root
	try {
		const Config::RootDirective* root = 
			location->getFirstChild<Config::RootDirective>();
		if (root) {
			std::cout << indentStr << "  Root: " << root->path << std::endl;
		}
	} catch (const Config::DirectiveNotFound&) {}
	
	// Index
	try {
		const Config::IndexDirective* index = 
			location->getFirstChild<Config::IndexDirective>();
		if (index) {
			std::cout << indentStr << "  Index: ";
			for (size_t i = 0; i < index->files.size(); ++i) {
				std::cout << index->files[i];
				if (i < index->files.size() - 1) std::cout << ", ";
			}
			std::cout << std::endl;
		}
	} catch (const Config::DirectiveNotFound&) {}
	
	// Autoindex
	try {
		const Config::AutoindexDirective* autoindex = 
			location->getFirstChild<Config::AutoindexDirective>();
		if (autoindex) {
			std::cout << indentStr << "  Autoindex: " << (autoindex->enabled ? "on" : "off") << std::endl;
		}
	} catch (const Config::DirectiveNotFound&) {}
	
	// Upload enable
	try {
		const Config::UploadEnableDirective* uploadEnable = 
			location->getFirstChild<Config::UploadEnableDirective>();
		if (uploadEnable) {
			std::cout << indentStr << "  Upload enable: " << (uploadEnable->enabled ? "on" : "off") << std::endl;
		}
	} catch (const Config::DirectiveNotFound&) {}
	
	// Upload store
	try {
		const Config::UploadStoreDirective* uploadStore = 
			location->getFirstChild<Config::UploadStoreDirective>();
		if (uploadStore) {
			std::cout << indentStr << "  Upload store: " << uploadStore->path << std::endl;
		}
	} catch (const Config::DirectiveNotFound&) {}
	
	// CGI pass
	try {
		const Config::CgiPassDirective* cgiPass = 
			location->getFirstChild<Config::CgiPassDirective>();
		if (cgiPass) {
			std::cout << indentStr << "  CGI pass: " << cgiPass->interpreterPath << std::endl;
		}
	} catch (const Config::DirectiveNotFound&) {}
	
	// CGI params
	std::vector<Config::CgiParamDirective*> cgiParams = 
		location->getChildren<Config::CgiParamDirective>();
	if (!cgiParams.empty()) {
		std::cout << indentStr << "  CGI params:" << std::endl;
		for (size_t i = 0; i < cgiParams.size(); ++i) {
			std::cout << indentStr << "    " << cgiParams[i]->name 
					  << " = " << cgiParams[i]->value << std::endl;
		}
	}
	
	// Nested locations
	std::vector<Config::LocationDirective*> nestedLocations = 
		location->getChildren<Config::LocationDirective>();
	if (!nestedLocations.empty()) {
		std::cout << indentStr << "  Nested locations:" << std::endl;
		for (size_t i = 0; i < nestedLocations.size(); ++i) {
			printLocationConfig(nestedLocations[i], indent + 4);
		}
	}
}

int main() {
	try {
		// Write comprehensive configuration file
		std::ofstream conf("tests/webserv_complete.conf");
		conf << "http {\n"
			 << "    # HTTP level defaults\n"
			 << "    client_max_body_size 10M;\n"
			 << "    keepalive_timeout 60s 30s;\n"
			 << "    error_page 500 502 503 504 /50x.html;\n"
			 << "    cgi_extension .php .py;\n"
			 << "\n"
			 << "    # First server - Main website on port 8080\n"
			 << "    server {\n"
			 << "        listen 127.0.0.1:8080;\n"
			 << "        server_name localhost www.example.com example.com;\n"
			 << "        root /var/www/html;\n"
			 << "        index index.html index.htm;\n"
			 << "        autoindex off;\n"
			 << "        error_page 404 /404.html;\n"
			 << "        error_page 403 /403.html;\n"
			 << "\n"
			 << "        # Static files location\n"
			 << "        location / {\n"
			 << "            allowed_methods GET POST;\n"
			 << "            index index.html;\n"
			 << "        }\n"
			 << "\n"
			 << "        # API location with redirect\n"
			 << "        location /api {\n"
			 << "            return 301 https://api.example.com;\n"
			 << "        }\n"
			 << "\n"
			 << "        # Downloads with autoindex\n"
			 << "        location /downloads {\n"
			 << "            allowed_methods GET;\n"
			 << "            root /var/www/downloads;\n"
			 << "            autoindex on;\n"
			 << "        }\n"
			 << "\n"
			 << "        # Upload location\n"
			 << "        location /upload {\n"
			 << "            allowed_methods POST PUT DELETE;\n"
			 << "            upload_enable on;\n"
			 << "            upload_store /var/www/uploads;\n"
			 << "            client_max_body_size 50M;\n"
			 << "        }\n"
			 << "\n"
			 << "        # CGI scripts - PHP\n"
			 << "        location /cgi-bin/php/ {\n"
			 << "            allowed_methods GET POST;\n"
			 << "            root /var/www/cgi-bin;\n"
			 << "            cgi_pass /usr/bin/php-cgi;\n"
			 << "            cgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;\n"
			 << "            cgi_param QUERY_STRING $query_string;\n"
			 << "        }\n"
			 << "\n"
			 << "        # Python CGI\n"
			 << "        location /cgi-bin/python/ {\n"
			 << "            allowed_methods GET POST;\n"
			 << "            root /var/www/cgi-bin;\n"
			 << "            cgi_pass /usr/bin/python3;\n"
			 << "            cgi_param SCRIPT_NAME $fastcgi_script_name;\n"
			 << "        }\n"
			 << "    }\n"
			 << "\n"
			 << "    # Second server - Admin panel on port 8081\n"
			 << "    server {\n"
			 << "        listen 127.0.0.1:8081;\n"
			 << "        server_name admin.example.com;\n"
			 << "        root /var/www/admin;\n"
			 << "        index admin.html;\n"
			 << "        client_max_body_size 5M;\n"
			 << "\n"
			 << "        location / {\n"
			 << "            allowed_methods GET POST PUT DELETE;\n"
			 << "        }\n"
			 << "\n"
			 << "        location /secure {\n"
			 << "            return 403;\n"
			 << "        }\n"
			 << "    }\n"
			 << "\n"
			 << "    # Third server - Default server (catches all other requests)\n"
			 << "    server {\n"
			 << "        listen 0.0.0.0:80;\n"
			 << "        server_name _;\n"
			 << "        return 444;\n"
			 << "    }\n"
			 << "}\n";
		conf.close();
		
		// Parse the configuration
		std::cout << "=== Testing Complete Webserv Configuration ===" << std::endl;
		std::cout << "\nParsing configuration file..." << std::endl;
		
		Config::Parser parser = Config::Parser::openConfigFile("tests/webserv_complete.conf");
		while (parser.getNextDirectives()) {
			std::string directive = parser.copyDirective();
			Config::IDirective::parseDirective(Config::global, directive);
		}
		
		// Get HTTP block
		const Config::HttpDirective* http = 
			Config::global->getFirstChild<Config::HttpDirective>();
		if (!http) {
			std::cerr << "ERROR: No HTTP block found" << std::endl;
			return 1;
		}
		
		std::cout << "\n=== HTTP Level Configuration ===" << std::endl;
		
		// HTTP level max body size
		const Config::ClientMaxBodySizeDirective* httpMaxBodySize = 
			http->getFirstChild<Config::ClientMaxBodySizeDirective>();
		if (httpMaxBodySize) {
			std::cout << "  Default max body size: " << httpMaxBodySize->size << std::endl;
		}
		
		// HTTP level keepalive
		const Config::KeepaliveTimeoutDirective* httpKeepalive = 
			http->getFirstChild<Config::KeepaliveTimeoutDirective>();
		if (httpKeepalive) {
			std::cout << "  Default keepalive timeout: " << httpKeepalive->timeout;
			if (!httpKeepalive->headerTimeout.empty()) {
				std::cout << " (header: " << httpKeepalive->headerTimeout << ")";
			}
			std::cout << std::endl;
		}
		
		// HTTP level error pages
		std::vector<Config::ErrorPageDirective*> httpErrorPages = 
			http->getChildren<Config::ErrorPageDirective>();
		if (!httpErrorPages.empty()) {
			std::cout << "  Default error pages:" << std::endl;
			for (size_t i = 0; i < httpErrorPages.size(); ++i) {
				std::cout << "    Codes: ";
				for (size_t j = 0; j < httpErrorPages[i]->codes.size(); ++j) {
					std::cout << httpErrorPages[i]->codes[j];
					if (j < httpErrorPages[i]->codes.size() - 1) std::cout << ", ";
				}
				std::cout << " -> " << httpErrorPages[i]->uri << std::endl;
			}
		}
		
		// HTTP level CGI extensions
		const Config::CgiExtensionDirective* httpCgiExt = 
			http->getFirstChild<Config::CgiExtensionDirective>();
		if (httpCgiExt) {
			std::cout << "  Default CGI extensions: ";
			for (size_t i = 0; i < httpCgiExt->extensions.size(); ++i) {
				std::cout << httpCgiExt->extensions[i];
				if (i < httpCgiExt->extensions.size() - 1) std::cout << ", ";
			}
			std::cout << std::endl;
		}
		
		// Get all servers
		std::vector<Config::ServerDirective*> servers = 
			http->getChildren<Config::ServerDirective>();
		
		std::cout << "\n=== Found " << servers.size() << " server blocks ===" << std::endl;
		
		// Print each server configuration
		for (size_t i = 0; i < servers.size(); ++i) {
			std::cout << "\n--- Server " << (i + 1) << " ---" << std::endl;
			printServerConfig(servers[i]);
			
			// Get all locations in this server
			std::vector<Config::LocationDirective*> locations = 
				servers[i]->getChildren<Config::LocationDirective>();
			
			if (!locations.empty()) {
				std::cout << "    Locations:" << std::endl;
				for (size_t j = 0; j < locations.size(); ++j) {
					printLocationConfig(locations[j]);
				}
			}
		}
		
		std::cout << "\n=== All Tests Passed ===" << std::endl;
		return 0;
		
	} catch (const std::exception& e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
		return 1;
	}
}
