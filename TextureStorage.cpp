#include "TextureStorage.hpp"

#include <iostream>


// static members
sf::Image TextureStorage::spriteSheet;
sf::Texture TextureStorage::spriteSheetTexture;
std::array<sf::Sprite, LogicGate::LAST_ENUM> TextureStorage::sprites;


int TextureStorage::Init(float scale)
{
    const std::string spriteSheetPath {"LogicGateSpriteSheet_Labeled.png"};
    if(!spriteSheet.loadFromFile(spriteSheetPath)) {
        std::cerr << "Failed to load image: '" << spriteSheetPath << "'\n Exiting.\n"; return 1;
    }
    if(!spriteSheetTexture.loadFromImage(spriteSheet /*, sf::IntRect(0, 0, 1024, 1024)*/)) {
        std::cout << "Failed to set texture!\n Exiting.\n"; return 2;
    }
    
    for (int i{0}; i < LogicGate::LAST_ENUM; ++i) 
    {
        constexpr int W {512}, H {256}; // 1024x1024
        const int X {W*(i%2)}, Y {H*(i/2)};
        
        sf::Sprite& sprite = sprites[i];
        sprite = sf::Sprite{spriteSheetTexture, sf::IntRect(X, Y, W, H)};
        sprite.setScale(scale, scale);
        sprite.setPosition(X*scale, Y*scale);
    }
    
    return 0;
}


