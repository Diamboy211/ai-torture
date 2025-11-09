#include <bitset>
#include <cstring>
#include <iostream>
#include <iomanip>
#include "vgafont16.hh"
#include "ocr.hh"

CharGrid::CharGrid(int w, int h) : w(w), h(h), data(w*h) {}

uint8_t &CharGrid::operator()(int x, int y)
{
	return data[y*w+x];
}

const uint8_t &CharGrid::operator()(int x, int y) const
{
	return data[y*w+x];
}

static int sim(uint8_t a[16], uint8_t b[16])
{
	uint64_t a64, b64;
	std::memcpy(&a64, a, 8);
	std::memcpy(&b64, b, 8);
	int r = __builtin_popcountll(a64 ^ b64);
	std::memcpy(&a64, a+8, 8);
	std::memcpy(&b64, b+8, 8);
	r += __builtin_popcountll(a64 ^ b64);
	return 128 - r;
}

CharGrid ocr(Pix *img)
{
	int w, h;
	pixGetDimensions(img, &w, &h, nullptr);
	int cw = w / 9, ch = h / 16;
	uint8_t c[16];
	CharGrid cg(cw, ch);
	for (int y = 0; y < ch; y++)
		for (int x = 0; x < cw; x++)
		{
			uint32_t bg;
			pixGetPixel(img, x*9+8, y*16+15, &bg);
			bool is_space = true;
			for (int i = 0; i < 16; i++)
			{
				c[i] = 0;
				for (int j = 0; j < 8; j++)
				{
					uint32_t col;
					pixGetPixel(img, x*9+j, y*16+i, &col);
					if (col != bg) c[i] |= 0x80u >> j;
				}
				is_space = is_space && c[i] == 0;
			}
			if (is_space)
			{
				cg(x, y) = ' ';
				continue;
			}
			int max_sim = -1;
			uint8_t max_sim_char = 0;
			for (int i = 1; i < 256; i++)
			{
				int s = sim(c, vgafont16 + i*16);
				if (max_sim < s) max_sim = s, max_sim_char = i;
			}
			cg(x, y) = max_sim_char;
		}
	return cg;
}
