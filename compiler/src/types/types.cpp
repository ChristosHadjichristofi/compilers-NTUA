#include "types.hpp"

/************************************/
/*             CUSTOMTYPE           */
/************************************/

CustomType::CustomType() {}

CustomType::CustomType(std::string n): name(n) {}

std::string CustomType::getName() { return name; }


/************************************/
/*               UNIT               */
/************************************/

Unit::Unit() { typeValue = TYPE_UNIT; ofType = nullptr; size = -1; }

std::string Unit::getName() { return "Unit"; }

/************************************/
/*             INTEGER              */
/************************************/

Integer::Integer() { typeValue = TYPE_INT; ofType = nullptr; size = -1; }

std::string Integer::getName() { return "Integer"; }

/************************************/
/*            CHARACTER             */
/************************************/

Character::Character() { typeValue = TYPE_CHAR; ofType = nullptr; size = -1; }
   
std::string Character::getName() { return "Character"; }

/************************************/
/*             BOOLEAN              */
/************************************/

Boolean::Boolean() { typeValue = TYPE_BOOL; ofType = nullptr; size = -1; }

std::string Boolean::getName() { return "Boolean"; }

/************************************/
/*              FLOAT               */
/************************************/

Float::Float() { typeValue = TYPE_FLOAT; ofType = nullptr; size = -1; }

std::string Float::getName() { return "Float"; }

/************************************/
/*             FUNCTION             */
/************************************/

Function::Function(/*CustomType *it ,*/ CustomType *ot) { outputType = ot; typeValue = TYPE_FUNC; ofType = nullptr; size = -1; }

std::string Function::getName() { return "Function"; }

/************************************/
/*            REFERENCE             */
/************************************/

Reference::Reference(CustomType *ct) { 
    typeValue = TYPE_REF; 
    // this should be for keyword 'new'
    if(ct->typeValue == TYPE_ARRAY){
        yyerror("Input cannot be of CustomType 'Array'");
    }
    ofType = ct;
    size = -1; 
}

std::string Reference::getName() { return "Reference"; }

/************************************/
/*               ARRAY              */
/************************************/

Array::Array(CustomType *ct, int s) {
    typeValue = TYPE_ARRAY;
    if(ct->typeValue == TYPE_ARRAY){
        yyerror("Input cannot be of CustomType 'Array'");
    }
    ofType = ct;
    size = s;
    isInferred = false;
}

std::string Array::getName() { return "Array"; }

/************************************/
/*             CUSTOMID             */
/************************************/

CustomId::CustomId(std::string n) { typeValue = TYPE_ID; ofType = nullptr; size = -1; name = n; }

std::string CustomId::getName() { return "CustomId"; }

std::vector<CustomType *> CustomId::getParams() { return params; }

void CustomId::pushToParams(CustomType *newParam) { params.push_back(newParam); }

void CustomId::replaceParam(CustomType *newType, int i) { params.at(i) = newType; }

/************************************/
/*              UNKNOWN             */
/************************************/

Unknown::Unknown() { typeValue = TYPE_UNKNOWN; ofType = nullptr; size = -1; }

std::string Unknown::getName() { return "Unknown"; }