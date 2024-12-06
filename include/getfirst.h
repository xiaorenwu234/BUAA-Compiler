//
// Created by xht  on 2024/10/9.
//

#ifndef COMPILER_GETFIRST_H
#define COMPILER_GETFIRST_H

#include<string>
#include"lexical_analysis.h"

//获取到first集

bool isBType(word_match inputword);

bool isConstDef(word_match inputword);

bool isVarDef(word_match inputword);

bool isFuncFParams(word_match inputword);

bool isFuncFParam(word_match inputword);

bool isForStmt(word_match inputword);

bool isExp(word_match inputword);

bool isCond(word_match inputword);

bool isLVal(word_match inputword);

bool isPrimaryExp(word_match inputword);

bool isNum(word_match inputword);

bool isCharacter(word_match inputword);

bool isUnaryExp(word_match inputword);

bool isUnaryOp(word_match inputword);

bool isMulExp(word_match inputword);

bool isAddExp(word_match inputword);

bool isRelExp(word_match inputword);

bool isEqExp(word_match inputword);

bool isLAndExp(word_match inputword);

bool isLOrExp(word_match inputword);

bool isConstExp(word_match inputword);

#endif //COMPILER_GETFIRST_H
