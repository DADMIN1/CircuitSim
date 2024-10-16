#ifndef CIRCUITSIM_COMPONENTMAP_HPP
#define CIRCUITSIM_COMPONENTMAP_HPP


#include "Interactives.hpp"


class ComponentMap: std::map<std::string, Component>
{
    static bool shouldBreak;
    
    public:
    static void Break() { shouldBreak = true; }
    
    auto& Push(LogicGate::OpType T) { return insert({LogicGate::GetNextUUID(T), Component(T)}).first->second; }
    auto& Push(LogicGate::OpType T, sf::Sprite S) { return insert({LogicGate::GetNextUUID(T), Component(T, S)}).first->second; }
    
    void Disconnect()
    {
        
    }
    
    void ForEach(auto&& lambda) {
        shouldBreak = false;
        for(auto&[key, value]: *this) { lambda(value); if(shouldBreak) break; }
    }
    
    /*auto ForEach(auto lambda)
    {
        shouldBreak = false;
        std::vector<auto> retvals;
        for (auto&[key, value]: *this) {
            retvals.push_back(lambda(value));
            if(shouldBreak) break;
        }
        return retvals;
    }*/
    //auto ForEach(auto(*functionPtr)(Component&))
    
};


#endif
