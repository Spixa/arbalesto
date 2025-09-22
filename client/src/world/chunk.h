#pragma once

#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>

constexpr uint8_t CHUNK_SIZE = 16;

enum class Tile: uint32_t {
    Grass = 0,
    Water = 1,
    Wood = 2,
    Cobble = 3,
};

constexpr float TILE_SIZE = 32.f;

class Chunk;
struct TileRef {
    Chunk* chunk;
    int lx, ly;
    bool valid() const { return chunk != nullptr; }
};


class Chunk {
public:
    Chunk(std::array<Tile, CHUNK_SIZE*CHUNK_SIZE> const& data, sf::Vector2i pos);
    virtual ~Chunk();

    bool isSolidTile(int r, int c) {
        if (r < 0 || r >= CHUNK_SIZE || c < 0 || c >= CHUNK_SIZE)
            return false;

        Tile tile = data[r * CHUNK_SIZE + c];
        return tile == Tile::Cobble;
    }

    bool isPassableTile(int r, int c) {
        if (r < 0 || r >= CHUNK_SIZE || c < 0 || c >= CHUNK_SIZE)
            return false;

        Tile tile = data[r * CHUNK_SIZE + c];
        return (tile == Tile::Cobble || tile == Tile::Water);
    }

    Tile getTile(int r, int c) const {
        return data[r*CHUNK_SIZE + c];
    }

    sf::Vector2f getOffset() const { return offset; }
    sf::Vector2i getPos() const { return pos; }

    sf::FloatRect getBounds() const {
        float size = CHUNK_SIZE * TILE_SIZE;
        return sf::FloatRect({offset.x, offset.y}, {size, size});
    }

    [[nodiscard]] bool load() {
        auto path = getFile();
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            std::cout << "[world] chunk " << path << " does not exist on disk. generating... " << std::endl;
            return false;
        }

        file.read(reinterpret_cast<char*>(data.data()), CHUNK_SIZE * CHUNK_SIZE * sizeof(Tile));
        build();

        std::cout << "[world] loaded chunk " << path << std::endl;
        return file.good();
    }

    bool save() {
        auto path = getFile();
        std::ofstream file(path, std::ios::binary);
        if (!file) return false;

        file.write(reinterpret_cast<const char*>(data.data()), CHUNK_SIZE * CHUNK_SIZE * sizeof(Tile));
        std::cout << "[world] saved chunk " << path << " onto disk" << std::endl;
        return true;
    }
    void update_tick(sf::Vector2f mouse_coords, Tile selected);
    void render(sf::RenderTarget& target);

    std::string getFile() const {
        std::ostringstream oss;
        auto code = encode_name();
        oss << "save/" << std::hex << code << ".bin";
        return oss.str();
    }
    void update_tile(int r, int c, Tile new_tile);

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
private:
    std::shared_ptr<sf::Texture> tilesheet;
    sf::Vector2f offset;
    sf::Vector2i pos;
    bool dirty{false};
    std::array<Tile, CHUNK_SIZE*CHUNK_SIZE> data;
    sf::VertexArray vertices;

    sf::RectangleShape border;
};