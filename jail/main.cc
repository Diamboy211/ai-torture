#include <iostream>
#include <cstdio>
#include <string>
#include <memory>
#include <leptonica/allheaders.h>
#include "rfb_handler.hh"
#include "helper.hh"
#include "ocr.hh"

int main(int argc, char **argv)
{
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
	RFBContext rfb(argv[0], rfb_host);

	while (rfb.running())
	{
		std::getchar();
		Pix *screenie = rfb.screenshot();
		auto ocr_res = ocr(screenie);
		for (int y = 0; y < ocr_res.h; y++)
		{
			for (int x = 0; x < ocr_res.w; x++)
				std::cerr << static_cast<char>(ocr_res(x, y));
			std::cerr << '\n';
		}
		pixDestroy(&screenie);
	}
	return 0;
}
