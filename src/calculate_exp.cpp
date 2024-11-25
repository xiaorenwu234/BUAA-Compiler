//
// Created by 13069 on 2024/11/14.
//
#include<calculate_exp.h>

int CurrentExpIndex=0;

int Primary_Exp(std::vector<std::string> exps) {
    int a;
    if (exps[CurrentExpIndex] == "(") {
        CurrentExpIndex++;
        a = Add_Exp(exps);
        if (exps[CurrentExpIndex] == ")") {
            CurrentExpIndex++;
        }
    }
    else{
        a=std::stoi(exps[CurrentExpIndex]);
        CurrentExpIndex++;
    }
    return a;

}

int Unary_Exp(std::vector<std::string> exps) {
    if (exps[CurrentExpIndex] == "+") {
        CurrentExpIndex++;
        return Unary_Exp(exps);
    } else if (exps[CurrentExpIndex] == "-") {
        CurrentExpIndex++;
        return -Unary_Exp(exps);
    } else {
        return Primary_Exp(exps);
    }
}

int Mul_Exp(std::vector<std::string> exps) {
    int a = Unary_Exp(exps);
    while (CurrentExpIndex < exps.size() &&
           (exps[CurrentExpIndex] == "*" || exps[CurrentExpIndex] == "/" || exps[CurrentExpIndex] == "%")) {
        if (exps[CurrentExpIndex] == "*") {
            CurrentExpIndex++;
            a = a * Unary_Exp(exps);
        } else if (exps[CurrentExpIndex] == "/") {
            CurrentExpIndex++;
            a = a / Unary_Exp(exps);
        } else {
            CurrentExpIndex++;
            a = a % Unary_Exp(exps);
        }
    }
    return a;
}

int Add_Exp(std::vector<std::string> exps) {
    int a = Mul_Exp(exps);
    while (CurrentExpIndex < exps.size() && (exps[CurrentExpIndex] == "+" || exps[CurrentExpIndex] == "-")) {
        if (exps[CurrentExpIndex] == "+") {
            CurrentExpIndex++;
            a = a + Mul_Exp(exps);
        } else {
            CurrentExpIndex++;
            a = a - Mul_Exp(exps);
        }
    }
    return a;
}


int Cal_Exp(std::vector<std::string> exps){
    CurrentExpIndex=0;
    return Add_Exp(exps);
}