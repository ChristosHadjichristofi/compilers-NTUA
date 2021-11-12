#include "types.hpp"

/************************************/
/*             CUSTOMTYPE           */
/************************************/

CustomType::CustomType() {}

CustomType::CustomType(std::string n): name(n) {}

std::string CustomType::getName() { return name; }

std::string CustomType::getTypeName() {
    return name;
}

/************************************/
/*               UNIT               */
/************************************/

Unit::Unit() { typeValue = TYPE_UNIT; ofType = nullptr; size = -1; }

std::string Unit::getName() { return "Unit"; }

std::string Unit::getTypeName() {
    return "unit";
}

/************************************/
/*             INTEGER              */
/************************************/

Integer::Integer() { typeValue = TYPE_INT; ofType = nullptr; size = -1; }

std::string Integer::getName() { return "Integer"; }

std::string Integer::getTypeName() {
    return "int";
}

/************************************/
/*            CHARACTER             */
/************************************/

Character::Character() { typeValue = TYPE_CHAR; ofType = nullptr; size = -1; }
   
std::string Character::getName() { return "Character"; }

std::string Character::getTypeName() {
    return "char";
}

/************************************/
/*             BOOLEAN              */
/************************************/

Boolean::Boolean() { typeValue = TYPE_BOOL; ofType = nullptr; size = -1; }

std::string Boolean::getName() { return "Boolean"; }

std::string Boolean::getTypeName() {
    return "bool";
}

/************************************/
/*              FLOAT               */
/************************************/

Float::Float() { typeValue = TYPE_FLOAT; ofType = nullptr; size = -1; }

std::string Float::getName() { return "Float"; }

std::string Float::getTypeName() {
    return "float";
}

/************************************/
/*             FUNCTION             */
/************************************/

Function::Function(/*CustomType *it ,*/ CustomType *ot) { outputType = ot; typeValue = TYPE_FUNC; ofType = nullptr; size = -1; }

std::string Function::getName() { return "Function"; }

std::string Function::getTypeName() {
    std::string type;
    if (!params.empty()) {
        for (auto i : params) type += i->getTypeName() + " -> ";
    }
    type += outputType->getTypeName();
    return type;
}

/************************************/
/*            REFERENCE             */
/************************************/

Reference::Reference(CustomType *ct) { typeValue = TYPE_REF; ofType = ct; size = -1; }

std::string Reference::getName() { return "Reference"; }

std::string Reference::getTypeName() {
    std::string type = "ref ";
    if(ofType != nullptr) type += ofType->getTypeName();

    return type;
}

/************************************/
/*               ARRAY              */
/************************************/

Array::Array(CustomType *ct, int s) { typeValue = TYPE_ARRAY; ofType = ct; size = s; isInferred = false; }

std::string Array::getName() { return "Array"; }

std::string Array::getTypeName() {
    std::string type = "array [";
    for (int i = 0; i < size - 1; i++) type += "*,";
    type += "*] of "; 
    type += ofType->getTypeName();
    return type;
}

/************************************/
/*             CUSTOMID             */
/************************************/

CustomId::CustomId(std::string n) { typeValue = TYPE_ID; ofType = nullptr; size = -1; name = n; }

std::string CustomId::getName() { return "CustomId"; }

std::vector<CustomType *> CustomId::getParams() { return params; }

void CustomId::pushToParams(CustomType *newParam) { params.push_back(newParam); }

void CustomId::replaceParam(CustomType *newType, int i) { params.at(i) = newType; }

std::string CustomId::getTypeName() {
    std::string type = name + "(";
    if(!params.empty()) {
        bool first = true;
        for (auto p : params) { 
            if(first) first = false;
            else type += ", "; 
            type += p->getTypeName();
        }
    }
    type += ")";
    return type;
}

/************************************/
/*              UNKNOWN             */
/************************************/

Unknown::Unknown() { typeValue = TYPE_UNKNOWN; ofType = nullptr; size = -1; }

std::string Unknown::getName() { return "Unknown"; }

std::string Unknown::getTypeName() {
    if (size == -1) return "unknown"; 
    else return "none";
}