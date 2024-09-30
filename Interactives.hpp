#ifndef CIRCUITSIM_INTERACTIVES_HPP
#define CIRCUITSIM_INTERACTIVES_HPP

#include <vector>
#include <string>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include "LogicGate.hpp"
#include "TextureStorage.hpp"


struct Pin: sf::RectangleShape
{
    const enum Type { Input, Output, } mtype;
    const int index; // counting connections for current component. Used to offset wire layouts
    bool isConnected{false};
    bool state{false};
    //sf::Text label;
    
    static constexpr float size = 25.f;
    Pin(Type T, int I): sf::RectangleShape{{size, size}}, mtype{T}, index{I}
    {
        setOrigin(size/2.f, size/2.f);
        setFillColor(sf::Color::Transparent);
        setOutlineColor(sf::Color::White);
        setOutlineThickness(-1.f);
    }
};


class Component: public sf::Drawable
{
    //sf::Text label;
    LogicGate gate;
    sf::Sprite sprite;
    std::vector<Pin> inputs;
    std::vector<Pin> outputs;
    
  public:
    // implementing the SFML 'draw' function for this class
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override 
    {
        target.draw(sprite, states);
        for (const Pin& pin: inputs ) { target.draw(pin); }
        for (const Pin& pin: outputs) { target.draw(pin); }
    }
    
    inline std::string UUID() const { return gate.GetName() + '_' + std::to_string(gate.UUID); }
    inline std::string Name() const { return gate.GetName(); }
    std::size_t GetPinCount() const { return inputs.size() ; }
    
    bool isOutputPinClicked(const sf::Vector2f& coord) const
    { return outputs[0].getGlobalBounds().contains(coord); }
    
    bool inputHitboxClicked(const sf::Vector2f& coord) const {
        for(const Pin& pin: inputs) { if(pin.getGlobalBounds().contains(coord)) return true; }
        return false;
    }
    
    bool ContainsCoord(const sf::Vector2f& coord) const {
        bool spriteContains = sprite.getGlobalBounds().contains(coord);
        bool ouputsContains = isOutputPinClicked(coord);
        bool inputsContains = inputHitboxClicked(coord);
        return (spriteContains || ouputsContains || inputsContains);
    }
    
    void SetPosition(float X, float Y)
    {
        sprite.setPosition(X, Y);
        const float hOffset = 0.f; // pins aligned to end of wires
        //const float hOffset = 32.f; // pins aligned to sprite's body
        const float vOffset = sprite.getGlobalBounds().height / (inputs.size()*2);
        
        for (Pin& pin: inputs) {
            pin.setPosition(X + hOffset, Y + vOffset*(1 + pin.index*2));
            //if(pin.index > 0) pin.move(0, vOffset);
            // wires' spacing is biased towards edge, so additonal offset is needed
        }
        
        // assuming only one output
        outputs[0].setPosition(
            X + sprite.getGlobalBounds().width - hOffset,  // offset from right edge
            Y + (sprite.getGlobalBounds().height/2.f)
        );
    }
    
    void Init(std::string name="")
    {
        //const auto size = sprite.getGlobalBounds().getSize();
        //sprite.setOrigin(size.x/2.f, size.y/2.f);
        
        inputs.reserve(2);
        if(name.empty()) { name = gate.GetName(); }
        // label = name;
        
        int numInputs = ((gate.mType <= 1)? 1 : 2);
        for (int I{0}; I < numInputs; ++I) { inputs.emplace_back(Pin::Input, I); }
        outputs.emplace_back(Pin::Output, 0);
        
        const auto&[X, Y] = sprite.getPosition();
        SetPosition(X, Y);
    }
    
    explicit Component(LogicGate::OpType T, const sf::Sprite& S, std::string name=""):
    gate{T}, sprite{S} { Init(name); }
    
    explicit Component(LogicGate::OpType T, std::string name=""):
             Component(T, TextureStorage::GetSprite(T), name){;}
};


#endif
