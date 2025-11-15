#include <iostream>
#include <sstream>
#include <cstdio>
#include <string>
#include <memory>
#include <map>
#include <sys/wait.h>
#include <leptonica/allheaders.h>
#include <liboai.h>
#include <nlohmann/json.hpp>
#include "rfb_handler.hh"
#include "helper.hh"
#include "ocr.hh"
#include "sysprom.inc.cc"

nlohmann::json tc_ocr(RFBContext &rfb, const nlohmann::json &args)
{
	#include "utf8map.inc.cc"
	Pix *scr = rfb.screenshot();
	CharGrid cg = ocr(scr);
	pixDestroy(&scr);
	std::string res;
	bool uni = false;
	for (int i = 0; i < cg.h; i++)
	{
		for (int j = 0; j < cg.w; j++)
		{
			uint16_t c = codepoint_map[cg(j, i)];
			if (c < 0x80) res += c;
			else if (c < 0x800)
			{
				res += 0b11000000 + (c >> 6);
				res += 0b10000000 + (c & 63);
				uni = true;
			}
			else
			{
				res += 0b11100000 + (c >> 12);
				res += 0b10000000 + ((c >> 6) & 63);
				res += 0b10000000 + (c & 63);
				uni = true;
			}
		}
		if (i != cg.h-1) res += '\n';
	}
	if (!uni) return nlohmann::json({ { "text", res } });
	else return nlohmann::json({ { "text", res }, { "warning", "Non-ASCII characters detected." } });
}

nlohmann::json tc_assemble(RFBContext &rfb, const nlohmann::json &args)
{
	(void)rfb;
	constexpr int ERR_LIMIT = 1024;
	constexpr int BYTECODE_LIMIT = 1024;
	std::string code;
	try { code = args.value("code", ""); }
	catch (nlohmann::json::type_error &e)
	{
		return nlohmann::json({ { "error", "args.code must be a string" } });
	}
	char tmpn_in[] = "/tmp/fileXXXXXX";
	char tmpn_out[] = "/tmp/fileXXXXXX";
	// defeats the point of mkstemp?
	close(mkstemp(tmpn_in));
	close(mkstemp(tmpn_out));
	std::ofstream fi(tmpn_in);
	fi << code;
	fi.close();
	char exec_argv0[] = "nasm";
	char exec_argv1[] = "-o";
	char *exec_argv[5] = { exec_argv0, exec_argv1, tmpn_out, tmpn_in, NULL };
	int pipefd[2];
	pipe(pipefd);
	pid_t pid = fork();
	if (pid == 0)
	{
		dup2(pipefd[1], STDERR_FILENO);
		close(pipefd[0]);
		close(pipefd[1]);
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		execv("/usr/bin/nasm", exec_argv);
		write(STDERR_FILENO, "exec failed", 11);
		_exit(EXIT_FAILURE);
	}
	close(pipefd[1]);
	char err[ERR_LIMIT+1];
	ssize_t read_bytes = read(pipefd[0], err, ERR_LIMIT);
	err[read_bytes] = 0;
	int wstatus;
	waitpid(pid, &wstatus, 0);
	bool success = WIFEXITED(wstatus) && WEXITSTATUS(wstatus) == 0;
	close(pipefd[0]);
	std::ifstream fo(tmpn_out);
	char bytecode[BYTECODE_LIMIT*2];
	int bytecode_length = 0;
	for (; success && bytecode_length < BYTECODE_LIMIT; bytecode_length++)
	{
		uint8_t ch = fo.get();
		if (fo.eof()) break;
		const char *hex_table = "0123456789abcdef";
		bytecode[bytecode_length * 2    ] = hex_table[ch / 16];
		bytecode[bytecode_length * 2 + 1] = hex_table[ch % 16];
	}
	fo.close();
	std::remove(tmpn_in);
	std::remove(tmpn_out);
	return nlohmann::json({
		{ "hex", std::string_view(bytecode, bytecode_length*2) },
		{ "bytes", bytecode_length },
		{ "success", success },
		{ "err", err }
	});
}

nlohmann::json tc_type_auto(RFBContext &rfb, const nlohmann::json &args)
{
	std::string text;
	try { text = args.value("text", ""); }
	catch (nlohmann::json::type_error &e)
	{
		return nlohmann::json({ { "error", "args.text must be a string" } });
	}
	for (char c : text)
	{
		uint32_t key;
		if (c >= 0x20 && c <= 0x7e) key = c;
		else if (c == '\b') key = XK_BackSpace;
		else if (c == '\n' || c == '\r') key = XK_Return;
		else if (c == '\t') key = XK_Tab;
		else continue;
		rfb.send_key_event(key, true);
		rfb.send_key_event(key, false);
	}
	return nlohmann::json({ { "OK", true } });
}

nlohmann::json tc_type_manual(RFBContext &rfb, const nlohmann::json &args)
{
	#include "scancode.inc.cc"
	nlohmann::json events;
	try { events = args.value("events", nlohmann::json::array()); }
	catch (nlohmann::json::type_error &e)
	{
		return nlohmann::json({ { "error", "args.events must be an array" } });
	}
	for (const nlohmann::json &jstr : events)
	{
		if (!jstr.is_string()) continue;
		std::istringstream evss(jstr.get<std::string>());
		std::string down, ev;
		if (!(evss >> down >> ev)) continue;
		int bdown = 0;
		if (down == "down") bdown = 1;
		else if (down == "up") bdown = -1;
		else if (down != "press") continue;
		if (scancode_table.count(ev) == 0) continue;
		uint32_t key = scancode_table.at(ev);
		if (bdown >= 0) rfb.send_key_event(key, true);
		if (bdown <= 0) rfb.send_key_event(key, false);
	}
	return nlohmann::json({ { "OK", true } });
}

