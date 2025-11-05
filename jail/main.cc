#include "liboai.h"
#include <iostream>

std::string getenv_cc(std::string_view name, std::string_view def = "")
{
	std::string name_cpy = std::string(name);
	const char *env = getenv(name_cpy.c_str());
	if (env == NULL) return std::string(def);
	return std::string(env);
}

int main()
{
	std::string oai_endpoint = getenv_cc("OPENAI_API_BASE", "https://api.deepseek.com");
	liboai::OpenAI oai(oai_endpoint);
	if (!oai.auth.SetKeyEnv("OPENAI_API_KEY"))
	{
		std::cerr << "pls provide a valid api key in the env variable OPENAI_API_KEY" << std::endl;
		return 1;
	}
	std::string oai_model = getenv_cc("OPENAI_MODEL", "deepseek-chat");
	std::cout << "Using API endpoint: " << oai_endpoint << '\n';
	std::cout << "Using model: " << oai_model << '\n';
	try
	{
		liboai::Conversation conv("You are a little sister character talking to your brother (named Onii-chan).");
		std::string user_res;
		for (;;)
		{
			std::cout << "> ";
			std::getline(std::cin, user_res);
			if (user_res == "q") break;
			conv.AddUserData(user_res);
			liboai::Response res = oai.ChatCompletion->create(oai_model, conv);
			conv.Update(res);
			std::cout << conv.GetLastResponse() << std::endl;
		}
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
}
