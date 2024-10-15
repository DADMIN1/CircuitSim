#include "Interactives.hpp"


// static members
bool Pin::displayHitboxes{true};
bool Pin::hideConnectedHitboxes{true};


void Wire::LinkTo(Pin* pin)
{
    pin->isConnected = true;
    drain = pin;
    
    const sf::Vector2f dist{ pin->getPosition() - source.getPosition() };
    const sf::Vector2f halfDist {dist/2.f};
    constexpr float halfThick {thickness/2.f};
    const float extraLength {(dist.y > 0.f)? thickness : -thickness}; // adjustment must be relative to y-direction
    const float hoffset{ (7.f * (1-pin->index)) + (((dist.x > 0.f)!=(dist.y > 0.f))? 2.f : -2.f) };
    // offset by target pin, and an additional directionally-based offset to avoid overlaps between wires going opposite directions
    
    // if distance is primarily vertical, split the distance into two vertical components and one horizontal
    // also use vertical layout for backwards connections, to prevent the wires from cutting across the gates and linking from the wrong side.
    if ((std::abs(dist.x) < std::abs(dist.y)) || (dist.x < 0.f))
    {
        sf::RectangleShape& verticalOne = lines.emplace_back(sf::Vector2f{thickness, halfDist.y});
        verticalOne.setOrigin({halfThick, 0});
        verticalOne.setPosition(source.getPosition()); // hitbox is positioned at the end of the lead
        verticalOne.move(-hoffset, 0);
        
        sf::RectangleShape& horizontal = lines.emplace_back(sf::Vector2f{dist.x+halfThick-hoffset, thickness});
        horizontal.setOrigin({0, halfThick}); // don't change X-origin; it complicates alignment
        horizontal.setPosition(verticalOne.getPosition()); horizontal.move({-halfThick, halfDist.y}); // aligning to body of gate
        
        sf::RectangleShape& verticalTwo = lines.emplace_back(sf::Vector2f{thickness, halfDist.y+extraLength});
        verticalTwo.setOrigin({halfThick, 0});
        verticalTwo.setPosition(horizontal.getPosition()); verticalTwo.move({dist.x-hoffset, -extraLength/2.f});
        
        sf::RectangleShape& gap = lines.emplace_back(sf::Vector2f{hoffset*2.f, thickness});
        gap.setOrigin({0, halfThick});
        gap.setPosition(drain->getPosition());
        gap.move(-hoffset*2.f, 0);
        gap.setOutlineThickness(-1);
        
        horizontal.setOutlineThickness(-1); verticalOne.setOutlineThickness(-1); verticalTwo.setOutlineThickness(-1);
    } 
    else //if distance is primarily horizontal, split into two horizontal components instead of two vertical
    {
        sf::RectangleShape& horizontal = lines.emplace_back(sf::Vector2f{halfDist.x-hoffset, thickness});
        horizontal.setOrigin({0, halfThick}); // don't change X-origin; it complicates alignment
        horizontal.setPosition(source.getPosition());
        
        sf::RectangleShape& vertical = lines.emplace_back(sf::Vector2f{thickness, dist.y+extraLength});
        vertical.setOrigin({halfThick, 0});
        vertical.setPosition(horizontal.getPosition()); vertical.move({halfDist.x-hoffset, -extraLength/2.f});
        
        sf::RectangleShape& horizontalTwo = lines.emplace_back(sf::Vector2f{halfDist.x+(hoffset*2.f), thickness});
        horizontalTwo.setOrigin({0, halfThick});
        horizontalTwo.setPosition(source.getPosition()); horizontalTwo.move({halfDist.x-hoffset, dist.y});
        
        vertical.setOutlineThickness(-1); horizontal.setOutlineThickness(-1); horizontalTwo.setOutlineThickness(-1);
    }
    
    PropagateState();
    return;
}


void Component::SetPosition(float X, float Y)
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


void Component::UpdateLeadColors()
{ 
    // updating sprite texture to match state
    const auto oldPosition = sprite.getPosition();
    sprite = TextureStorage::GetSprite(gate.mType, gate.state);
    sprite.setPosition(oldPosition);
    // for some reason 'setTexture' doesn't work
    //sprite.setTexture(*TextureStorage::GetSprite(gate.mType, gate.state).getTexture());
    
    for (int I{0}; I < int(inputs.size()); ++I) {
        leads[I].setOutlineColor(inputs[I].state? sf::Color(0x000000AA) : sf::Color(0xFFFFFFAA));
        leads[I].setFillColor( ( inputs[I].state? sf::Color::Red : sf::Color::Black)); }
    leads.back().setFillColor( (outputs[0].state? sf::Color::Red : sf::Color::Black)); //back lead is the output line
    leads.back().setOutlineColor(outputs[0].state?sf::Color(0x000000AA) : sf::Color(0xFFFFFFAA));
    outputs[0].setFillColor(sf::Color::Transparent);
    
    return;
}


void Component::PropagateLogic()
{
    outputs[0].isConnected = !wires.empty();
    const bool oldState = gate.state;
    if (oldState != gate.Update(inputs[0].state, inputs[1].state)) 
    {
        outputs[0].state = gate.state;
        for(auto& [s,wire]: wires) { wire.PropagateState(); }
    }
    UpdateLeadColors();
    return;
}


void Component::CreateConnection(Component* target, Pin* targetPin)
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


void Component::Init(std::string name)
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
