#include <iostream>
#include <format>

#include <SFML/Graphics.hpp> // RenderWindow
//#include <SFML/Window.hpp> // defines sf::Event


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
    const std::string spriteSheetPath {"LogicGateSpriteSheet_1024.png"};
    if(!spriteSheet.loadFromFile(spriteSheetPath)) {
        std::cerr << "Failed to load image: '" << spriteSheetPath << "'\n Exiting.\n"; return 1;
    }
    
    sf::Texture spriteSheetTexture;
    if(!spriteSheetTexture.loadFromImage(spriteSheet /*, sf::IntRect(0, 0, 1024, 1024)*/)) {
        std::cout << "Failed to set texture!\n Exiting.\n"; return 2;
    }
    
    std::array<sf::Sprite, 8> sprites;
    for (int i{0}; i < 8; ++i) {
        //constexpr int W {128}, H {64};
        constexpr int W {512}, H {256};
        const int X {W*(i%2)}, Y {H*(i/2)};
        sf::Sprite& sprite = sprites[i];
        sprite = sf::Sprite {spriteSheetTexture, sf::IntRect(X, Y, W, H)};
        sprite.setPosition(X, Y);
        //sprite.setScale(0.5f, 0.5f);
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
