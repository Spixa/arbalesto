#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <unordered_map>
#include <memory>
#include <iostream>

#include "logger.hpp"

enum class ResourceType : int {
    TextureResource = 1,
    SoundResource = 2,
    ValueResource = 3,
    NoneResource = -1
};


struct ResourceMeta {
    ResourceMeta(std::string name, ResourceType type) : name(name), type(type) {
        log("res").trace("New meta \"" + name + "\" with typeid=" + std::to_string(static_cast<int>(type)));
    }

    std::string name;
    ResourceType type = ResourceType::NoneResource;
    bool operator==(ResourceMeta const& other) const {
        return name == other.name && type == other.type;
    }
};

struct hash_fn
{
    std::size_t operator() (const ResourceMeta &node) const
    {
        std::size_t h1 = std::hash<std::string>()(node.name);
        std::size_t h2 = std::hash<int>()(static_cast<int>(node.type));

        return h1 ^ h2;
    }
};

class Resource {
public:
    Resource(ResourceMeta const& meta) : meta(meta) {
        if (meta.type == ResourceType::NoneResource) {
            log("res").error("Cannot construct resource with meta None");
            exit(0);
        }

        log("res").trace("New resource created");
        log("res").trace("Meta consumed and pushed resource to lookup vector");

        texture = std::make_shared<sf::Texture>();
    }

    ~Resource() {}

    std::shared_ptr<sf::Texture> getTexture() {
        if (meta.type == ResourceType::TextureResource) {
            return texture;
        } else {
            log("res").error("Cannot get texture on resource that is not of type texture");
            exit(0);
        }
    }

private:
    union {
        std::shared_ptr<sf::Texture> texture;
        std::shared_ptr<sf::SoundBuffer> sound;
        int integer;
        std::string string;
    };

    ResourceMeta meta;
};

class ResourceManager {
public:
    ResourceManager() {
        log("res").info("Resource manager has begun.");
    }

    void push(ResourceMeta const& meta) {
        std::shared_ptr<Resource> res = std::make_shared<Resource>(meta);
        resources.insert( std::make_pair(meta, std::move(res)));
    }

    std::shared_ptr<Resource> get(ResourceMeta const& meta) {
        auto it = resources.find(meta);

        return it->second;
    } 
private:
    std::unordered_map<ResourceMeta, std::shared_ptr<Resource>, hash_fn> resources;
};