#ifndef CIRCUITSIM_TEXTURESTORAGE_HPP
#define CIRCUITSIM_TEXTURESTORAGE_HPP

#include <array>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "LogicGate.hpp"


struct TextureStorage
{
    static sf::Image spriteSheet;
    static sf::Texture spriteSheetTexture;
    static std::array<sf::Sprite, LogicGate::LAST_ENUM> sprites;
    
    static sf::Sprite GetSprite(LogicGate::OpType T) { return sprites[T]; }
    static int Init(float scale=1.f); // returns non-zero on failure
};


#endif
