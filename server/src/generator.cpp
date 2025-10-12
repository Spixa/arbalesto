#include "generator.h"
#include <random>
#include <sstream>

ChunkGenerator::ChunkGenerator(unsigned seed)
    : seed(seed) {}

std::string ChunkGenerator::chunkPath(int32_t cx, int32_t cy) const {
    std::ostringstream oss;
    oss << "chunks/" << cx << "_" << cy << ".bin";
    return oss.str();
}

float ChunkGenerator::noise2d(int x, int y) const {
    std::uint32_t n = x + y * 57 + seed * 131;
    n = (n << 13) ^ n;
    return 1.0f - ((n * (n * n * 15731u + 789221u) + 1376312589u) & 0x7fffffff) / 1073741824.0f;
}

float ChunkGenerator::perlin(float x, float y) const {
    int xi = static_cast<int>(std::floor(x));
    int yi = static_cast<int>(std::floor(y));
    float xf = x - xi;
    float yf = y - yi;

    float n00 = noise2d(xi, yi);
    float n10 = noise2d(xi + 1, yi);
    float n01 = noise2d(xi, yi + 1);
    float n11 = noise2d(xi + 1, yi + 1);

    float u = xf * xf * (3 - 2 * xf);
    float v = yf * yf * (3 - 2 * yf);

    float nx0 = n00 * (1 - u) + n10 * u;
    float nx1 = n01 * (1 - u) + n11 * u;
    return nx0 * (1 - v) + nx1 * v;
}

NetChunk ChunkGenerator::generateOrLoad(int32_t cx, int32_t cy) {
    NetChunk chunk;
    chunk.x = cx;
    chunk.y = cy;

    std::filesystem::create_directories("chunks");
    auto path = chunkPath(cx, cy);

    if (std::filesystem::exists(path)) {
        std::ifstream file(path, std::ios::binary);
        if (file) {
            for (auto& t : chunk.tiles) {
                Tile val = Tile::Grass;
                file.read(reinterpret_cast<char*>(&val), sizeof(Tile));
                t = val;
            }
            return chunk;
        }
    }

    constexpr int OCTAVES = 4;
    constexpr float LACUNARITY = 2.0f;
    constexpr float GAIN = 0.5f;
    constexpr float SCALE = 0.008f;     // base frequency (smaller = bigger features)
    constexpr float MASK_SCALE = 0.0025f; // lower freq for island mask
    constexpr float MASK_BLEND = 0.45f; // how strongly mask affects final height

    // thresholds (in [0..1] height space)
    constexpr float WATER_LEVEL = 0.38f;
    constexpr float SAND_LEVEL  = 0.44f;
    constexpr float GRASS_LEVEL = 0.48f;

    // helper: FBM built from perlin (perlin returns approx [-1,1]) ---
    auto fbm = [&](float x, float y) {
        float sum = 0.f;
        float amp = 1.f;
        float freq = 1.f;
        float maxAmp = 0.f;
        for (int o = 0; o < OCTAVES; ++o) {
            float n = perlin(x * freq, y * freq); // expected [-1,1]
            sum += n * amp;
            maxAmp += amp;
            amp *= GAIN;
            freq *= LACUNARITY;
        }
        return sum / maxAmp; // still roughly in [-1,1]
    };

    // generate raw heightmap (float per tile)
    std::array<float, CHUNK_SIZE * CHUNK_SIZE> height{};
    for (int y = 0; y < CHUNK_SIZE; ++y) {
        for (int x = 0; x < CHUNK_SIZE; ++x) {
            // world-space tile coords (integer)
            float wx = static_cast<float>(cx * CHUNK_SIZE + x);
            float wy = static_cast<float>(cy * CHUNK_SIZE + y);

            // base detail (normalized to [0,1])
            float base = (fbm(wx * SCALE, wy * SCALE) + 1.f) * 0.5f;

            // low-frequency mask to determine island "presence"
            float mask = (fbm(wx * MASK_SCALE, wy * MASK_SCALE) + 1.f) * 0.5f;

            // blend base and mask (mask gives large coherent island shapes)
            float h = base * (1.f - MASK_BLEND) + mask * MASK_BLEND;

            height[y * CHUNK_SIZE + x] = h;
        }
    }

    // 3x3 box blur to remove speckles (cheap, effective)
    std::array<float, CHUNK_SIZE * CHUNK_SIZE> blurred{};
    for (int y = 0; y < CHUNK_SIZE; ++y) {
        for (int x = 0; x < CHUNK_SIZE; ++x) {
            float sum = 0.f;
            int count = 0;
            for (int oy = -1; oy <= 1; ++oy) {
                for (int ox = -1; ox <= 1; ++ox) {
                    int nx = x + ox;
                    int ny = y + oy;
                    if (nx < 0 || nx >= CHUNK_SIZE || ny < 0 || ny >= CHUNK_SIZE) continue;
                    sum += height[ny * CHUNK_SIZE + nx];
                    ++count;
                }
            }
            blurred[y * CHUNK_SIZE + x] = sum / static_cast<float>(std::max(1, count));
        }
    }

    // map blurred heights to discrete tile bytes
    for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
        float h = blurred[i];

        Tile tile = Tile::Water; // default water
        if (h < WATER_LEVEL) {
            tile = Tile::Water; // water
        } else if (h < SAND_LEVEL) {
            tile = Tile::Sand;
        } else {
            tile = Tile::Grass; // grass
        }
        chunk.tiles[i] = tile;
    }

    // save it (write uint32_t, consistent with load)
    {
        std::ofstream file(path, std::ios::binary);
        if (file) {
            for (auto const& t : chunk.tiles) {
                file.write(reinterpret_cast<const char*>(&t), sizeof(uint32_t));
            }
        }
    }

    return chunk;
}