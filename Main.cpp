#include <iostream>
#include <format>

#include <SFML/Window.hpp> //sf::Event

#include "LogicGate.hpp"
#include "TextureStorage.hpp"
#include "SelectorWindow.hpp"
#include "Interactives.hpp"


//create a component for each gate on startup and validate pincount
#define CHECK_COMPONENT_PINCOUNT
//#undef CHECK_COMPONENT_PINCOUNT


bool usingVsync {false};
extern constexpr unsigned int framerateCap{300};
extern const sf::Color backgroundColor{0x999999FF};
constexpr float spriteScale = 0.25f;


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
    mainWindow.setPosition({2600, 0});
    
    int status = TextureStorage::Init(spriteScale);
    if(status != 0) { return status; }
    
    SelectorWindow selectorWindow (spriteScale);
    
    sf::Sprite heldSprite = TextureStorage::GetSprite(selectorWindow.selection);
    std::vector<Component> components;
    components.reserve(127);
    Component* selectedComponent{nullptr};
    
    #ifdef CHECK_COMPONENT_PINCOUNT
      bool tooManyInputs{false};
      for (int i{1}; i < LogicGate::LAST_ENUM; ++i) {
          Component& component = components.emplace_back(LogicGate::OpType(i));
          component.SetPosition(i*96, i*64);
          std::cout << component.Name() << std::format(": {} inputs", component.GetPinCount()) << '\n';
          if(component.GetPinCount() > 2) tooManyInputs = true;
          
          // staggered
          Component& componentTwo = components.emplace_back(LogicGate::OpType(i));
          componentTwo.SetPosition(i*96, ((i-1)*64 + 512));
      }
      if(tooManyInputs) { std::cerr << "\nError: #Inputs > 2 \n Exiting.\n"; return 3; }
      std::cout << "\n";
    #endif
    
    MakeGlobalIO(components, true, std::vector<bool>{ true, false, /*true, false, true, false, true, false,*/ } );
    MakeGlobalIO(components, false, {});
    
    {
        std::vector<Component> InputBits{};
        for(const Component& component: components) { if(component.isGlobalIn) InputBits.push_back(component); }
        std::cout << "\nGlobal Input = " << ReadIO(InputBits ) << "\n\n";
    }
    
    // printing truth tables
    #define EVALTEST(a, b) std::cout << std::boolalpha << \
    std::format( "  ({}, {}): {}\n", a, b, LogicGate::Eval(LogicGate::OpType(i), a, b) );
    
    for (int i{2}; i < LogicGate::LAST_ENUM; ++i) 
    {
        std::cout << LogicGate::GetName(LogicGate::OpType(i)) << "\n";
        EVALTEST(true, true);
        EVALTEST(true, false);
        EVALTEST(false, true);
        EVALTEST(false, false);
        std::cout << '\n';
    }
    #undef EVALTEST
    
    
    while (mainWindow.isOpen())
    {
        if (selectorWindow.isOpen()) {
            selectorWindow.EventLoop();
            selectorWindow.Redraw();
            if (selectorWindow.selectionHasChanged) {
                heldSprite = TextureStorage::GetSprite(selectorWindow.selection);
                selectorWindow.selectionHasChanged = false;
            }
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
                        
                        case sf::Keyboard::Tilde:
                            selectorWindow.setVisible(true);
                            selectorWindow.requestFocus();
                        break;
                        
                        // maps numpad/numrow inputs
                        #define CASE_KEYNUM_OPTYPE(N) \
                        case sf::Keyboard::Numpad##N: \
                        case sf::Keyboard::Num##N:    \
                            selectorWindow.SetSelection(LogicGate::OpType(N)); \
                        break;
                        
                        case sf::Keyboard::Escape:
                            CASE_KEYNUM_OPTYPE(0);
                            CASE_KEYNUM_OPTYPE(1);
                            CASE_KEYNUM_OPTYPE(2);
                            CASE_KEYNUM_OPTYPE(3);
                            CASE_KEYNUM_OPTYPE(4);
                            CASE_KEYNUM_OPTYPE(5);
                            CASE_KEYNUM_OPTYPE(6);
                            CASE_KEYNUM_OPTYPE(7);
                        static_assert(7 < LogicGate::OpType::LAST_ENUM);
                        #undef CASE_KEYNUM_OPTYPE
                        
                        case sf::Keyboard::Space:
                        {
                            for(Component& component: components) { component.PropagateLogic(); }
                            
                            {
                                std::vector<Component> OutputBits{};
                                for(const Component& component: components) 
                                { if(component.isGlobalOut) OutputBits.push_back(component); }
                                std::cout << "\nGlobal Output = " << ReadIO(OutputBits) << "\n\n";
                            }
                        }
                        break;
                        
                        default: break;
                    }
                }
                break;
                
                case sf::Event::MouseWheelScrolled:
                    selectorWindow.SetSelection(
                        selectorWindow.NextSelection((event.mouseWheelScroll.delta >= 0), false)
                    );
                break;
                
                case sf::Event::MouseButtonPressed:
                    if (selectorWindow.selection > 0) // place component if it's not 'EQ'
                    { components.emplace_back(selectorWindow.selection, heldSprite); }
                    else
                    {
                        bool hitboxFound{false};
                        std::string identifier;
                        const sf::Vector2f mousePosition{ sf::Mouse::getPosition(mainWindow) };
                        
                        for (Component& component: components)
                        {
                            if (component.isOutputPinClicked(mousePosition)) {
                                identifier = std::format("{} output-pin", component.UUID());
                                hitboxFound = true; selectedComponent = &component;
                                component.HighlightOutputPin(); break;
                            } else if(component.inputHitboxClicked(mousePosition)) {
                                identifier = std::format("{} input-pin", component.UUID());
                                hitboxFound = true; selectedComponent = nullptr; break;
                            } else if(component.ContainsCoord(mousePosition)) {
                                identifier = std::format("{}", component.UUID());
                                hitboxFound = true; selectedComponent = nullptr; break;
                            }
                        }
                        if (!hitboxFound) identifier = "empty click";
                        std::cout << std::format("{} @({}, {})", identifier, mousePosition.x, mousePosition.y);
                        if (!selectedComponent) std::cout << '\n';
                    }
                break;
                
                case sf::Event::MouseButtonReleased:
                {
                    if (selectorWindow.selection != LogicGate::OpType::EQ) break;
                    if (!selectedComponent) break; // only output pins can be routed to input
                    
                    bool hitboxFound{false};
                    const sf::Vector2f mousePosition{ sf::Mouse::getPosition(mainWindow) };
                    for (Component& component: components) {
                        if (component.inputHitboxClicked(mousePosition)) {
                            std::cout << std::format(" -> {} input-pin @({}, {})\n",
                                component.UUID(), mousePosition.x, mousePosition.y);
                            hitboxFound = true;
                            selectedComponent->CreateConnection(component.getClickedInput(mousePosition));
                            selectedComponent->PropagateLogic();
                            component.PropagateLogic();
                            break;
                        }
                    }
                    if (!hitboxFound) std::cout << '\n'; // flushing held output
                    selectedComponent = nullptr;
                }
                break;
                
                default: break;
            }
        }
        
        mainWindow.clear(backgroundColor);
        
        for(const Component& component: components) { mainWindow.draw(component); }
        
        if (selectorWindow.selection > 0)
        {
            const auto&& [x, y] = sf::Mouse::getPosition(mainWindow);
            heldSprite.setPosition(x-64, y-32); // offsets to center it
            mainWindow.draw(heldSprite);
        }
        
        mainWindow.display();
    }
    
    return 0;
}
