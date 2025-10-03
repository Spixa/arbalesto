#pragma once

#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>

#include "static.h"

constexpr uint8_t CHUNK_SIZE = 16;
constexpr float TILE_SIZE = 32.f;

class TileHighlight : public sf::Drawable, public sf::Transformable {
    sf::RectangleShape rect;
    sf::Text pos_text;
public:
    TileHighlight(sf::Font& font);

    void update(sf::Vector2f mouse);
protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        states.transform *= getTransform();
        target.draw(rect, states);
        target.draw(pos_text, states);
    }
};

enum class Tile: uint32_t {
    Grass = 0,
    Water = 1,
    Wood = 2,
    Cobble = 3,
};

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

    bool isSolidTile(int r, int c) const {
        if (r < 0 || r >= CHUNK_SIZE || c < 0 || c >= CHUNK_SIZE)
            return false;

        // Tile-level solidity
        if (data[r * CHUNK_SIZE + c] == Tile::Cobble)
            return true;

        // Static objects
        for (auto const& obj : objects) {
            sf::Vector2i start = obj.origin;
            sf::Vector2i end = obj.origin + obj.size;
            if (c >= start.x && c < end.x && r >= start.y && r < end.y && obj.solid)
                return true;
        }
        return false;
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

    bool load();
    bool save() {
        auto path = getFile();
        std::ofstream file(path, std::ios::binary);
        if (!file) return false;

        file.write(reinterpret_cast<const char*>(data.data()), CHUNK_SIZE * CHUNK_SIZE * sizeof(Tile));

        uint32_t count = static_cast<uint32_t>(objects.size());
        file.write(reinterpret_cast<const char*>(&count), sizeof(count));

        // write each object
        for (auto const& obj : objects) {
            file.write(reinterpret_cast<const char*>(&obj.type), sizeof(obj.type));
            file.write(reinterpret_cast<const char*>(&obj.origin), sizeof(obj.origin));
            file.write(reinterpret_cast<const char*>(&obj.size), sizeof(obj.size));
            file.write(reinterpret_cast<const char*>(&obj.solid), sizeof(obj.solid));
            file.write(reinterpret_cast<const char*>(&obj.emit), sizeof(obj.emit));
            file.write(reinterpret_cast<const char*>(&obj.light_radius), sizeof(obj.light_radius));
            file.write(reinterpret_cast<const char*>(&obj.atlas_rect), sizeof(obj.atlas_rect));
        }

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
    void placeStatic(StaticObjectType type, sf::Vector2i worldTile, sf::Vector2i size, bool solid, bool emit, float light_radius);
    bool breakStatic(sf::Vector2i ctile);
    void rebuildStaticVerts();

    void collectLights(std::vector<StaticObject*>& lightsOut);
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
    sf::VertexArray static_vertices;
    std::vector<StaticObject> objects;
    std::shared_ptr<sf::Texture> object_atlas;
    bool init{false};
    sf::RectangleShape border;
};