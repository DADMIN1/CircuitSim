#include <iostream>
#include <format>
#include <cmath> // for arc-tangent and square-root (in MouseDragLoop)

#include <SFML/Window.hpp> //sf::Event

#include "LogicGate.hpp"
#include "TextureStorage.hpp"
#include "SelectorWindow.hpp"
#include "Interactives.hpp"
#include "ComponentMap.hpp"


//create a component for each gate on startup and validate pincount
#define CHECK_COMPONENT_PINCOUNT
#undef CHECK_COMPONENT_PINCOUNT


bool usingVsync {false};
extern constexpr unsigned int framerateCap{300};
extern const sf::Color backgroundColor{0x808088FF};
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


//draws a line following the mouse while holding left-click
void MouseDragLoop(sf::RenderWindow& mainWindow, sf::Vector2f initalPosition, bool activeColor)
{
    auto [szx, szy] = mainWindow.getSize();
    sf::Texture overlayTexture{}; overlayTexture.create(szx, szy); overlayTexture.setSmooth(false);
    //overlayTexture.loadFromImage(mainWindow.capture()); //deprecated
    overlayTexture.update(mainWindow); //captures the window
    sf::Sprite overlay{overlayTexture};
    
    sf::RectangleShape dragline{};
    dragline.setPosition(initalPosition);
    dragline.setOutlineThickness(-2);
    dragline.setOutlineColor(sf::Color(0x00000077));
    dragline.setFillColor(activeColor? sf::Color(0xFF222277) : sf::Color(0xAABBFF88));
    
    while(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) 
    {
        auto nextPosition = sf::Vector2f{sf::Mouse::getPosition(mainWindow)};
        auto [dx,dy] = nextPosition-initalPosition;
        
        dragline.setSize({5.f, std::sqrt(dx*dx + dy*dy)}); //pythagorean theorem.
        dragline.setRotation(std::atan2(dy,dx)*(180.f/3.141592653f) - 90.f); // radian-to-degree conversion is 180/PI
        // -90-degrees to actually match the mouse (angle 0 is straight up, positive rotations are counter-clockwise)
        
        //mainWindow.clear(); // not actually necessary since overlay always overdraws dragline
        mainWindow.draw(overlay); // pretty cool if you disable this
        mainWindow.draw(dragline);
        mainWindow.display();
    }
    
    return;
}


