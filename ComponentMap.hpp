#ifndef CIRCUITSIM_COMPONENTMAP_HPP
#define CIRCUITSIM_COMPONENTMAP_HPP

#include <unordered_map>

#include "Interactives.hpp"


class ComponentMap: std::unordered_map<std::string, Component>
{
    static bool shouldBreak;
    
    public:
    static void Break() { shouldBreak = true; }
    
    auto& Push(LogicGate::OpType T) { return insert({LogicGate::GetNextUUID(T), Component(T)}).first->second; }
    auto& Push(LogicGate::OpType T, sf::Sprite S) { return insert({LogicGate::GetNextUUID(T), Component(T, S)}).first->second; }
    
    void ForEach(auto&& lambda) {
        shouldBreak = false;
        for(auto&[key, value]: *this) { lambda(value); if(shouldBreak) break; }
    }
    
    std::vector<Component*> AddBank(LogicGate::OpType T, int count /* , int bank_index=-1 */)
    {
        /* static int last_bank_index{0};
        if (bank_index == -1) { bank_index = last_bank_index++; }
        else { last_bank_index = bank_index; } */
        
        static int Xoffset{172};
        std::vector<Component*> bank; bank.reserve(count);
        
        for (int I{0}; I < count; ++I) 
        {
            Component& component = Push(T);
            component.SetPosition(Xoffset + (I%4)*7, (I+1)*(1024.f/(count+1)));
            bank.emplace_back(&component);
        }
        
        Xoffset += 172;
        return bank;
    }
    
    void Remove(Component& component) { erase(component.UUID()); }
};


#endif
