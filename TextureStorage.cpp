#include "TextureStorage.hpp"

#include <iostream>


// static members
sf::Image TextureStorage::spriteSheet;
sf::Texture TextureStorage::spriteSheetTexture;
std::array<sf::Sprite, LogicGate::LAST_ENUM*2> TextureStorage::sprites;


int TextureStorage::Init(float scale)
{
    const std::string spriteSheetPath {"LogicGateSpriteSheet.png"};
    if(!spriteSheet.loadFromFile(spriteSheetPath)) {
        std::cerr << "Failed to load image: '" << spriteSheetPath << "'\n Exiting.\n"; return 1;
    }
    if(!spriteSheetTexture.loadFromImage(spriteSheet /*, sf::IntRect(0, 0, 1024, 1024)*/)) {
        std::cout << "Failed to set texture!\n Exiting.\n"; return 2;
    }
    
    constexpr int imgsz{1024}; // square 1024x1024
    constexpr int W {imgsz/2}, H {imgsz/4}; // sprite dimensions
    
    for (int offset{0}; offset < 2; ++offset)
    {  // looping for red sprites (horizontal offset by 1024 pixels)
        for (int i{0}; i < LogicGate::LAST_ENUM; ++i) 
        {
            const int X {W*(i%2)}, Y {H*(i/2)}; // two sprites per row
            
            sf::Sprite& sprite = sprites[i+int(offset*LogicGate::LAST_ENUM)];
            sprite = sf::Sprite{spriteSheetTexture, sf::IntRect(X+(imgsz*offset), Y, W, H)};
            sprite.setScale(scale, scale);
            #ifdef SELECTORWINDOW_DEBUG
              const float xOffset{float(imgsz*offset)/4.f}; // division by four is required because window width (and h-scaling) also doubles
            #else
              const float xOffset{float(imgsz*offset)};
            #endif
            sprite.setPosition(X*scale + xOffset, Y*scale);
        }
    }
    
    
    return 0;
}


