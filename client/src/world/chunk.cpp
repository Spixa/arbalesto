#include "chunk.h"
#include "../game.h"

#include <cmath>
#include "magic_enum.hpp"

TileHighlight::TileHighlight(sf::Font& font) : pos_text(font) {
    rect.setSize({TILE_SIZE, TILE_SIZE});
    rect.setFillColor(sf::Color::Transparent);
    rect.setOutlineColor(sf::Color::Yellow);
    rect.setOutlineThickness(.7f);

    pos_text.setCharacterSize(72.f);
    pos_text.setFillColor(sf::Color::Yellow);
    pos_text.setPosition({0, -12});
    pos_text.setScale({0.125, 0.125});
}

void TileHighlight::update(sf::Vector2f mouse) {
    auto w = Game::getInstance()->getWorld();
    auto wt = w->worldToTileCoords(mouse);
    setPosition(w->tileToWorldCoords(wt));

    std::stringstream ss;
    auto t = w->resolve(wt);
    if (t.valid())
        ss << "(" << wt.x << ", " << wt.y << ") " << magic_enum::enum_name(t.chunk->getTile(t.ly, t.lx));
    pos_text.setString(ss.str());
}

Chunk::Chunk(std::array<Tile, CHUNK_SIZE*CHUNK_SIZE> const& data, sf::Vector2i pos)
    : data(data), tilesheet(Game::getInstance()->getTextureManager().get("tilesheet")), pos(pos), object_atlas(Game::getInstance()->getTextureManager().get("obj_atlas"))
      , vertices(sf::PrimitiveType::Triangles, CHUNK_SIZE*CHUNK_SIZE*6), static_vertices(sf::PrimitiveType::Triangles) {
    offset = sf::Vector2f(
        static_cast<float>(pos.x * CHUNK_SIZE * TILE_SIZE),
        static_cast<float>(pos.y * CHUNK_SIZE * TILE_SIZE)
    );

    border.setPosition(offset);
    border.setSize({CHUNK_SIZE * TILE_SIZE, CHUNK_SIZE * TILE_SIZE});
    border.setOutlineColor(sf::Color::Yellow);
    border.setOutlineThickness(.5f);
    border.setFillColor(sf::Color::Transparent);

    tilesheet->setSmooth(false); // disables linear filtering
    build();
}

Chunk::~Chunk() {

}

bool Chunk::load() {
    auto path = getFile();
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }

    file.read(reinterpret_cast<char*>(data.data()), CHUNK_SIZE * CHUNK_SIZE * sizeof(Tile));
    build();

    objects.clear();
    uint32_t count = 0;
    if (file.read(reinterpret_cast<char*>(&count), sizeof(count))) {
        objects.resize(count);
        for (uint32_t i = 0; i < count; ++i) {
            file.read(reinterpret_cast<char*>(&objects[i].type), sizeof(objects[i].type));
            file.read(reinterpret_cast<char*>(&objects[i].origin), sizeof(objects[i].origin));
            file.read(reinterpret_cast<char*>(&objects[i].size), sizeof(objects[i].size));
            file.read(reinterpret_cast<char*>(&objects[i].solid), sizeof(objects[i].solid));
            file.read(reinterpret_cast<char*>(&objects[i].emit), sizeof(objects[i].emit));
            file.read(reinterpret_cast<char*>(&objects[i].light_radius), sizeof(objects[i].light_radius));
            file.read(reinterpret_cast<char*>(&objects[i].atlas_rect), sizeof(objects[i].atlas_rect));
        }
    }
    return file.good();
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

void Chunk::update_tick(sf::Vector2f mouse_coords, Tile selected) {
    sf::Vector2f local = mouse_coords - offset;

    int c = static_cast<int>(local.x / TILE_SIZE);
    int r = static_cast<int>(local.y / TILE_SIZE);

    if (c < 0 || c >= CHUNK_SIZE || r < 0 || r >= CHUNK_SIZE)
        return;

    update_tile(r, c, selected);
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

    dirty = true;
}

void Chunk::placeStatic(StaticObjectType type, sf::Vector2i worldTile, sf::Vector2i size, bool solid, bool emit, float light_radius) {
    // Convert world to local tile coords
    sf::Vector2i local = worldTile - sf::Vector2i(pos.x * CHUNK_SIZE, pos.y * CHUNK_SIZE);
    if (local.x < 0 || local.x >= CHUNK_SIZE || local.y < 0 || local.y >= CHUNK_SIZE)
        return; // out of bounds

    for (auto const& o: objects) {
        if (o.origin == local) {
            return;
        }
    }

    StaticObject obj;
    obj.origin = local;
    obj.size = size;
    obj.type = type;
    obj.solid = solid;
    obj.emit = emit;
    obj.light_radius = light_radius;

    objects.push_back(obj);
    rebuildStaticVerts();
    dirty = true;

    // Add light if needed
    if (emit && Game::getInstance()->getWorld()) {
        Game::getInstance()->getWorld()->rebakeLighting();
    }
}

bool Chunk::breakStatic(sf::Vector2i lt) {
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        sf::Vector2i start = it->origin;
        sf::Vector2i end = it->origin + it->size;
        if (lt.x >= start.x && lt.x < end.x &&
            lt.y >= start.y && lt.y < end.y) {
            objects.erase(it);
            rebuildStaticVerts();
            dirty = true;

            if (Game::getInstance()->getWorld())
                Game::getInstance()->getWorld()->rebakeLighting();

            return true;
        }
    }
    return false;
}

void Chunk::rebuildStaticVerts() {
    static_vertices.clear();
    for (auto const& obj : objects) {
        sf::Vector2f leftTop = offset + sf::Vector2f(obj.origin.x * TILE_SIZE, obj.origin.y * TILE_SIZE);
        sf::Vector2f rightBottom = leftTop + sf::Vector2f(obj.size.x * TILE_SIZE, obj.size.y * TILE_SIZE);

        // First triangle
        static_vertices.append({{leftTop.x, leftTop.y}, sf::Color::White});
        static_vertices.append({{rightBottom.x, leftTop.y}, sf::Color::White});
        static_vertices.append({{leftTop.x, rightBottom.y}, sf::Color::White});

        // Second triangle
        static_vertices.append({{rightBottom.x, leftTop.y}, sf::Color::White});
        static_vertices.append({{rightBottom.x, rightBottom.y}, sf::Color::White});
        static_vertices.append({{leftTop.x, rightBottom.y}, sf::Color::White});
    }
}

void Chunk::collectLights(std::vector<StaticObject*>& lightsOut) {
    for (auto& obj : objects) {
        if (obj.emit)
            lightsOut.push_back(&obj);
    }
}


void Chunk::render(sf::RenderTarget& target) {
    sf::RenderStates states;
    states.texture = tilesheet.get();
    target.draw(vertices, states);

    for (auto const& obj : objects) {
        sf::Sprite sprite{*object_atlas};

        switch (obj.type) {
            case StaticObjectType::Torch: {
                sprite.setTextureRect({{0, 0}, {32, 32}});
            } break;
            case StaticObjectType::WoodenDoor: {
                sprite.setTextureRect({{32, 0}, {32, 32}});
            } break;
            case StaticObjectType::Fence: {
                sprite.setTextureRect({{64, 0}, {32, 32}});
            } break;
        }

        sprite.setPosition({
            offset.x + obj.origin.x * TILE_SIZE,
            offset.y + obj.origin.y * TILE_SIZE
        });
        target.draw(sprite);
    }
}
