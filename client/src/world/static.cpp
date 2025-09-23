#include "static.h"
#include "chunk.h"

sf::FloatRect StaticObject::getWorldBounds(sf::Vector2f const& chunk_offset) const {
    return {
        {chunk_offset.x + origin.x * TILE_SIZE, chunk_offset.y +  origin.y * TILE_SIZE},
        {size.x * TILE_SIZE, size.y * TILE_SIZE}
    };
}