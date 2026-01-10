#ifndef SERVERDIRECTIVE_HPP
#define SERVERDIRECTIVE_HPP

#include "IDirective.hpp"
#include "../config.hpp"

namespace Config
{
	class ServerDirective : public IDirective
	{
	public:
		ServerDirective();
		ServerDirective(const ServerDirective& other);
		ServerDirective& operator=(const ServerDirective& other);
		~ServerDirective();

		static ServerDirective* initServerDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
		
		/**
		 * Convert this ServerDirective and its children to a ServerConfig
		 * 
		 * Traverses all child directives (listen, server_name, location, etc.)
		 * and populates a ServerConfig structure with all configuration values.
		 */
		ServerConfig toServerConfig() const;
	
	private:
		/**
		 * Convert a LocationDirective to a RouteConfig
		 */
		RouteConfig locationToRouteConfig(const class LocationDirective* location) const;
		
		/**
		 * Apply context-inheritable directives to RouteConfig
		 */
		void applyContextDirectives(RouteConfig& route, const IDirective* context) const;
	};
}

#endif
