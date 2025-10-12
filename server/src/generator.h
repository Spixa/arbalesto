#pragma once
#include "packets.h"
#include <string>
#include <filesystem>
#include <fstream>
#include <cmath>

class ChunkGenerator {
public:
    explicit ChunkGenerator(unsigned seed = 1337);

    NetChunk generateOrLoad(int32_t cx, int32_t cy);

private:
    float perlin(float x, float y) const;
    float noise2d(int x, int y) const;

    std::string chunkPath(int32_t cx, int32_t cy) const;

private:
    unsigned seed;
};
