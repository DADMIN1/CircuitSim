#include "Interactives.hpp"


void MakeGlobalIO(std::vector<Component>& components, bool isInput, std::vector<bool> inputBits)
{
    if (!isInput) { for(int i{0}; i<8; ++i) inputBits.push_back(false); }
    for (int I{1}; bool bit: inputBits)
    {
        Component& component { components.emplace_back(
            Component{LogicGate::EQ, (isInput? "input":"output")+std::to_string(I)}
        )};
        
        component.isGlobalIn  = isInput;
        component.isGlobalOut = !isInput;
        
        //int Xoffset = (isInput? -96: 1024-32); // set unused hitbox offscreen
        int Xoffset = (isInput? -72: 1024-36); // less offscreen
        component.SetPosition(Xoffset, I*(1024.f/(inputBits.size()+1)));
        
        std::vector<Pin>& pins {(isInput? component.inputs : component.outputs)};
        pins[0].state = bit;
        for (sf::RectangleShape& lead: component.leads) {
            if (bit) lead.setFillColor(sf::Color::Red);
        }
        //for(Pin& pin: component.inputs) { pins[0].state = bit; }
        component.PropagateLogic();
        ++I;
    }
    return;
}


int ReadIO(const std::vector<Component>& pins)
{
    int result = pins[0].ReadState(); int base = 2;
    for (std::size_t I{1}; I < pins.size(); ++I, base*=2) { result += base*pins[I].ReadState(); }
    return result;
}
