//
// Created by 13069 on 2024/11/6.
//

#ifndef COMPILER_INTERMEDIATE_CODE_H
#define COMPILER_INTERMEDIATE_CODE_H

#include<iostream>
#include<function.h>

//全局变量初始化的时候先存储起来，然后进入main函数之后直接将它们全部输出
extern std::vector<std::string> assign_strs;

//存储上面全局变量初始化的字符串
extern int store_register_cnt;

class Register_Pair {
public:
    std::string Register;
    std::string Type;

    Register_Pair(std::string Register_Pair_ = "", std::string Type_ = "")
            : Register(std::move(Register_Pair_)), Type(std::move(Type_)) {}
};

//当前使用的临时寄存器
extern int register_cnt;

//是否已经进入了函数
extern bool is_global;

//声明函数（仅给出当前函数名）
void Define_Function(const std::string &FuncType, const std::string &FuncName);

//向当前声明的函数中添加变量
void Define_Function_Params(const std::string &ParamType, const std::string &ParamName);

//声明变量（如果有数组的话）
void Define_Varible(const std::string &VarType, const std::string &VarName, int ArrayLength = 0);

//给非数组赋值
void
Assign_Varible_Const_NotArray(const std::string &VarName, const std::string &VarType, bool global_state, int value);

//将数组赋值给一个i**
void Assign_Varible_Ptr_Array(const Register_Pair &LeftRegister,const Register_Pair &RightRegister);

//将变量加载到临时寄存器中
Register_Pair Load_Varible(const std::string &VarName, const std::string &VarType, bool global);

//加载整个数组
Register_Pair Load_Varible_Array(const std::string &VarName, const std::string &VarType, bool global,int ArrayLength);

//处理表达式求值,返回的是临时的虚拟寄存器以及其对应的类型
Register_Pair Handle_Exp(Register_Pair LeftArg, Register_Pair RightArg, const std::string &ope);

//给非数组'变量'赋值
void
Assign_Varible_Not_Array(Register_Pair leftPair,
                         Register_Pair registerPair);

//给常量数组初始化赋值
void Array_Init_Const(const std::string &VarName, const std::string &VarType, int ArrayLength, bool global,
                      const std::vector<int> &values);

//给变量数组初始化赋值
void Array_Init_Var(const std::string &VarName, const std::string &VarType, int ArrayLength, bool global,
                    const std::vector<Register_Pair> &values);

//给当前的临时寄存器进行零扩展
Register_Pair Zext_Register(const Register_Pair &registerPair);

//给当前的临时寄存器进行截断
Register_Pair Trunc_Register(const Register_Pair &registerPair);

//加载当前的数组元素
Register_Pair
Load_Current_Array_Element( std::string VarName, std::string VarType, bool global, int Array_Length,
                            Register_Pair registerPair);

//调用函数
Register_Pair Call_Func(const std::string &FuncName, const std::string &FuncType, std::vector<std::string> par_types,
                        std::vector<Register_Pair> par_regs);

//向当前的部分添加常量字符串
void Add_String(const std::string &str, bool need_print = false);

//把变量转成bool类型
Register_Pair To_Bool(const Register_Pair &registerPair);

//if标签跳转
void
If_Jump(Register_Pair registerPair, int depth, const std::string &Then_Register, const std::string &Else_Register);

//函数返回
void Func_Return(const std::string &FuncType, Register_Pair registerPair);

//输出全局变量的赋值
void Print_Global();

//在函数声明的时候将形参转移到重新声明的变量中
void Move_FParams(const std::string &FormRegister, const std::string &NewRegister, const std::string &VarType);

//输出内容，第一个参数是格式化字符串，第二个参数是输出的寄存器
void Print(const std::string &FormatString, std::vector<Register_Pair> OutPars);

//有目标类型的Trunc
Register_Pair Trunc_Register_Bool(const Register_Pair &registerPair);

//调用输出字符的函数
void Call_PutChar(const std::string &CharName);

//调用输出数字的函数
void Call_PutInt(const std::string &IntName);

//在or运算中判断当前临时寄存器是否是0
Register_Pair Judge_Zero(Register_Pair registerPair);

//将judge_var这个全局变量赋值
void Load_1_Reg();

void Load_0_Reg();

int get_cur_num(const std::string &input_str);

//判断是否不是0
Register_Pair Judge_Not_Zero(Register_Pair registerPair);

//加载指针
Register_Pair LoadPtr(const Register_Pair &registerPair);

#endif //COMPILER_INTERMEDIATE_CODE_H
