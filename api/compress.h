#pragma once

#include <zlib.h>

#include <iostream>
#include <vector>

//std::vector<unsigned char> gzipCompress(const std::vector<unsigned char>& input);

std::vector<uint8_t> compressBuffer(uint8_t* input_ptr, uint32_t input_size);