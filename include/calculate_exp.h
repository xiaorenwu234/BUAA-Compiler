//
// Created by 13069 on 2024/11/14.
//

#ifndef COMPILER_CALCULATE_EXP_H
#define COMPILER_CALCULATE_EXP_H

#include<vector>
#include<iostream>

extern int CurrentExpIndex;

int Primary_Exp(std::vector<std::string> exps);

int Unary_Exp(std::vector<std::string> exps);

int Mul_Exp(std::vector<std::string> exps);

int Add_Exp(std::vector<std::string> exps);

int Cal_Exp(std::vector<std::string> exps);

#endif //COMPILER_CALCULATE_EXP_H
