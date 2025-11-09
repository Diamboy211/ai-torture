#include <cstdlib>
#include "helper.hh"

std::string getenv_cc(std::string_view name, std::string_view def)
{
	std::string name_cpy = std::string(name);
	const char *env = getenv(name_cpy.c_str());
	if (env == NULL) return std::string(def);
	return std::string(env);
}
