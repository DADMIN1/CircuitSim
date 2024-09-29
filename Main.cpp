#include <iostream>
#include <format>

#include <SFML/Window.hpp> //sf::Event

#include "LogicGate.hpp"
#include "TextureStorage.hpp"
#include "SelectorWindow.hpp"


bool usingVsync {false};
extern constexpr unsigned int framerateCap{300};
extern const sf::Color backgroundColor{0x999999FF};


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
    
    sf::RenderWindow mainWindow (sf::VideoMode(1024, 1024), "Circuit Simulator", sf::Style::Close);
    mainWindow.setFramerateLimit(framerateCap);
    mainWindow.setVerticalSyncEnabled(usingVsync);
    
    int status = TextureStorage::Init(0.25f);
    if(status != 0) { return status; }
    
    SelectorWindow selectorWindow (0.25f);
    
    for (int i{2}; i < LogicGate::LAST_ENUM; ++i) 
    {
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
    
    
    while (mainWindow.isOpen())
    {
        if (selectorWindow.isOpen()) {
            selectorWindow.EventLoop();
            selectorWindow.Redraw();
        }
        
        sf::Event event;
        while(mainWindow.pollEvent(event))
        {
            switch(event.type)
            {
                case sf::Event::Closed:
                    mainWindow.close();
                break;
                
                case sf::Event::KeyPressed:
                {
                    switch(event.key.code)
                    {
                        case sf::Keyboard::Q:
                            mainWindow.close();
                        break;
                        
                        default: break;
                    }
                }
                break;
                
                default: break;
            }
        }
        
        mainWindow.clear(backgroundColor);
        mainWindow.display();
    }
    
    return 0;
}
