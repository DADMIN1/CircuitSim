#ifndef CIRCUITSIM_LOGICGATE_HPP
#define CIRCUITSIM_LOGICGATE_HPP

#include <string>


class LogicGate
{
    static int nextID;
    int UUID;
    
    protected:
    bool state {false};
    friend class Component;
    
    public:
    enum OpType
    {
        EQ,  NOT,  // 'Buffer' is the real name for 'EQ'
        OR,  NOR,
        AND, NAND,
        XOR, XNOR,
        LAST_ENUM,
    } mType;
    
    static bool Eval(OpType T, bool A, bool B) {
        switch(T) {
            case  EQ: return (A);      case  NOT: return !(A); // unary ops should not be valid here
            case  OR: return (A || B); case  NOR: return !(A || B);
            case AND: return (A && B); case NAND: return !(A && B);
            case XOR: return (A != B); case XNOR: return !(A != B);
            default: return false;
        }
    }
    
    bool Update(bool A) { state = ((mType == NOT)? !A : A); return state; } // unary
    bool Update(bool A, bool B) { state = LogicGate::Eval(mType, A, B); return state; } // binary
    
    static std::string GetName(OpType T) {
        switch(T) {
            case   EQ: return  "EQ"; case  NOT: return "NOT";
            case   OR: return  "OR"; case  NOR: return "NOR";
            case  AND: return "AND"; case NAND: return "NAND";
            case  XOR: return "XOR"; case XNOR: return "XNOR";
            default: return "INVALID";
        }
    }
    
    std::string GetName() const { return GetName(mType); }
    
    LogicGate(OpType T): UUID{nextID++}, mType{T}
    { ; }
};


#endif
