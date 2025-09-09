#pragma once

#include <SFML/Graphics.hpp>
#include <fstream>

constexpr uint8_t CHUNK_SIZE = 16;

enum class Tile: uint32_t {
    Grass = 0,
    Water = 1,
    Tree = 2,
    Cobble = 3,
};

constexpr float TILE_SIZE = 32.f;

class Chunk {
public:
    Chunk(std::array<Tile, CHUNK_SIZE*CHUNK_SIZE> const& data, sf::Vector2i pos);
    virtual ~Chunk();

    bool isSolidTile(int r, int c) {
        if (r < 0 || r >= CHUNK_SIZE || c < 0 || c >= CHUNK_SIZE)
            return false;

        Tile tile = data[r * CHUNK_SIZE + c];
        return tile == Tile::Water;
    }

    Tile getTile(int r, int c) const {
        return data[r*CHUNK_SIZE + c];
    }

    sf::Vector2f getOffset() const { return offset; }

    [[nodiscard]] bool load() {
        auto path = getFile();
        std::ifstream file(path, std::ios::binary);
        if (!file) return false;

        file.read(reinterpret_cast<char*>(data.data()), CHUNK_SIZE * CHUNK_SIZE * sizeof(Tile));
        build();

        return file.good();
    }

    bool save() {
        auto path = getFile();
        std::ofstream file(path, std::ios::binary);
        if (!file) return false;

        file.write(reinterpret_cast<const char*>(data.data()), CHUNK_SIZE * CHUNK_SIZE * sizeof(Tile));
        return true;
    }
    void update_tick(sf::Vector2f mouse_coords, Tile selected);
    void render(sf::RenderTarget& target);

    std::string getFile() const {
        std::ostringstream oss;
        auto code = encode_name();
        oss << std::hex << code << ".bin";
        return oss.str();
    }

    bool isDirty() const { return dirty; }
    void tidy() { dirty = false; }
private:
    inline uint64_t encode_name() const {
        // pack signed into 32+32 bits
        uint64_t ux = static_cast<uint32_t>(pos.x);
        uint64_t uy = static_cast<uint32_t>(pos.y);
        return (ux << 32) | uy;
    }

    inline sf::Vector2i decode_name(uint64_t v) const {
        int x = static_cast<int32_t>(v >> 32);
        int y = static_cast<int32_t>(v & 0xffffffff);
        return {x, y};
    }
private:
    void build();
    void update_tile(int r, int c, Tile new_tile);
private:
    std::shared_ptr<sf::Texture> tilesheet;
    sf::Vector2f offset;
    sf::Vector2i pos;
    bool dirty{false};
    std::array<Tile, CHUNK_SIZE*CHUNK_SIZE> data;
    sf::VertexArray vertices;
};