#ifndef CIRCUITSIM_TEXTURESTORAGE_HPP
#define CIRCUITSIM_TEXTURESTORAGE_HPP

#include <array>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "LogicGate.hpp"


struct TextureStorage
{
    static sf::Image spriteSheet;
    static sf::Image spriteSheetRed;
    static sf::Texture spriteSheetTexture;
    static sf::Texture spriteSheetTextureRed;
    static std::array<sf::Sprite, LogicGate::LAST_ENUM> sprites;
    static std::array<sf::Sprite, LogicGate::LAST_ENUM> spritesRed;
    
    static sf::Sprite GetSprite(LogicGate::OpType T, bool isRed=false) { return (isRed? spritesRed[T] : sprites[T]); }
    static int Init(float scale=1.f); // returns non-zero on failure
};


#endif
