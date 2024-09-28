#include <iostream>
#include <format>

#include <SFML/Graphics.hpp> // RenderWindow

#include "LogicGate.hpp"


constexpr unsigned int framerateCap {300};
bool usingVsync {false};


void PrintProgramConfiguration()
{
    #define PRINT(a) std::cout << std::format( #a ": {}\n", a );
    #define PRINTTWO(a, b) std::cout << std::format( #a ": {}  " #b ": {}\n", a, b);
    
    std::cout << '\n';
    
    std::cout << "Maximum texture size: " << sf::Texture::getMaximumSize() << " pixels\n";
    PRINT(usingVsync);
    PRINT(framerateCap);
    
    std::cout << '\n';
    
    return;
    #undef PRINT
    #undef PRINTTWO
}


int main(int argc, char** argv)
{
    std::cout << "Circuit Simulator\n";
    
    for (int C{0}; C < argc; ++C) {
        std::string arg {argv[C]};
        std::cout << "C: " << C << " \t arg: " << arg << '\n';
    }
    
    PrintProgramConfiguration();
    
    sf::RenderWindow mainwindow (sf::VideoMode(1024, 1024), "Circuit Simulator", sf::Style::Default);
    mainwindow.setFramerateLimit(framerateCap);
    mainwindow.setVerticalSyncEnabled(usingVsync);
    
    sf::Image spriteSheet;
    const std::string spriteSheetPath {"LogicGateSpriteSheet_Labeled.png"};
    if(!spriteSheet.loadFromFile(spriteSheetPath)) {
        std::cerr << "Failed to load image: '" << spriteSheetPath << "'\n Exiting.\n"; return 1;
    }
    
    sf::Texture spriteSheetTexture;
    if(!spriteSheetTexture.loadFromImage(spriteSheet /*, sf::IntRect(0, 0, 1024, 1024)*/)) {
        std::cout << "Failed to set texture!\n Exiting.\n"; return 2;
    }
    
    std::array<sf::Sprite, LogicGate::LAST_ENUM> sprites;
    for (int i{0}; i < LogicGate::LAST_ENUM; ++i) 
    {
        constexpr int W {512}, H {256}; // 1024x1024
        const int X {W*(i%2)}, Y {H*(i/2)};
        sf::Sprite& sprite = sprites[i];
        sprite = sf::Sprite {spriteSheetTexture, sf::IntRect(X, Y, W, H)};
        sprite.setPosition(X, Y);
        //sprite.setScale(0.25f, 0.25f);
        
        if(i<2) continue; // skip truth-table for unary operators
        std::cout << LogicGate::GetName(LogicGate::OpType(i)) << "\n";
        
        #define EVALTEST(a, b) std::cout << std::boolalpha << \
            std::format( "  ({}, {}): {}\n", a, b, LogicGate::Eval(LogicGate::OpType(i), a, b) );
        
        EVALTEST(true, true);
        EVALTEST(true, false);
        EVALTEST(false, true);
        EVALTEST(false, false);
        
        #undef EVALTEST
        std::cout << '\n';
    }
    
    while (mainwindow.isOpen())
    {
        sf::Event event;
        while(mainwindow.pollEvent(event))
        {
            switch(event.type)
            {
                case sf::Event::Closed:
                    mainwindow.close();
                break;
                
                case sf::Event::KeyPressed:
                {
                    switch(event.key.code)
                    {
                        case sf::Keyboard::Q:
                            mainwindow.close();
                        break;
                        
                        default: break;
                    }
                }
                break;
                
                default: break;
            }
        }
        
        //mainwindow.clear(sf::Color::Transparent);
        mainwindow.clear(sf::Color(0x999999FF));
        for (const sf::Sprite& sprite: sprites) {
            mainwindow.draw(sprite);
        }
        
        mainwindow.display();
    }
    
    return 0;
}