nlohmann::json tc_delay(RFBContext &rfb, const nlohmann::json &args)
{
	int ms;
	try { ms = args.value("ms", 0); }
	catch (nlohmann::json::type_error &e)
	{
		return nlohmann::json({ { "error", "args.ms must be a number" } });
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	return nlohmann::json({ { "OK", true } });
}

nlohmann::json tc_set_key_delay(RFBContext &rfb, const nlohmann::json &args)
{
	int ms;
	try { ms = args.value("ms", 0); }
	catch (nlohmann::json::type_error &e)
	{
		return nlohmann::json({ { "error", "args.ms must be a number" } });
	}
	rfb.set_key_cooldown(ms);
	return nlohmann::json({ { "OK", true } });
}

nlohmann::json tc_call_for_help(RFBContext &rfb, const nlohmann::json &args)
{
	std::string ms;
	try { ms = args.value("message", ""); }
	catch (nlohmann::json::type_error &e)
	{
		return nlohmann::json({ { "error", "args.message must be a string" } });
	}
	std::string res;
	std::cout << "res: ";
	std::getline(std::cin, res, std::cin.widen('\\'));
	return nlohmann::json({ { "message", res } });
}

nlohmann::json tc_wait_keys(RFBContext &rfb, const nlohmann::json &args)
{
	rfb.wait_keys();
	return nlohmann::json({ { "OK", true } });
}

int main(int argc, char **argv)
{
	signal(SIGPIPE, SIG_IGN);
	static const std::map<std::string_view, nlohmann::json (*)(RFBContext &, const nlohmann::json &)> tc_table{
		{ "ocr", tc_ocr },
		{ "assemble", tc_assemble },
		{ "type_auto", tc_type_auto },
		{ "type_manual", tc_type_manual },
		{ "delay", tc_delay },
		{ "set_key_delay", tc_set_key_delay },
		{ "wait_keys", tc_wait_keys },
		{ "call_for_help", tc_call_for_help },
	};
	using namespace std::chrono_literals;
	int rfb_port;
	try
	{
		rfb_port = stoi(getenv_cc("RFB_PORT", "0"));
	}
	catch (std::invalid_argument &)
	{
		std::cerr << "warning: the environment variable RFB_PORT is not a valid number, using default\n";
		rfb_port = 0;
	}
	char rfb_host[24];
	std::sprintf(rfb_host, "127.0.0.1:%d", 5900 + rfb_port);

	std::string oai_endpoint = getenv_cc("OPENAI_API_BASE", "https://api.deepseek.com");
	liboai::OpenAI oai(oai_endpoint);
	oai.auth.SetMaxTimeout(3600000);
	if (!oai.auth.SetKeyEnv("OPENAI_API_KEY"))
	{
		std::cerr << "pls provide a valid api key in the env variable OPENAI_API_KEY" << std::endl;
		return 1;
	}
	std::string oai_model = getenv_cc("OPENAI_MODEL", "deepseek-chat");
	std::cout << "Using API endpoint: " << oai_endpoint << '\n';
	std::cout << "Using model: " << oai_model << '\n';
	liboai::Conversation conv(system_prompt);

	RFBContext rfb(argv[0], rfb_host);

	for (int T = 1;; T++)
	{
		std::cout << "\n------- TURN " << T << " -------\n";
		liboai::Response res;
		bool err = false;
		try
		{
			std::cout << "waiting...\n";
			res = oai.ChatCompletion->create(oai_model, conv, std::nullopt, 0);
			if (!conv.Update(res)) err = true;
		}
		catch (std::exception &e)
		{
			std::cerr << e.what() << std::endl;
			err = true;
		}
		std::cout << "res: " << conv.GetLastResponse() << '\n';
		bool seq = err;
		std::istringstream ss(conv.GetLastResponse());
		std::string line;
		std::string res_data;
		while (!err && !ss.eof())
		{
			std::getline(ss, line);
			nlohmann::json toolcall = nlohmann::json::parse(line, nullptr, false, true);
			if (toolcall.is_discarded()) continue;
			std::string call, id;
			nlohmann::json args;
			static int unnamed_id_count = 0;
			try
			{
				call = toolcall.value("call", "");
				id = toolcall.value("id", "");
				args = toolcall.value("args", nlohmann::json::object());
			}
			catch (nlohmann::json::type_error &e)
			{
				std::cout << "type error\n";
				continue;
			}
			if (id == "") id = "__unnamed" + std::to_string(unnamed_id_count++);
			std::cout << "call: " << call << "\nid: " << id << "\nargs: " << args.dump() << '\n';
			nlohmann::json result;
			if (tc_table.count(call)) result = tc_table.at(call)(rfb, args);
			else if (call == "sequence_point")
			{
				int ok;
				std::cout << "result for seq call: ";
				std::cin >> ok;
				result = { { "OK", !!ok } };
			}
			else result = { { "error", "not a valid tool call" } };
			nlohmann::json tc_res({
				{ "call", call },
				{ "id", id },
				{ "return", result }
			});
			res_data += tc_res.dump() + '\n';
		}
		if (res_data == "") res_data = "Warning: no tool calls detected. There might be a possible syntax error.";
		if (!err)
		{
			std::cout << "result: " << res_data;
			bool lol = conv.AddUserData(res_data);
		}
		if (seq)
		{
			std::cout << "continue?";
			std::getchar();
		}
	}
	return 0;
}
