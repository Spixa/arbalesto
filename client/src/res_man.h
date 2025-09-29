#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>
#include <iostream>

// Generic template for resources with loadFromFile
template <typename T>
class ResourceManager {
public:
    std::shared_ptr<T> get(const std::string& id, const std::string& filename) {
        auto it = resources.find(id);
        if (it != resources.end()) {
            return it->second;
        }

        auto resource = std::make_shared<T>();
        if (!resource->loadFromFile(filename)) {
            throw std::runtime_error("Failed to load resource: " + filename);
        }


        std::cout << "Loaded '" << filename << "' as '" << id << "'" << std::endl;

        resources[id] = resource;
        return resource;
    }

    std::shared_ptr<T> get(const std::string& id) {
        auto it = resources.find(id);
        if (it == resources.end()) {
            throw std::runtime_error("Resource not found: " + id);
        }

        return it->second;
    }

private:
    std::unordered_map<std::string, std::shared_ptr<T>> resources;
};

// Specialization for sf::Font (uses openFromFile instead)
template <>
class ResourceManager<sf::Font> {
public:
    std::shared_ptr<sf::Font> get(const std::string& id, const std::string& filename) {
        auto it = resources.find(id);
        if (it != resources.end()) {
            return it->second;
        }

        auto resource = std::make_shared<sf::Font>();
        if (!resource->openFromFile(filename)) {
            throw std::runtime_error("Failed to open font: " + filename);
        }
        resource->setSmooth(true);
        resources[id] = resource;
        return resource;
    }

    std::shared_ptr<sf::Font> get(const std::string& id) {
        auto it = resources.find(id);
        if (it == resources.end()) {
            throw std::runtime_error("Font not found: " + id);
        }
        return it->second;
    }

private:
    std::unordered_map<std::string, std::shared_ptr<sf::Font>> resources;
};
