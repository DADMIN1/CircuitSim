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
    
    // updates states from inputs, then returns true if it's state changed
    bool Update(bool A) { bool old{state};  state = ((mType == NOT)? !A : A); return (old==state); } // unary
    bool Update(bool A, bool B) { bool old{state}; state = Eval(mType, A, B); return (old==state); } // binary
    
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
    static std::string GetNextUUID(OpType op) { return GetName(op) + '_' + std::to_string(nextID); }
    
    LogicGate(OpType T): UUID{nextID++}, mType{T}
    { ; }
};


#endif
