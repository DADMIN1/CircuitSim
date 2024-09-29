#include "SelectorWindow.hpp"
#include "TextureStorage.hpp"

#include <SFML/Window.hpp>  //sf::Event


// Main.cpp
extern bool usingVsync;
extern const unsigned int framerateCap;
extern const sf::Color backgroundColor;


void SelectorWindow::Redraw()
{
    clear(backgroundColor);
    for(const sf::Sprite& sprite: TextureStorage::sprites) { draw(sprite); }
    draw(selectionRect);
    
    display();
    return;
}


SelectorWindow::SelectorWindow(float scale): spriteScale{scale}
{
    const int windowSize = static_cast<int>(1024.f*scale);
    const sf::Vector2f spriteSize {windowSize/2.f, windowSize/4.f};
    
    create(sf::VideoMode(windowSize, windowSize), "SelectorWindow", sf::Style::Titlebar);
    setVerticalSyncEnabled(usingVsync);
    setFramerateLimit(framerateCap);
    
    selectionRect = sf::RectangleShape{spriteSize};
    selectionRect.setFillColor(sf::Color::Transparent);
    selectionRect.setOutlineColor(sf::Color::Cyan);
    selectionRect.setOutlineThickness(-4.f);
    SetSelection(OpType::AND);
    
    Redraw();
    
    return;
}


LogicGate::OpType SelectorWindow::NextSelection(OpType sel, bool reverse, bool byColumn)
{
    int next = static_cast<int>(sel);
    int offset = (byColumn? 2 : 1);
    if(reverse) offset *= -1;
    
    next += offset;
    next = next % OpType::LAST_ENUM;
    if(next <= 0) next = 1;
    
    return OpType(next);
}


void SelectorWindow::SetSelection(OpType sel)
{
    selection = sel;
    
    //constexpr int W {512}, H {256}; // 1024x1024
    constexpr int W {128}, H {64}; // 0.25 scale
    const int X {W*(sel%2)}, Y {H*(sel/2)};
    selectionRect.setPosition(X, Y);
    
    Redraw();
    
    return;
}


void SelectorWindow::EventLoop()
{
    if(!hasFocus()) return;
    
    sf::Event event;
    while(pollEvent(event))
    {
        switch(event.type)
        {
            case sf::Event::Closed: close(); break;
            
            case sf::Event::KeyPressed:
            switch(event.key.code)
            {
                // maps numpad/numrow inputs
                #define CASE_KEYNUM_OPTYPE(N) \
                case sf::Keyboard::Numpad##N: \
                case sf::Keyboard::Num##N:   \
                    SetSelection(OpType(N)); \
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
                static_assert(7 < OpType::LAST_ENUM);
                #undef CASE_KEYNUM_OPTYPE
                
                default: break;
            }
            break;
            
            // TODO: handle mouse inputs
            
            default: break;
        }
    }
    
    return;
}
