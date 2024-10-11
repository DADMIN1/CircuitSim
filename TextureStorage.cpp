#include "TextureStorage.hpp"

#include <iostream>


// static members
sf::Image TextureStorage::spriteSheet;
sf::Image TextureStorage::spriteSheetRed;
sf::Texture TextureStorage::spriteSheetTexture;
sf::Texture TextureStorage::spriteSheetTextureRed;
std::array<sf::Sprite, LogicGate::LAST_ENUM> TextureStorage::sprites;
std::array<sf::Sprite, LogicGate::LAST_ENUM> TextureStorage::spritesRed;


int TextureStorage::Init(float scale)
{
    const std::string spriteSheetPath {"LogicGateSpriteSheet_Labeled.png"};
    if(!spriteSheet.loadFromFile(spriteSheetPath)) {
        std::cerr << "Failed to load image: '" << spriteSheetPath << "'\n Exiting.\n"; return 1;
    }
    if(!spriteSheetTexture.loadFromImage(spriteSheet /*, sf::IntRect(0, 0, 1024, 1024)*/)) {
        std::cout << "Failed to set texture!\n Exiting.\n"; return 2;
    }
    
    const std::string spriteSheetPathRed {"LogicGateSpriteSheetRed_Labeled.png"};
    if(!spriteSheetRed.loadFromFile(spriteSheetPathRed)) {
        std::cerr << "Failed to load image: '" << spriteSheetPathRed << "'\n Exiting.\n"; return 1;
    }
    if(!spriteSheetTextureRed.loadFromImage(spriteSheetRed /*, sf::IntRect(0, 0, 1024, 1024)*/)) {
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
        
        sf::Sprite& spriteRed = spritesRed[i];
        spriteRed = sf::Sprite{spriteSheetTextureRed, sf::IntRect(X, Y, W, H)};
        spriteRed.setScale(scale, scale);
        spriteRed.setPosition(X*scale, Y*scale);
    }
    
    return 0;
}


