#include "chunk.h"
#include "../game.h"

#include <cmath>

Chunk::Chunk(std::array<Tile, CHUNK_SIZE*CHUNK_SIZE> const& data, sf::Vector2i pos)
    : data(data), tilesheet(Game::getInstance()->getTextureManager().get("tilesheet")),
      vertices(sf::PrimitiveType::Triangles, CHUNK_SIZE*CHUNK_SIZE*6) {
    offset = sf::Vector2f(
        static_cast<float>(pos.x * CHUNK_SIZE * TILE_SIZE),
        static_cast<float>(pos.y * CHUNK_SIZE * TILE_SIZE)
    );

    tilesheet->setSmooth(false); // disables linear filtering
    build();
}

Chunk::~Chunk() {

}

void Chunk::build() {
    const float epsilon = 0.01f;

    for (int r = 0; r < CHUNK_SIZE; ++r) {
        for (int c = 0; c < CHUNK_SIZE; ++c) {
            int idx = r * CHUNK_SIZE + c;
            Tile tile = data[idx];
            sf::Vertex* quad = &vertices[idx * 6];

            // Pixel-perfect positions
            float x = std::floor(offset.x + c * TILE_SIZE);
            float y = std::floor(offset.y + r * TILE_SIZE);

            // First triangle
            quad[0].position = {x, y};
            quad[1].position = {x + TILE_SIZE, y};
            quad[2].position = {x, y + TILE_SIZE};

            // Second triangle
            quad[3].position = {x + TILE_SIZE, y};
            quad[4].position = {x + TILE_SIZE, y + TILE_SIZE};
            quad[5].position = {x, y + TILE_SIZE};

            // Texture coordinates with epsilon padding
            int tx = static_cast<int>(tile) * static_cast<int>(TILE_SIZE);
            int ty = 0;

            quad[0].texCoords = {tx + epsilon, ty + epsilon};
            quad[1].texCoords = {tx + TILE_SIZE - epsilon, ty + epsilon};
            quad[2].texCoords = {tx + epsilon, ty + TILE_SIZE - epsilon};
            quad[3].texCoords = {tx + TILE_SIZE - epsilon, ty + epsilon};
            quad[4].texCoords = {tx + TILE_SIZE - epsilon, ty + TILE_SIZE - epsilon};
            quad[5].texCoords = {tx + epsilon, ty + TILE_SIZE - epsilon};
        }
    }
}

void Chunk::update_tick(sf::Vector2f mouse_coords) {
    sf::Vector2f local = mouse_coords - offset;

    int c = static_cast<int>(local.x / TILE_SIZE);
    int r = static_cast<int>(local.y / TILE_SIZE);

    if (c < 0 || c >= CHUNK_SIZE || r < 0 || r >= CHUNK_SIZE)
        return;

    update_tile(r, c, Tile::Water);
}

void Chunk::update_tile(int r, int c, Tile new_tile) {
    data[r * CHUNK_SIZE + c] = new_tile;
    const float epsilon = 0.01f;
    int idx = r * CHUNK_SIZE + c;
    sf::Vertex* quad = &vertices[idx * 6];

    int tx = static_cast<int>(new_tile) * static_cast<int>(TILE_SIZE);
    int ty = 0;

    quad[0].texCoords = {tx + epsilon, ty + epsilon};
    quad[1].texCoords = {tx + TILE_SIZE - epsilon, ty + epsilon};
    quad[2].texCoords = {tx + epsilon, ty + TILE_SIZE - epsilon};
    quad[3].texCoords = {tx + TILE_SIZE - epsilon, ty + epsilon};
    quad[4].texCoords = {tx + TILE_SIZE - epsilon, ty + TILE_SIZE - epsilon};
    quad[5].texCoords = {tx + epsilon, ty + TILE_SIZE - epsilon};
}

void Chunk::render(sf::RenderTarget& target) {
    sf::RenderStates states;
    states.texture = tilesheet.get();
    target.draw(vertices, states);
}
