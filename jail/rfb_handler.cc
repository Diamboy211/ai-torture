#include "rfb_handler.hh"
#include <iostream>
#include <mutex>
#include <memory>
#include <cstring>

using namespace std::chrono_literals;

void RFBContext::ev_loop()
{
	do
	{
		int n = WaitForMessage(client, 0);
		if (n < 0)
		{
			err = true;
			break;
		}
		if (n != 0)
		{
			mutex_rfb.lock();
			bool succ = HandleRFBServerMessage(client);
			mutex_rfb.unlock();
			if (!succ)
			{
				err = true;
				break;
			}
		}
		auto current_time = std::chrono::steady_clock::now();
		mutex_rfb.lock();
		// supports vroom vroom mode
		while (current_time - kb_last >= kb_cooldown && !kb_queue.empty())
		{
			kb_last = current_time;
			auto ev = kb_queue.front();
			kb_queue.pop();
			SendKeyEvent(client, ev.key, ev.down);
		}
		mutex_rfb.unlock();
	}
	while (!stop);
	stop = true;
}
RFBContext::RFBContext(std::string_view prog, std::string_view hostname)
{
	kb_last = std::chrono::steady_clock::now();
	kb_cooldown = 67ms;
	auto tovec = [](std::string_view s)
	{
		std::vector<char> c(s.size() + 1);
		c[s.size()] = 0;
		s.copy(c.data(), s.size());
		return c;
	};
	vprog = tovec(prog);
	vhost = tovec(hostname);
	rfb_argv[0] = vprog.data();
	rfb_argv[1] = vhost.data();
	rfb_argv[2] = NULL;
	client = rfbGetClient(8, 3, 4);
	if (client == nullptr)
		throw std::runtime_error("rfbGetClient");
	if (!rfbInitClient(client, &rfb_argc, rfb_argv))
	{
		client = nullptr;
		throw std::runtime_error("rfbInitClient");
	}
	thread_rfb = std::thread(&RFBContext::ev_loop, this);
}

bool RFBContext::running() const
{
	return !stop;
}

void RFBContext::send_key_event(uint32_t key, bool down)
{
	mutex_rfb.lock();
	kb_queue.push({ key, down });
	mutex_rfb.unlock();
}

void RFBContext::set_key_cooldown(int ms)
{
	mutex_rfb.lock();
	kb_cooldown = std::chrono::milliseconds(ms);
	mutex_rfb.unlock();
}

void RFBContext::wait_keys()
{
	for (;;)
	{
		mutex_rfb.lock();
		bool done = kb_queue.empty();
		auto delay = kb_cooldown;
		mutex_rfb.unlock();
		if (done)
		{
			std::this_thread::sleep_for(delay);
			return;
		}
	}
}

Pix *RFBContext::screenshot()
{
	mutex_rfb.lock();
	int w = client->width;
	int h = client->height;
	int step = client->format.bitsPerPixel / 8;
	if (fb == nullptr || pixGetWidth(fb) != w || pixGetHeight(fb) != h)
	{
		pixDestroy(&fb);
		fb = pixCreate(w, h, 32);
		if (fb == nullptr)
		{
			mutex_rfb.unlock();
			return nullptr;
		}
	}
	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++)
		{
		#ifdef L_BIG_ENDIAN
			int offv = 4 - step;
		#else
			int offv = 0;
		#endif
			uint32_t colp = 0;
			for (int i = 0; i < step; i++)
				reinterpret_cast<uint8_t *>(&colp)[i + offv] = client->frameBuffer[(y*w+x)*step+i];
			uint8_t r = (colp >> client->format.redShift) * 255 / client->format.redMax;
			uint8_t g = (colp >> client->format.greenShift) * 255 / client->format.greenMax;
			uint8_t b = (colp >> client->format.blueShift) * 255 / client->format.blueMax;
			pixSetRGBPixel(fb, x, y, r, g, b);
		}
	mutex_rfb.unlock();
	return pixClone(fb);
}

RFBContext::~RFBContext()
{
	if (thread_rfb.joinable())
	{
		stop = true;
		thread_rfb.join();
	}
	if (client != nullptr) rfbClientCleanup(client);
	pixDestroy(&fb);
}

