#pragma once

#include <string>
#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#include <mutex>
#include <rfb/rfbclient.h>
#include <leptonica/allheaders.h>

struct KeyEvent
{
	uint32_t key;
	bool down;
};

class RFBContext
{
	std::thread thread_rfb;
	std::mutex mutex_rfb;
	rfbClient *client = nullptr;
	Pix *fb = nullptr;
	std::vector<char> vprog, vhost;
	std::queue<KeyEvent> kb_queue;
	std::chrono::time_point<std::chrono::steady_clock> kb_last;
	std::chrono::milliseconds kb_cooldown;
	char *rfb_argv[3];
	int rfb_argc = 2;
	std::atomic<bool> stop = false, err = false;
	void ev_loop();
public:
	RFBContext(std::string_view prog, std::string_view hostname);
	RFBContext(const RFBContext &) = delete;
	bool running() const;
	void send_key_event(uint32_t key, bool down);
	void set_key_cooldown(int ms);
	void wait_keys();
	Pix *screenshot();
	~RFBContext();
};

