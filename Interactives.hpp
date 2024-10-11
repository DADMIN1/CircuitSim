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
    
    bool BelongsTo(std::string parentUID) const {
        return (UUID == (parentUID + '#' + std::to_string(index+mtype)));
    }
    
    static constexpr float size = 25.f;
    Pin(Type T, int I, std::string S): sf::RectangleShape{{size, size}}, 
       mtype{T}, index{I}, UUID{ S + '#' + std::to_string(index+mtype)}
    {
        setOrigin(size/2.f, size/2.f);
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
    
    void LinkTo(Pin* pin)
    {
        pin->isConnected = true;
        drain = pin;
        
        const sf::Vector2f dist{ pin->getPosition() - source.getPosition() };
        const sf::Vector2f halfDist {dist/2.f};
        constexpr float halfThick {thickness/2.f};
        const float extraLength {(dist.y > 0.f)? thickness : -thickness}; // adjustment must be relative to y-direction
        
        // if distance is primarily vertical, split the distance into two vertical components and one horizontal
        if (std::abs(dist.x) < std::abs(dist.y))
        {
            sf::RectangleShape& verticalOne = lines.emplace_back(sf::Vector2f{thickness, halfDist.y});
            verticalOne.setOrigin({halfThick, 0});
            verticalOne.setPosition(source.getPosition()); // hitbox is positioned at the end of the lead
            
            sf::RectangleShape& horizontal = lines.emplace_back(sf::Vector2f{dist.x+halfThick, thickness});
            horizontal.setOrigin({0, halfThick}); // don't change X-origin; it complicates alignment
            horizontal.setPosition(source.getPosition()); horizontal.move({-halfThick, halfDist.y}); // aligning to body of gate
            
            sf::RectangleShape& verticalTwo = lines.emplace_back(sf::Vector2f{thickness, halfDist.y+extraLength});
            verticalTwo.setOrigin({halfThick, 0});
            verticalTwo.setPosition(horizontal.getPosition()); verticalTwo.move({dist.x, -extraLength/2.f});
            
            horizontal.setOutlineThickness(-1); verticalOne.setOutlineThickness(-1); verticalTwo.setOutlineThickness(-1);
        } 
        else //if distance is primarily horizontal, split into two horizontal components instead of two vertical
        {
            sf::RectangleShape& horizontal = lines.emplace_back(sf::Vector2f{halfDist.x, thickness});
            horizontal.setOrigin({0, halfThick}); // don't change X-origin; it complicates alignment
            horizontal.setPosition(source.getPosition());
            
            sf::RectangleShape& vertical = lines.emplace_back(sf::Vector2f{thickness, dist.y+extraLength});
            vertical.setOrigin({halfThick, 0});
            vertical.setPosition(horizontal.getPosition()); vertical.move({halfDist.x, -extraLength/2.f});
            
            sf::RectangleShape& horizontalTwo = lines.emplace_back(sf::Vector2f{halfDist.x, thickness});
            horizontalTwo.setOrigin({0, halfThick});
            horizontalTwo.setPosition(source.getPosition()); horizontalTwo.move({halfDist.x, dist.y});
            
            vertical.setOutlineThickness(-1); horizontal.setOutlineThickness(-1); horizontalTwo.setOutlineThickness(-1);
        }
        
        PropagateState();
        return;
    }
    
    Wire() = delete;
    explicit Wire(const Pin& sourcePin, std::string componentID)
    : source{sourcePin}, parentID{componentID}
    { 
        lines.reserve(3); //two primary segments + a middle joining segment
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
    
    void HighlightOutputPin() { outputs[0].setFillColor(sf::Color(0xFFFFFF77)); }
    
    inline void UpdateLeadColors() { 
        for (int I{0}; I < int(inputs.size()); ++I) {
            leads[I].setOutlineColor(inputs[I].state? sf::Color(0x000000AA) : sf::Color(0xFFFFFFAA));
            leads[I].setFillColor( ( inputs[I].state? sf::Color::Red : sf::Color::Black)); }
        leads.back().setFillColor( (outputs[0].state? sf::Color::Red : sf::Color::Black)); //back lead is the output line
        leads.back().setOutlineColor(outputs[0].state?sf::Color(0x000000AA) : sf::Color(0xFFFFFFAA));
        outputs[0].setFillColor(sf::Color::Transparent);
    }
    
    // implementing the SFML 'draw' function for this class
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override 
    {
        target.draw(sprite, states);
        for (const Pin& pin: inputs ) { target.draw(pin, states); }
        for (const Pin& pin: outputs) { target.draw(pin, states); }
        for (const auto& lead: leads) { target.draw(lead,states); }
        for(const auto&[s,wire]:wires){ target.draw(wire,states); }
    }
    
    void PropagateLogic()
    {
        outputs[0].isConnected = !wires.empty();
        bool oldState = gate.state;
        if (oldState != gate.Update(inputs[0].state, inputs[1].state)) 
        {
            outputs[0].state = gate.state;
            sprite.setTexture(*TextureStorage::GetSprite(gate.mType, gate.state).getTexture());
            for(auto& [s,wire]: wires) { wire.PropagateState(); }
        }
        UpdateLeadColors();
        return;
    }
    
    void CreateConnection(Component* target, Pin* targetPin)
    {
        if(!target || !targetPin) return;
        if (target == this) return; // disallow self-connections
        
        // check other connections to target, and disconnect them if they go to targetPin
        if (target->incoming.contains(targetPin->UUID)) {
            Component* oldParent = target->incoming[targetPin->UUID];
            oldParent->wires.erase(targetPin->UUID);
            target->incoming.erase(targetPin->UUID);
        }
        
        outputs[0].isConnected = true;
        target->incoming[targetPin->UUID] = this;
        Wire& wire = wires.emplace(targetPin->UUID, Wire{outputs[0], UUID()}).first->second;
        wire.LinkTo(targetPin);
        PropagateLogic();
        return;
    }
    
    void SetPosition(float X, float Y)
    {
        sprite.setPosition(X, Y);
        const float hOffset = 0.f; // pins aligned to end of wires
        //const float hOffset = 32.f; // pins aligned to sprite's body
        const float vOffset = sprite.getGlobalBounds().height / (inputs.size()*2);
        
        for (Pin& pin: inputs) {
            // wires' spacing is biased towards edge, so additonal offset is needed
            pin.setPosition(X + hOffset, Y + vOffset*(1 + pin.index*2));
            //if(pin.index > 0) pin.move(0, vOffset);
            
            sf::RectangleShape& lead = leads[pin.index];
            lead.setPosition(pin.getPosition()); // assuming left-side
        }
        
        // assuming only one output
        outputs[0].setPosition(
            X + sprite.getGlobalBounds().width - hOffset,  // offset from right edge
            Y + (sprite.getGlobalBounds().height/2.f)
        );
        
        // output lead
        leads.back().setPosition(outputs[0].getPosition());
        leads.back().move({-Wire::leadLength, 0});
        
        return;
    }
    
    void Init(std::string name="")
    {
        //const auto size = sprite.getGlobalBounds().getSize();
        //sprite.setOrigin(size.x/2.f, size.y/2.f);
        
        inputs.reserve(2);
        leads.reserve(3); // 2 input, 1 output
        if(name.empty()) { name = gate.GetName(); }  //TODO: name is unused
        // label = name;
        
        int numInputs = ((gate.mType <= 1)? 1 : 2);
        for (int I{0}; I < numInputs; ++I) { 
            Pin& pin = inputs.emplace_back(Pin::Input, I, UUID());
            sf::RectangleShape& lead = leads.emplace_back(sf::Vector2f{Wire::leadLength, Wire::thickness});
            lead.setFillColor(sf::Color::Black);
            lead.setOutlineColor(sf::Color(0xFFFFFFAA));
            lead.setOutlineThickness(-1);
            lead.setOrigin({0, Wire::thickness/2.f});
            lead.setPosition(pin.getPosition()); // assuming left-side
        }
        
        sf::RectangleShape& leadout = leads.emplace_back(sf::Vector2f{Wire::leadLength, Wire::thickness});
        leadout.setOrigin({0, Wire::thickness/2.f}); // don't change X-origin; it complicates alignment
        //leadout.setPosition(outputs[0].getPosition()); leadout.move({-Wire::leadLength, 0}); // aligning to body of gate
        // 'outputs[0]' hasn't been positioned yet.
        
        leadout.setFillColor(sf::Color::Black);
        leadout.setOutlineColor(sf::Color(0xFFFFFFAA));
        leadout.setOutlineThickness(-1);
        
        const auto&[X, Y] = sprite.getPosition();
        SetPosition(X, Y);
        return;
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
