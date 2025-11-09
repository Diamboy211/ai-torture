#pragma once

#include <vector>
#include <cstdint>
#include <leptonica/allheaders.h>

struct CharGrid
{
	std::vector<uint8_t> data;
	int w, h;
	CharGrid(int w, int h);
	uint8_t &operator()(int x, int y);
	const uint8_t &operator()(int x, int y) const;
};

CharGrid ocr(Pix *img);
