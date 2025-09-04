#pragma once

#include <SFML/Graphics.hpp>

constexpr uint8_t CHUNK_SIZE = 16;

enum class Tile: int {
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

    void from_data(std::array<Tile, CHUNK_SIZE*CHUNK_SIZE> const& data) {
        this->data = data;
        build();
    }
    void update_tick(sf::Vector2f mouse_coords);
    void render(sf::RenderTarget& target);
private:
    void build();
    void update_tile(int r, int c, Tile new_tile);
private:
    std::shared_ptr<sf::Texture> tilesheet;
    sf::Vector2f offset;
    std::array<Tile, CHUNK_SIZE*CHUNK_SIZE> data;
    sf::VertexArray vertices;
};