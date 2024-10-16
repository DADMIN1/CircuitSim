#ifndef CIRCUITSIM_SELECTORWINDOW_HPP
#define CIRCUITSIM_SELECTORWINDOW_HPP

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include "LogicGate.hpp"


class SelectorWindow: sf::RenderWindow
{
    const int windowSize;
    
    using OpType = LogicGate::OpType;
    OpType selection;
    float spriteScale;
    bool selectionHasChanged{false};
    sf::RectangleShape selectionRect{};
    
    OpType NextSelection(bool reverse=false, bool byColumn=false);
    void SetSelection(OpType);
    void EventLoop();
    void Redraw();
    void Create();
    SelectorWindow(float spriteScale=1.f);
    
    public:
    friend int main(int argc, char** argv);
};



#endif
