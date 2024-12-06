//
// Created by xht  on 2024/10/8.
//

#ifndef COMPILER_GRAMMER_ANALYSIS_H
#define COMPILER_GRAMMER_ANALYSIS_H

#include<string>
#include<iostream>
#include<vector>
#include<utility>
#include"symbletable.h"

extern std::vector<std::vector<std::string>> strs;

extern std::vector<std::pair<int,std::string>> funcnames;

void CompUnit();

void Decl();

void ConstDecl();

void BType();

void ConstDef();

void ConstInitVal();

void VarDecl();

void VarDef();

void InitVal();

void FuncDef();

void MainFuncDef();

void FuncType();

void FuncFParams();

void FuncFParam();

void Block();

void BlockItem();

void Stmt();

void ForStmt();

Register_Pair Exp();

void Cond();

Register_Pair LVal(bool is_in_exp=false);

Register_Pair PrimaryExp();

void Number();

void Character();

Register_Pair UnaryExp();

void UnaryOp();

void FuncRParams();

Register_Pair MulExp();

Register_Pair AddExp();

Register_Pair RelExp();

Register_Pair EqExp();

void LAndExp();

void LOrExp();

void ConstExp();

#endif //COMPILER_GRAMMER_ANALYSIS_H
