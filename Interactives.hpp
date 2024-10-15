#ifndef CIRCUITSIM_INTERACTIVES_HPP
#define CIRCUITSIM_INTERACTIVES_HPP

#include <vector>
#include <string>
#include <map>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include "LogicGate.hpp"
#include "TextureStorage.hpp"


struct Pin: sf::RectangleShape
{
    const enum Type { Output, Input, } mtype;
    const int index; // counting connections for current component. Used to offset wire layouts
    bool isConnected{false};
    bool state{false};
    const std::string UUID; // component UUID + index (index counts from 1 for inputs)
    //sf::Text label;
    
    static bool displayHitboxes;
    static bool hideConnectedHitboxes; // don't display hitboxes for connected pins
    
    bool BelongsTo(std::string parentUID) const {
        return (UUID == (parentUID + '#' + std::to_string(index+mtype)));
    }
    
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override 
    {
        if (!displayHitboxes || (hideConnectedHitboxes && isConnected)) return;
        //target.draw(*static_cast<const sf::RectangleShape*>(this), states);
        // passing any kind of reference or pointer will crash here; only copying the base class works
        target.draw(sf::RectangleShape{*this}, states);
    }
    
    static constexpr float size = 25.f;
    Pin(Type T, int I, std::string S): sf::RectangleShape{{size*2.f, size}}, 
       mtype{T}, index{I}, UUID{ S + '#' + std::to_string(index+mtype)}
    {
        const float xorigin{ (mtype == Input)? size/2.f : size*1.5f }; // align left for inputs, right for outputs
        setOrigin(xorigin, size/2.f);
        setFillColor(sf::Color::Transparent);
        setOutlineColor(sf::Color::White);
        setOutlineThickness(-1.f);
    }
};


class Wire: public sf::Drawable
{
    const Pin& source;
    const std::string parentID; // UUID of component providing the 'source' Pin (which must be an 'Output' Pin)
    std::vector<sf::RectangleShape> lines{};
    Pin* drain{nullptr};
    
    static constexpr float thickness{4.f};
    static constexpr float leadLength{36.f}; // length of segments leading in/out of gates
    
    public:
    friend class Component;
    
    // implementing the SFML 'draw' function for this class
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override 
    { for (const sf::RectangleShape& line: lines) { target.draw(line, states); } }
    
    void UpdateColor() {
        const sf::Color lineColor{(source.state? sf::Color::Red : sf::Color::Black)};
        const sf::Color outlineColor{(source.state? sf::Color(0x000000AA) : sf::Color(0xFFFFFF99))};
        for (sf::RectangleShape& line: lines) { 
            line.setFillColor(lineColor);
            line.setOutlineColor(outlineColor);
        }
    }
    
    void PropagateState() {
        if(drain) { drain->state = source.state; }
        UpdateColor();
    }
    
    void LinkTo(Pin* pin);
    
    Wire() = delete;
    explicit Wire(const Pin& sourcePin, std::string componentID)
    : source{sourcePin}, parentID{componentID}
    { 
        lines.reserve(4); //two primary segments + a middle joining segment
        // TODO: exceeding the size specified here during 'LinkTo' causes an abort: 
        //  "pure virtual method called. terminate called without an exception. Aborted (core dumped)"
    }
};


class Component: public sf::Drawable
{
    //sf::Text label;
    LogicGate gate;
    sf::Sprite sprite;
    std::vector<Pin> inputs;
    std::vector<Pin> outputs;
    std::vector<sf::RectangleShape> leads; // line segments leading in/out of gates
    
    // keys are the pin-UUIDs
    std::map<std::string, Component*> incoming; //key is input-pinID, value is parent of connecting wire
    std::map<std::string, Wire> wires; //key is target pinID
    
    public:
    bool isGlobalIn {false}; //TODO: this is bad
    bool isGlobalOut{false};
    
    inline std::string UUID() const { return gate.GetName() + '_' + std::to_string(gate.UUID); }
    inline std::string Name() const { return gate.GetName(); }
    std::size_t GetPinCount() const { return inputs.size() ; }
    
    bool isOutputPinClicked(const sf::Vector2f& coord) const
    { return outputs[0].getGlobalBounds().contains(coord); }
    
    bool inputHitboxClicked(const sf::Vector2f& coord) const {
        for(const Pin& pin: inputs) { if(pin.getGlobalBounds().contains(coord)) return true; }
        return false;
    }
    
    Pin* getClickedInput(const sf::Vector2f& coord) {
        for(Pin& pin: inputs) { if(pin.getGlobalBounds().contains(coord)) return &pin; }
        return nullptr;
    }
    
    bool ContainsCoord(const sf::Vector2f& coord) const {
        bool spriteContains = sprite.getGlobalBounds().contains(coord);
        bool ouputsContains = isOutputPinClicked(coord);
        bool inputsContains = inputHitboxClicked(coord);
        return (spriteContains || ouputsContains || inputsContains);
    }
    
    void SetPosition(float X, float Y);
    void HighlightOutputPin(bool on=true) { outputs[0].setFillColor(on? sf::Color(0xFFFFFF77) : sf::Color::Transparent); }
    void UpdateLeadColors();
    void PropagateLogic();
    void CreateConnection(Component* target, Pin* targetPin);
    void Init(std::string name="");
    
    // implementing the SFML 'draw' function for this class
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override 
    {
        target.draw(sprite, states);
        for (const Pin& pin: inputs ) { target.draw(pin, states); }
        for (const Pin& pin: outputs) { target.draw(pin, states); }
        for (const auto& lead: leads) { target.draw(lead,states); }
        for(const auto&[s,wire]:wires){ target.draw(wire,states); }
    }
    
    explicit Component(LogicGate::OpType T, const sf::Sprite& S, std::string name="")
    : gate{T}, sprite{S}, outputs{{Pin::Output, 0, UUID()}}
    { Init(name); }
    
    explicit Component(LogicGate::OpType T, std::string name=""):
             Component(T, TextureStorage::GetSprite(T), name){;}
    
    friend void MakeGlobalIO(std::vector<Component>&, bool, std::vector<bool>);
    friend int ReadIO(const std::vector<Component>&);
    bool ReadState() const { return gate.state; }
};

void MakeGlobalIO(std::vector<Component>& outvec, bool isInput, std::vector<bool> inputBits);
int ReadIO(const std::vector<Component>&);


#endif
