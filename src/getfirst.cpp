//
// Created by xht  on 2024/10/9.
//
#include"getfirst.h"

bool isBType(word_match inputword) {
    return inputword.word == "int" || inputword.word == "char";
}

bool isConstDef(word_match inputword) {
    return inputword.category == "IDENFR";
}

bool isVarDef(word_match inputword) {
    return inputword.category == "IDENFR";
}

bool isFuncFParams(word_match inputword) {
    return isFuncFParam(inputword);
}

bool isFuncFParam(word_match inputword) {
    return isBType(inputword);
}

bool isForStmt(word_match inputword) {
    return isLVal(inputword);
}

bool isExp(word_match inputword) {
    return isAddExp(inputword);
}

bool isCond(word_match inputword) {
    return isLOrExp(inputword);
}

bool isLVal(word_match inputword) {
    return inputword.category == "IDENFR";
}

bool isPrimaryExp(word_match inputword) {
    return inputword.word == "(" || isNum(inputword) || isCharacter(inputword) ||
           isLVal(inputword);
}

bool isNum(word_match inputword) {
    return inputword.category == "INTCON";
}

bool isCharacter(word_match inputword) {
    return inputword.category == "CHRCON";
}

bool isUnaryExp(word_match inputword) {
    return isPrimaryExp(inputword) || inputword.category == "IDENFR" || isUnaryOp(inputword);
}

bool isUnaryOp(word_match inputword) {
    return inputword.word == "+" || inputword.word == "-" || inputword.word == "!";
}

bool isMulExp(word_match inputword) {
    return isUnaryExp(inputword);
}

bool isAddExp(word_match inputword) {
    return isMulExp(inputword);
}

bool isRelExp(word_match inputword) {
    return isAddExp(inputword);
}

bool isEqExp(word_match inputword) {
    return isRelExp(inputword);
}

bool isLAndExp(word_match inputword) {
    return isEqExp(inputword);
}

bool isLOrExp(word_match inputword) {
    return isLAndExp(inputword);
}

bool isConstExp(word_match inputword) {
    return isAddExp(inputword);
}