int main(int argc, char** argv)
{
    std::cout << "Circuit Simulator\n";
    
    for (int C{0}; C < argc; ++C) {
        std::string arg {argv[C]};
        std::cout << "C: " << C << " \t arg: " << arg << '\n';
    }
    
    PrintProgramConfiguration();
    
    sf::ContextSettings contextSettings{}; contextSettings.antialiasingLevel = 16; // 16 is highest
    sf::RenderWindow mainWindow (sf::VideoMode(1024, 1024), "Circuit Simulator", sf::Style::Close, contextSettings);
    mainWindow.setFramerateLimit(framerateCap);
    mainWindow.setVerticalSyncEnabled(usingVsync);
    mainWindow.setPosition({2600, 0});
    
    std::cout << "antialiasing level: " << mainWindow.getSettings().antialiasingLevel << "\n\n";
    
    int status = TextureStorage::Init(spriteScale);
    if(status != 0) { return status; }
    
    SelectorWindow selectorWindow(spriteScale);
    selectorWindow.setPosition({int(mainWindow.getPosition().x + mainWindow.getSize().x + 45), 0});
    
    #ifndef _ISDEBUG
    // trying to regain keyboard/window focus from selectorWindow on startup
    mainWindow.requestFocus();
    mainWindow.setVisible(false);
    mainWindow.setVisible(true);
    mainWindow.requestFocus();
    #endif
    
    sf::Sprite heldSprite = TextureStorage::GetSprite(selectorWindow.selection);
    ComponentMap components{};
    Component* selectedComponent{nullptr};
    
    #ifdef CHECK_COMPONENT_PINCOUNT
      bool tooManyInputs{false};
      for (int i{1}; i < LogicGate::LAST_ENUM; ++i) {
          Component& component = components.Push(LogicGate::OpType(i));
          component.SetPosition(i*96, i*64);
          std::cout << component.Name() << std::format(": {} inputs", component.GetPinCount()) << '\n';
          if(component.GetPinCount() > 2) tooManyInputs = true;
          
          // staggered
          Component& componentTwo = components.Push(LogicGate::OpType(i));
          componentTwo.SetPosition(i*96, ((i-1)*64 + 512));
      }
      if(tooManyInputs) { std::cerr << "\nError: #Inputs > 2 \n Exiting.\n"; return 3; }
      std::cout << "\n";
    #endif
    
    std::vector<Component> globalInputs{};
    std::vector<Component> globalOutput{};
    MakeGlobalIO(globalInputs, true, std::vector<bool>{ true, true, true, false, false, true, false, false, } );
    MakeGlobalIO(globalOutput, false, {});
    std::cout << "\nGlobal Input = " << ReadIO(globalInputs) << "\n\n";
    
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
    
    using BankT = std::vector<Component*>;
    std::vector<BankT> banks {
        components.AddBank(LogicGate::OR,  4),
        components.AddBank(LogicGate::XOR, 2),
    };
    
    // linking banks together, pairs from prev layer into each new one
    BankT* prev = nullptr;
    for (int bankIndex{0}; bankIndex < static_cast<int>(banks.size()); ++bankIndex) 
    {
        std::cout << "\nbank #" << bankIndex << '\n';
        BankT& bank = banks[bankIndex];
        
        int I{0};
        for (Component* component: bank)
        {
            std::cout << component->UUID() << '\n';
            if (!prev) continue;
            assert(I < static_cast<int>(prev->size()));
            
            for (int K{0}; K < 2; ++K) 
            {
                if((component->gate.mType == LogicGate::NOT) && (K > 0)) break;
                Pin* pin = &component->inputs[K];
                std::cout << "  " << prev->at(I+K)->UUID() << " -> " << pin->UUID << '\n';
                prev->at(I+K)->CreateConnection(component, pin);
            }
            I = ((I+2) % prev->size());
        }
        
        prev = &bank;
    }
    
    // linking to global inputs
    assert(banks.size() > 0);
    std::cout << "\nGLOBAL INPUTS\n";
    BankT& firstBank = banks[0];
    std::size_t I{0};
    for (Component* component: firstBank) {
        assert(I < globalInputs.size());
        for (int K{0}; K < 2; ++K) {
            if((component->gate.mType == LogicGate::NOT) && (K > 0)) break;
            Pin* pin = &component->inputs[K];
            std::cout << "  " << globalInputs.at(I+K).UUID() << " -> " << pin->UUID << '\n';
            globalInputs.at(I+K).CreateConnection(component, pin);
            globalInputs.at(I+K).PropagateLogic();
            component->PropagateLogic();
        }
        if(component->gate.mType == LogicGate::NOT) { ++I; continue; }
        I = ((I+2) % globalInputs.size());
    }
    
    // linking to global outputs
    std::cout << "\nGLOBAL OUTPUTS\n";
    BankT& lastBank{banks[banks.size()-1]};
    const std::size_t numOutputs { (lastBank.size() <= globalOutput.size())? lastBank.size() : globalOutput.size()};
    for (std::size_t I{0}; I < numOutputs; ++I)
    {
        Component& component = globalOutput.at(I);
        Pin* pin = &component.inputs[0];
        std::cout << "  " << lastBank.at(I)->UUID() << " -> " << pin->UUID << '\n';
        lastBank.at(I)->CreateConnection(&component, pin);
        lastBank.at(I)->PropagateLogic();
    }
    std::cout << "\n\n";
    
    
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
                            for(Component& component: globalInputs) { component.PropagateLogic(); }
                            components.ForEach([](Component& component) { component.PropagateLogic(); });
                            for(Component& component: globalOutput) { component.PropagateLogic(); }
                            
                            {
                                // don't reprint output if it hasn't changed
                                static int prevResult {-1};
                                const int result = ReadIO(globalOutput);
                                if (result == prevResult) break;
                                std::cout << "\nGlobal Output = " << result << "\n\n";
                                prevResult = result;
                            }
                        }
                        break;
                        
                        case sf::Keyboard::H:
                            Pin::displayHitboxes = !Pin::displayHitboxes;
                            std::cout << "\npins' hitboxes: " << (Pin::displayHitboxes? "shown" : "hidden") << "\n\n";
                            
                            for(Component& component: globalInputs) { component.UpdateLeadColors(); }
                            for(Component& component: globalOutput) { component.UpdateLeadColors(); }
                            components.ForEach([](Component& component){ component.UpdateLeadColors(); });
                        break;
                        
                        case sf::Keyboard::J:
                            Pin::hideConnectedHitboxes = !Pin::hideConnectedHitboxes;
                            std::cout << "\nconnected pins' hitboxes: " << (Pin::hideConnectedHitboxes? "hidden" : "shown") << "\n\n";
                            
                            for(Component& component: globalInputs) { component.UpdateLeadColors(); }
                            for(Component& component: globalOutput) { component.UpdateLeadColors(); }
                            components.ForEach([](Component& component){ component.UpdateLeadColors(); });
                        break;
                        
                        case sf::Keyboard::Delete:
                        {
                            selectedComponent = nullptr;
                            const sf::Vector2f mousePosition{ sf::Mouse::getPosition(mainWindow) };
                            auto search = [&](Component& component)
                            {
                                if(component.ContainsCoord(mousePosition)) {
                                    std::cout << "Deleting: " << component.UUID() << '\n';
                                    component.RemoveAllConnections();
                                    components.Remove(component);
                                    ComponentMap::Break(); return true;
                                } return false;
                            };
                            components.ForEach(search);
                        }
                        break;
                        
                        default: break;
                    }
                }
                break;
                
                case sf::Event::MouseWheelScrolled:
                    if(!selectorWindow.isOpen()) { //reopen window and minimize it
                        selectorWindow.Create(); selectorWindow.setVisible(false);
                    }
                    selectorWindow.SetSelection(
                        selectorWindow.NextSelection((event.mouseWheelScroll.delta >= 0), false)
                    );
                break;
                
                case sf::Event::MouseButtonPressed:
                {
                    switch(event.mouseButton.button)
                    {
                        case sf::Mouse::Button::Left:
                        if (selectorWindow.selection > 0) // place component if it's not 'EQ'
                        { components.Push(selectorWindow.selection, heldSprite); }
                        else
                        {
                            bool hitboxFound{false};
                            std::string identifier;
                            const sf::Vector2f mousePosition{ sf::Mouse::getPosition(mainWindow) };
                            
                            auto lambda = [&](Component& component)
                            {
                                if (component.isOutputPinClicked(mousePosition)) {
                                    identifier = std::format("{} output-pin", component.UUID());
                                    hitboxFound = true; selectedComponent = &component;
                                    component.HighlightOutputPin(); mainWindow.draw(component); // draw the highlight before screencap
                                    MouseDragLoop(mainWindow, mousePosition, component.ReadState());
                                    component.HighlightOutputPin(false); ComponentMap::Break(); return true;
                                } else if(component.inputHitboxClicked(mousePosition)) {
                                    identifier = std::format("{} input-pin", component.UUID());
                                    hitboxFound = true; selectedComponent = nullptr; ComponentMap::Break(); return true;
                                } else if(component.ContainsCoord(mousePosition)) {
                                    #ifdef _ISDEBUG
                                    component.PrintConnections();
                                    #endif
                                    identifier = std::format("{}", component.UUID());
                                    hitboxFound = true; selectedComponent = nullptr; ComponentMap::Break(); return true;
                                }
                                return false;
                            };
                            for(Component& component: globalInputs) { if(lambda(component)) goto endSearch; }
                            for(Component& component: globalOutput) { if(lambda(component)) goto endSearch; }
                            components.ForEach(lambda);
                            
                            endSearch:
                            if (!hitboxFound) identifier = "empty click";
                            std::cout << std::format("{} @({}, {})", identifier, mousePosition.x, mousePosition.y);
                            if (!selectedComponent) std::cout << '\n';
                        }
                        break;
                        
                        case sf::Mouse::Button::Right:
                        {
                            bool hitboxFound{false};
                            const sf::Vector2f mousePosition{ sf::Mouse::getPosition(mainWindow) };
                            auto lambda = [&](Component& component)
                            {
                                if(component.ContainsCoord(mousePosition)) {
                                    std::string identifier = std::format("{}", component.UUID());
                                    std::cout << std::format("disconnecting: {} @({}, {})\n", identifier, mousePosition.x, mousePosition.y);
                                    #ifdef _ISDEBUG
                                    component.PrintConnections();
                                    #endif
                                    component.RemoveAllConnections();
                                    selectedComponent = nullptr;
                                    //selectedComponent = &component;
                                    hitboxFound = true; ComponentMap::Break(); return true;
                                } return false;
                            };
                            
                            for(Component& component: globalInputs) { if(lambda(component)) goto endSearch2; }
                            for(Component& component: globalOutput) { if(lambda(component)) goto endSearch2; }
                            components.ForEach(lambda);
                            
                            endSearch2:
                            if (!hitboxFound) { std::cout << "empty right-click\n"; break; }
                            // don't need to do this for inputs because their state never changes
                            for(Component& component: globalInputs) { component.Update(); component.PropagateLogic(); component.UpdateLeadColors(); }
                            for(Component& component: globalOutput) { component.Update(); component.PropagateLogic(); component.UpdateLeadColors(); }
                            components.ForEach([](Component& component){ component.Update(); component.PropagateLogic(); component.UpdateLeadColors(); });
                        }
                        break;
                        
                        default: break;
                    }
                }
                break;
                
                case sf::Event::MouseButtonReleased:
                {
                    if (selectorWindow.selection != LogicGate::OpType::EQ) break;
                    if (!selectedComponent) break; // only output pins can be routed to input
                    
                    bool hitboxFound{false};
                    const sf::Vector2f mousePosition{ sf::Mouse::getPosition(mainWindow) };
                    auto lambda = [&](Component& component) {
                        if (component.inputHitboxClicked(mousePosition)) {
                            std::cout << std::format(" -> {} input-pin @({}, {})\n",
                                component.UUID(), mousePosition.x, mousePosition.y);
                            hitboxFound = true;
                            selectedComponent->CreateConnection(&component, component.getClickedInput(mousePosition));
                            selectedComponent->PropagateLogic();
                            component.PropagateLogic();
                            ComponentMap::Break(); return true;
                        } return false;
                    };
                    
                    for(Component& component: globalInputs) { if(lambda(component)) goto endSearch3; }
                    for(Component& component: globalOutput) { if(lambda(component)) goto endSearch3; }
                    components.ForEach(lambda);
                    
                    endSearch3:
                    if (!hitboxFound) std::cout << '\n'; // flushing held output
                    selectedComponent = nullptr;
                    if(hitboxFound) components.ForEach([](Component& component){ component.Update(); component.PropagateLogic(); component.UpdateLeadColors(); });
                }
                break;
                
                default: break;
            }
        }
        
        mainWindow.clear(backgroundColor);
        
        for(const Component& component: globalInputs) { mainWindow.draw(component); }
        for(const Component& component: globalOutput) { mainWindow.draw(component); }
        components.ForEach([&mainWindow](const Component& component){ mainWindow.draw(component); });
        
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
