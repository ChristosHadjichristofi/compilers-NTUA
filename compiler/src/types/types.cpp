#include "types.hpp"

/************************************/
/*             CUSTOMTYPE           */
/************************************/

CustomType::CustomType() {}

CustomType::CustomType(std::string n): name(n) {}

void CustomType::printOn(std::ostream &out) const {
    if (!name.empty()) out << "CustomType(" << name << ")";
    else out << "CustomType()";
}

std::string CustomType::getName() { return name; }

llvm::Value* CustomType::compile() const {
    return 0;
}

llvm::Type* CustomType::getLLVMType() {
    if (typeValue == TYPE_INT) return i32;
    if (typeValue == TYPE_FLOAT) return DoubleTyID;
    if (typeValue == TYPE_BOOL) return i1;
    if (typeValue == TYPE_CUSTOM) return TheModule->getTypeByName(name)->getPointerTo();
    if (typeValue == TYPE_ARRAY && ofType != nullptr && ofType->typeValue == TYPE_CHAR) return TheModule->getTypeByName("Array_String_1");
    if (typeValue == TYPE_ARRAY) {
        /* name of struct type that we're searching */
        std::string arrName = "Array_" + ofType->getName() + "_" + std::to_string(size);

        if (TheModule->getTypeByName(arrName) != nullptr) return TheModule->getTypeByName(arrName)->getPointerTo();

        /* create array */
        std::vector<llvm::Type *> members;
        /* ptr to array */
        members.push_back(llvm::PointerType::getUnqual(ofType->getLLVMType()));
        /* dimensions number of array */
        members.push_back(i32);

        for (int i = 0; i < size; i++) members.push_back(i32);         

        /* create the struct */
        llvm::StructType *arrayStruct = llvm::StructType::create(TheContext, arrName);
        arrayStruct->setBody(members);

        return arrayStruct->getPointerTo();
    }
    if (typeValue == TYPE_CHAR) return i8;
    if (typeValue == TYPE_REF) return ofType->getLLVMType();
    if (typeValue == TYPE_UNKNOWN) return TheModule->getTypeByName("unit"); 
    if (typeValue == TYPE_UNIT) return TheModule->getTypeByName("unit");

    return nullptr;
}

llvm::Value *CustomType::getLLVMValue() {
    if (typeValue == TYPE_INT) return c32(0);
    if (typeValue == TYPE_FLOAT) return fp(0);
    if (typeValue == TYPE_BOOL) return c1(0);
    if (typeValue == TYPE_CHAR) return c8(0);
    if (typeValue == TYPE_REF || typeValue == TYPE_ARRAY) return ofType->getLLVMValue();
    if (typeValue == TYPE_UNIT) return llvm::ConstantAggregateZero::get(TheModule->getTypeByName("unit"));

    return nullptr;
}

/************************************/
/*               UNIT               */
/************************************/

Unit::Unit() { typeValue = TYPE_UNIT; ofType = nullptr; size = -1; }
   
void Unit::printOn(std::ostream &out) const {
    out << "Unit"; 
    if (ofType != nullptr) { out << " of type "; ofType->printOn(std::cout); }
}

std::string Unit::getName() { return "Unit"; }

llvm::Value *Unit::compile() const {
    return 0;
}

/************************************/
/*             INTEGER              */
/************************************/

Integer::Integer() { typeValue = TYPE_INT; ofType = nullptr; size = -1; }
   
void Integer::printOn(std::ostream &out) const {
    out << "Integer"; 
    if (ofType != nullptr) { out << " of type "; ofType->printOn(std::cout); }
}

std::string Integer::getName() { return "Integer"; }

llvm::Value* Integer::compile() const {
    return 0;
}

/************************************/
/*            CHARACTER             */
/************************************/

Character::Character() { typeValue = TYPE_CHAR; ofType = nullptr; size = -1; }
   
void Character::printOn(std::ostream &out) const {
    out << "Character"; 
    if (ofType != nullptr) { out << " of type "; ofType->printOn(std::cout); }
}

std::string Character::getName() { return "Character"; }

llvm::Value* Character::compile() const {
    return 0;
}

/************************************/
/*             BOOLEAN              */
/************************************/

Boolean::Boolean() { typeValue = TYPE_BOOL; ofType = nullptr; size = -1; }

void Boolean::printOn(std::ostream &out) const {
    out << "Boolean"; 
    if (ofType != nullptr) { out << " of type "; ofType->printOn(std::cout); }
}

std::string Boolean::getName() { return "Boolean"; }

llvm::Value* Boolean::compile() const {
    return 0;
}

/************************************/
/*              FLOAT               */
/************************************/

Float::Float() { typeValue = TYPE_FLOAT; ofType = nullptr; size = -1; }

void Float::printOn(std::ostream &out) const {
    out << "Float"; 
    if (ofType != nullptr) { out << " of type "; ofType->printOn(std::cout); }
}

std::string Float::getName() { return "Float"; }

llvm::Value* Float::compile() const {
    return 0;
}

/************************************/
/*             FUNCTION             */
/************************************/

Function::Function(/*CustomType *it ,*/ CustomType *ot) { outputType = ot; typeValue = TYPE_FUNC; ofType = nullptr; size = -1; }

void Function::printOn(std::ostream &out) const {
    out << "fn: ";
    if (!params.empty()) 
        for (auto i : params) { i->printOn(out); out << " -> "; } 
    outputType->printOn(out);
}

std::string Function::getName() { return "Function"; }

llvm::Value* Function::compile() const {
    return 0;
}

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

void Reference::printOn(std::ostream &out) const {
    out << "Reference"; 
    if(ofType != nullptr) {
        out <<" of type "; ofType->printOn(out); if (SHOW_MEM) out << "  MEM:  " << ofType;
    }
}

llvm::Value* Reference::compile() const {
    return 0;
}

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

void Array::printOn(std::ostream &out) const {
    // out << "Array(ofType"; ofType->printOn(out); out <<", size:" << size <<")";
    out << "Array [";
    for (int i = 0; i < size - 1; i++) out << "*,";
    out << "*] of type "; 
    ofType->printOn(out);
}

std::string Array::getName() { return "Array"; }

llvm::Value* Array::compile() const {
    return 0;
}

/************************************/
/*             CUSTOMID             */
/************************************/

CustomId::CustomId(std::string n) { typeValue = TYPE_ID; ofType = nullptr; size = -1; name = n; }

void CustomId::printOn(std::ostream &out) const {
    out << name << "("; 
    if(!params.empty()) {
        bool first = true;
        for (auto p : params) { 
        if(first) first = false;
        else out << ", "; 
        p->printOn(out);
        }
    }
    out << ")";
}

std::string CustomId::getName() { return "CustomId"; }

std::vector<CustomType *> CustomId::getParams() { return params; }

void CustomId::pushToParams(CustomType *newParam) { params.push_back(newParam); }

void CustomId::replaceParam(CustomType *newType, int i) { params.at(i) = newType; }

llvm::Value* CustomId::compile() const {
    return 0;
}

/************************************/
/*              UNKNOWN             */
/************************************/

Unknown::Unknown() { typeValue = TYPE_UNKNOWN; ofType = nullptr; size = -1; }
   
void Unknown::printOn(std::ostream &out) const {
    if (size == -1) out << "Unknown"; else out << "None";
    if (SHOW_MEM) out << this;
    if (ofType != nullptr) { out << " of type "; ofType->printOn(std::cout); }
}

std::string Unknown::getName() { return "Unknown"; }

llvm::Value* Unknown::compile() const {
    return 0;
}