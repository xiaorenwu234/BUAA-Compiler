//
// Created by 13069 on 2024/11/17.
//

#ifndef COMPILER_TARGET_CODE_H
#define COMPILER_TARGET_CODE_H

#include<iostream>
#include<string>
#include<map>
#include<vector>
#include<function.h>
#include <fstream>
#include<algorithm>

//寄存器对应的编号
extern std::map<int, std::string> register_num;

//计算当前的icmp的编号
extern int icmp_cnt;

//当前不能够被挤掉的寄存器
extern std::vector<int> Current_Hot;

//当前寄存器是否被占用
extern bool state[24];

//寄存器与当前位置的对应
class Register_Match {
public:
    //虚拟寄存器名称（含百分号或@符号）
    std::string virtual_register;
    //当前位置（已经加载到寄存器上还是在栈上）
    std::string current_place;
    //如果在栈上，保存偏移量
    int offset;
    //如果在寄存器上，保存当前的实际寄存器编号
    int real_register;
    //保存当前虚拟寄存器的大小
    int space;

    explicit Register_Match(std::string virtual_register_ = "", std::string current_place_ = "", int offset_ = 0,
                            int real_register_ = 0, int space_ = 4)
            : virtual_register(std::move(virtual_register_)), current_place(std::move(current_place_)),
              offset(offset_), real_register(real_register_), space(space_) {}
};


//寄存器池，负责分配寄存器以及改变寄存器当前存储的位置
class Global_Pool {
public:

    void alloca_global_var(const std::string &name, int size) {
        alloca_space -= size;
        All_Registers[name] = Register_Match(name, "Stack", alloca_space, 0, size);
    }

    int get_alloca_space() {
        return alloca_space;
    }

    std::map<std::string, Register_Match> All_Registers;
    int alloca_space;
};

//存储全局寄存器偏移的寄存器池
extern Global_Pool global_pool;

//寄存器池，负责分配寄存器以及改变寄存器当前存储的位置
class Register_Pool {
public:
    //获取当前的寄存器
    std::string Get_Register_Position(const std::string &Register_Name) {
        auto it = All_Registers.find(Register_Name)->second;
        if (it.current_place == "Register") {
            return register_num[it.real_register];
        } else if (it.current_place == "Stack") {
            return std::to_string(it.offset) + "($sp)";
        }
    }
    
    void mips_print_check(const std::string &str){
        if(!check()){
            printf("%s\n",str.c_str());
            exit(0);
        }
        else{
            mips_print(str);
        }
    }

    //创建一个寄存器映射（为临时寄存器创建）
    void Get_Free_Register(const std::string &Register_Name, int space = 4) {
        for (int i = 0; i < register_num.size(); ++i) {
            if (!state[i]) {
                state[i] = true;
                Register_Match registerMatch(Register_Name, "Register", 0, i, space);
                All_Registers[Register_Name] = registerMatch;
                return;
            }
        }
        for (const auto &it: All_Registers) {
            if (it.second.current_place == "Register") {
                if (std::find(Current_Hot.begin(), Current_Hot.end(), it.second.real_register) != Current_Hot.end()) {
                    continue;
                }
                std::string Virtual_Name = it.first;
                alloca_space -= 4;
                int real_reg_num = it.second.real_register;
                if (space == 4) {
                    mips_print_check("sw " + register_num[real_reg_num] + ", " + std::to_string(alloca_space) +
                               "($sp)\n");
                } else {
                    mips_print_check("sb " + register_num[it.second.real_register] + ", " + std::to_string(alloca_space) +
                               "($sp)\n");
                }
                All_Registers.find(Virtual_Name)->second.current_place = "Stack";
                All_Registers.find(Virtual_Name)->second.offset = alloca_space;
                All_Registers.find(Virtual_Name)->second.real_register = 30;
                Register_Match registerMatch(Register_Name, "Register", 0, real_reg_num, space);
                All_Registers[Register_Name] = registerMatch;
                return;
            }
        }
    }

    //在alloca的时候在栈上开辟空间
    void Alloca_Sp(const std::string &Register_Name, int space) {
        alloca_space -= space;
        Register_Match registerMatch(Register_Name, "Stack", alloca_space, 0, space);
        All_Registers[Register_Name] = registerMatch;
    }

    //在调用函数之前将所有的东西都加载到栈上
    void Save_All() {
        mips_print_check("sw $ra, -4($sp)\n");
        for (const auto &it: All_Registers) {
            if (it.second.current_place == "Register") {
                state[it.second.real_register] = false;
                std::string Virtual_Name = it.first;
                int space = All_Registers.find(Virtual_Name)->second.space;
                alloca_space -= 4;
                if (space == 4) {
                    mips_print_check("sw " + register_num[it.second.real_register] + ", " + std::to_string(alloca_space) +
                               "($sp)\n");
                } else {
                    mips_print_check("sb " + register_num[it.second.real_register] + ", " + std::to_string(alloca_space) +
                               "($sp)\n");
                }
                All_Registers.find(Virtual_Name)->second.current_place = "Stack";
                All_Registers.find(Virtual_Name)->second.offset = alloca_space;
                All_Registers.find(Virtual_Name)->second.real_register = 30;
            }
        }
    }

    //将需要的寄存器加载进来,在调用函数的时候使用
    void Add_Params(const std::string &Register_Name, const std::string &Register_Type) {
        for (int i = 0; i < register_num.size(); ++i) {
            if (!state[i]) {
                state[i] = true;
                if (Register_Type.find('*') != std::string::npos || Register_Type.find("i32") != std::string::npos) {
                    if (All_Registers[Register_Name].current_place == "Register") {
                        mips_print_check("addu " + register_num[i] + ", " +
                                   register_num[All_Registers[Register_Name].real_register] + ", $zero\n");
                        state[All_Registers[Register_Name].real_register] = false;
                    } else {
                        mips_print_check(
                                "lw " + register_num[i] + ", " + std::to_string(All_Registers[Register_Name].offset) +
                                "($sp)\n");
                    }
                    if (Register_Type.find('*') != std::string::npos) {
                        mips_print_check(
                                "addi " + register_num[i] + ", " + register_num[i] + ", " +
                                std::to_string(-alloca_space) + "\n");
                    }
                } else {
                    if (All_Registers[Register_Name].current_place == "Register") {
                        mips_print_check("addu " + register_num[i] + ", " +
                                   register_num[All_Registers[Register_Name].real_register] + ", $zero\n");
                        state[All_Registers[Register_Name].real_register] = false;
                    } else {
                        mips_print_check(
                                "lb " + register_num[i] + ", " + std::to_string(All_Registers[Register_Name].offset) +
                                "($sp)\n");
                    }
                }
                break;
            }
        }
    }

    //清空寄存器池
    void Clear_Pool() {
        All_Registers.clear();
        for (int i = 0; i < register_num.size(); ++i) {
            state[i] = false;
        }
        All_Registers["$ra"] = Register_Match("$ra", "Stack", -4, 0, 4);
        alloca_space = -4;
    }

    //临时获取一个空的寄存器
    int Get_Reg_Tmp() {
        for (int i = 0; i < register_num.size(); ++i) {
            if (!state[i]) {
                state[i] = true;
                return i;
            }
        }
        for (const auto &it: All_Registers) {
            if (it.second.current_place == "Register") {
                if (std::find(Current_Hot.begin(), Current_Hot.end(), it.second.real_register) != Current_Hot.end()) {
                    continue;
                }
                int real_reg_num = it.second.real_register;
                std::string Virtual_Name = it.first;
                alloca_space -= 4;
                if (All_Registers[Virtual_Name].space == 4) {
                    mips_print_check("sw " + Get_Register_Position(Virtual_Name) + ", " + std::to_string(alloca_space) +
                               "($sp)\n");
                } else {
                    mips_print_check("sb " + Get_Register_Position(Virtual_Name) + ", " + std::to_string(alloca_space) +
                               "($sp)\n");
                }
                All_Registers.find(Virtual_Name)->second.current_place = "Stack";
                All_Registers.find(Virtual_Name)->second.offset = alloca_space;
                return real_reg_num;
            }
        }
        return 0;
    }

    //将当前名称的数据加载到寄存器中
    void Load_Data(const std::string &Register_Name) {
        if (All_Registers[Register_Name].current_place == "Register") {
            return;
        }
        for (int i = 0; i < register_num.size(); ++i) {
            if (!state[i]) {
                state[i] = true;
                int space = All_Registers.find(Register_Name)->second.space;
                All_Registers[Register_Name].real_register = i;
                if (space == 4) {
                    mips_print_check("lw " + register_num[All_Registers.find(Register_Name)->second.real_register] + ", " +
                               Get_Register_Position(Register_Name) + "\n");
                } else {
                    mips_print_check("lb " + register_num[All_Registers.find(Register_Name)->second.real_register] + ", " +
                               Get_Register_Position(Register_Name) + "\n");
                }
                All_Registers[Register_Name].current_place = "Register";
                return;
            }
        }
        for (const auto &it: All_Registers) {
            if (it.second.current_place == "Register") {
                if (std::find(Current_Hot.begin(), Current_Hot.end(), it.second.real_register) != Current_Hot.end()) {
                    continue;
                }
                std::string Virtual_Name = it.first;
                int real_reg_num = it.second.real_register;
                int space = All_Registers.find(Virtual_Name)->second.space;
                alloca_space -= 4;
                if (space == 4) {
                    mips_print_check("sw " + register_num[it.second.real_register] + ", " + std::to_string(alloca_space) +
                               "($sp)\n");
                } else {
                    mips_print_check("sb " + register_num[it.second.real_register] + ", " + std::to_string(alloca_space) +
                               "($sp)\n");
                }
                All_Registers.find(Virtual_Name)->second.current_place = "Stack";
                All_Registers.find(Virtual_Name)->second.offset = alloca_space;
                All_Registers[Register_Name].real_register = real_reg_num;
                space = All_Registers[Register_Name].space;
                if (space == 4) {
                    mips_print_check("lw " + register_num[All_Registers.find(Register_Name)->second.real_register] + ", " +
                               Get_Register_Position(Register_Name) + "\n");
                } else {
                    mips_print_check("lb " + register_num[All_Registers.find(Register_Name)->second.real_register] + ", " +
                               Get_Register_Position(Register_Name) + "\n");
                }
                All_Registers[Register_Name].current_place = "Register";
                return;
            }
        }
    }

    void store_ope(const std::string &from_type, const std::string &from_reg, const std::string &to_reg) {
        Current_Hot.clear();
        std::string tmp_str;
        //如果存储的是全局变量
        if (to_reg.find('@') != std::string::npos) {
            Current_Hot.clear();
            int offset = global_pool.All_Registers[to_reg].offset;
            if (from_type.find('*') != std::string::npos || from_type.find("i32") != std::string::npos) {
                if (is_string_number(from_reg)) {
                    int num = Get_Reg_Tmp();
                    Current_Hot.push_back(num);
                    mips_print_check("li " + register_num[num] + ", " + from_reg + "\n");
                    mips_print_check("sw " + register_num[num] + ", " + std::to_string(offset) + "($fp)\n");
                    state[num] = false;
                } else {
                    Load_Data(from_reg);
                    Current_Hot.push_back(All_Registers[from_reg].real_register);
                    mips_print_check("sw " + register_num[All_Registers[from_reg].real_register] + ", " +
                               std::to_string(offset) + "($fp)\n");
                }
            } else {
                if (is_string_number(from_reg)) {
                    int num = Get_Reg_Tmp();
                    Current_Hot.push_back(num);
                    mips_print_check("li " + register_num[num] + ", " + from_reg + "\n");
                    mips_print_check("sb " + register_num[num] + ", " + std::to_string(offset) + "($fp)\n");
                    state[num] = false;
                } else {
                    Load_Data(from_reg);
                    Current_Hot.push_back(All_Registers[from_reg].real_register);
                    mips_print_check("sb " + register_num[All_Registers[from_reg].real_register] + ", " +
                               std::to_string(offset) + "($fp)\n");
                }
            }
        } else {
            Current_Hot.clear();
            if (from_type.find('*') != std::string::npos || from_type.find("i32") != std::string::npos) {
                if (All_Registers[to_reg].current_place != "FStack") {
                    if (is_string_number(from_reg)) {
                        int num = Get_Reg_Tmp();
                        Current_Hot.push_back(num);
                        mips_print_check("li " + register_num[num] + ", " + from_reg + "\n");
                        mips_print_check("sw " + register_num[num] + ", " + Get_Register_Position(to_reg) + "\n");
                        state[num] = false;
                    } else {
                        Load_Data(from_reg);
                        Current_Hot.push_back(All_Registers[from_reg].real_register);
                        mips_print_check("sw " + register_num[All_Registers[from_reg].real_register] + ", " +
                                   Get_Register_Position(to_reg) + "\n");
                    }
                } else {
                    if (is_string_number(from_reg)) {
                        int num = Get_Reg_Tmp();
                        Current_Hot.push_back(num);
                        mips_print_check("li " + register_num[num] + ", " + from_reg + "\n");
                        int num2 = Get_Reg_Tmp();
                        Current_Hot.push_back(num2);
                        mips_print_check("lw " + register_num[num2] + ", " + std::to_string(All_Registers[to_reg].offset) +
                                   "($sp)\n");
                        mips_print_check("addu $sp, $sp, " + register_num[num2] + "\n");
                        mips_print_check("sw " + register_num[num] + ", 0($sp)\n");
                        mips_print_check("subu $sp, $sp, " + register_num[num2] + "\n");
                        state[num] = false;
                        state[num2] = false;
                    } else {
                        Current_Hot.clear();
                        Load_Data(from_reg);
                        Current_Hot.push_back(All_Registers[from_reg].real_register);
                        int num2 = Get_Reg_Tmp();
                        Current_Hot.push_back(num2);
                        mips_print_check("lw " + register_num[num2] + ", " + std::to_string(All_Registers[to_reg].offset) +
                                   "($sp)\n");
                        mips_print_check("addu $sp, $sp, " + register_num[num2] + "\n");
                        mips_print_check("sw " + register_num[All_Registers[from_reg].real_register] + ", 0($sp)\n");
                        mips_print_check("subu $sp, $sp, " + register_num[num2] + "\n");
                        state[num2] = false;
                    }
                }
            } else {
                if (All_Registers[to_reg].current_place != "FStack") {
                    if (is_string_number(from_reg)) {
                        int num = Get_Reg_Tmp();
                        Current_Hot.push_back(num);
                        mips_print_check("li " + register_num[num] + ", " + from_reg + "\n");
                        mips_print_check("sb " + register_num[num] + ", " + Get_Register_Position(to_reg) + "\n");
                        state[num] = false;
                    } else {
                        Load_Data(from_reg);
                        Current_Hot.push_back(All_Registers[from_reg].real_register);
                        mips_print_check("sb " + register_num[All_Registers[from_reg].real_register] + ", " +
                                   Get_Register_Position(to_reg) + "\n");
                    }
                } else {
                    if (is_string_number(from_reg)) {
                        int num = Get_Reg_Tmp();
                        Current_Hot.push_back(num);
                        mips_print_check("li " + register_num[num] + ", " + from_reg + "\n");
                        int num2 = Get_Reg_Tmp();
                        Current_Hot.push_back(num2);
                        mips_print_check("lw " + register_num[num2] + ", " + std::to_string(All_Registers[to_reg].offset) +
                                   "($sp)\n");
                        mips_print_check("addu $sp, $sp, " + register_num[num2] + "\n");
                        mips_print_check("sb " + register_num[num] + ", 0($sp)\n");
                        mips_print_check("subu $sp, $sp, " + register_num[num2] + "\n");
                        state[num] = false;
                        state[num2] = false;
                    } else {
                        Current_Hot.clear();
                        Load_Data(from_reg);
                        Current_Hot.push_back(All_Registers[from_reg].real_register);
                        int num2 = Get_Reg_Tmp();
                        Current_Hot.push_back(num2);
                        mips_print_check("lw " + register_num[num2] + ", " + std::to_string(All_Registers[to_reg].offset) +
                                   "($sp)\n");
                        mips_print_check("addu $sp, $sp, " + register_num[num2] + "\n");
                        mips_print_check("sb " + register_num[All_Registers[from_reg].real_register] + ", 0($sp)\n");
                        mips_print_check("subu $sp, $sp, " + register_num[num2] + "\n");
                        state[num2] = false;
                    }
                }
            }
        }
    }

    void load_ope(const std::string &type, const std::string &from_reg, const std::string &target_reg) {
        Current_Hot.clear();
        //从全局变量中加载
        if (from_reg.find('@') != std::string::npos) {
            int offset = global_pool.All_Registers[from_reg].offset;
            Current_Hot.clear();
            if (All_Registers.find(target_reg) == All_Registers.end()) {
                Get_Free_Register(target_reg);
                Current_Hot.push_back(All_Registers[target_reg].real_register);
            }
            if (type.find("i32") != std::string::npos || type.find('*') != std::string::npos) {
                mips_print_check(
                        "lw " + register_num[All_Registers[target_reg].real_register] + ", " + std::to_string(offset) +
                        "($fp)\n");
            } else {
                mips_print_check(
                        "lb " + register_num[All_Registers[target_reg].real_register] + ", " + std::to_string(offset) +
                        "($fp)\n");
            }
        } else {
            Current_Hot.clear();
            if (All_Registers.find(target_reg) == All_Registers.end()) {
                Get_Free_Register(target_reg);
                Current_Hot.push_back(All_Registers[target_reg].real_register);
            }
            if (type.find("i32") != std::string::npos || type.find('*') != std::string::npos) {
                if (All_Registers[from_reg].current_place != "FStack") {
                    mips_print_check("lw " + register_num[All_Registers[target_reg].real_register] + ", " +
                               Get_Register_Position(from_reg) + "\n");
                } else {
                    int num2 = Get_Reg_Tmp();
                    Current_Hot.push_back(num2);
                    mips_print_check("lw " + register_num[num2] + ", " + std::to_string(All_Registers[from_reg].offset) +
                               "($sp)\n");
                    mips_print_check("addu $sp, $sp, " + register_num[num2] + "\n");
                    mips_print_check("lw " + register_num[All_Registers[target_reg].real_register] + ", 0($sp)\n");
                    mips_print_check("subu $sp, $sp, " + register_num[num2] + "\n");
                    state[num2] = false;
                }
            } else {
                if (All_Registers[from_reg].current_place != "FStack") {
                    mips_print_check("lb " + register_num[All_Registers[target_reg].real_register] + ", " +
                               std::to_string(All_Registers[from_reg].offset) + "($sp)\n");
                } else {
                    int num = Get_Reg_Tmp();
                    Current_Hot.push_back(num);
                    mips_print_check("lw " + register_num[num] + ", " + std::to_string(All_Registers[from_reg].offset) +
                               "($sp)\n");
                    mips_print_check("addu $sp, $sp, " + register_num[num] + "\n");
                    mips_print_check("lb " + register_num[All_Registers[target_reg].real_register] + ", 0($sp)\n");
                    mips_print_check("subu $sp, $sp, " + register_num[num] + "\n");
                    state[num] = false;
                }
            }
        }
    }

    //处理加减乘除模表达式计算
    void exp_ope(std::string ope, const std::string &left_arg, const std::string &right_arg,
                 const std::string &result) {
        Current_Hot.clear();
        if (All_Registers.find(result) == All_Registers.end()) {
            Get_Free_Register(result);
        }
        Current_Hot.push_back(All_Registers[result].real_register);
        if (ope.find("add") != std::string::npos || ope.find("sub") != std::string::npos ||
            ope.find("mul") != std::string::npos) {
            if (ope == "add") {
                ope = "addu";
            } else if (ope == "sub") {
                ope = "subu";
            }
            std::string right_reg;
            std::string left_reg;
            int left_flag = 30;
            int right_flag = 30;
            if (!is_string_number(right_arg)) {
                Load_Data(right_arg);
                Current_Hot.push_back(All_Registers[right_arg].real_register);
                right_reg = register_num[All_Registers[right_arg].real_register];
            } else {
                int tmp_reg = Get_Reg_Tmp();
                Current_Hot.push_back(tmp_reg);
                mips_print_check("li " + register_num[tmp_reg] + ", " + right_arg + "\n");
                right_flag = tmp_reg;
                right_reg = register_num[tmp_reg];
            }
            if (!is_string_number(left_arg)) {
                Load_Data(left_arg);
                Current_Hot.push_back(All_Registers[left_arg].real_register);
                left_reg = register_num[All_Registers[left_arg].real_register];
            } else {
                int tmp_reg = Get_Reg_Tmp();
                Current_Hot.push_back(tmp_reg);
                mips_print_check("li " + register_num[tmp_reg] + ", " + left_arg + "\n");
                left_flag = tmp_reg;
                left_reg = register_num[tmp_reg];
            }
            mips_print_check(
                    ope + " " + register_num[All_Registers[result].real_register] + ", " + left_reg + ", " + right_reg +
                    "\n");
            if (left_flag != 30) {
                state[left_flag] = false;
            }
            if (right_flag != 30) {
                state[right_flag] = false;
            }
        }
        if (ope.find("sdiv") != std::string::npos || ope.find("srem") != std::string::npos) {
            std::string right_reg;
            std::string left_reg;
            int left_flag = 30;
            int right_flag = 30;
            if (!is_string_number(right_arg)) {
                Load_Data(right_arg);
                Current_Hot.push_back(All_Registers[right_arg].real_register);
                right_reg = register_num[All_Registers[right_arg].real_register];
            } else {
                int tmp_reg = Get_Reg_Tmp();
                Current_Hot.push_back(tmp_reg);
                mips_print_check("li " + register_num[tmp_reg] + ", " + right_arg + "\n");
                right_flag = tmp_reg;
                right_reg = register_num[tmp_reg];
            }
            if (!is_string_number(left_arg)) {
                Load_Data(left_arg);
                Current_Hot.push_back(All_Registers[left_arg].real_register);
                left_reg = register_num[All_Registers[left_arg].real_register];
            } else {
                int tmp_reg = Get_Reg_Tmp();
                Current_Hot.push_back(tmp_reg);
                mips_print_check("li " + register_num[tmp_reg] + ", " + left_arg + "\n");
                left_flag = tmp_reg;
                left_reg = register_num[tmp_reg];
            }
            mips_print_check("div " + left_reg + ", " + right_reg +
                       "\n");
            if (ope.find("sdiv") != std::string::npos) {
                mips_print_check("mflo " + register_num[All_Registers[result].real_register] + "\n");
            } else {
                mips_print_check("mfhi " + register_num[All_Registers[result].real_register] + "\n");
            }
            if (left_flag != 30) {
                state[left_flag] = false;
            }
            if (right_flag != 30) {
                state[right_flag] = false;
            }
        }
    }

    //处理函数调用
    void call_ope(const std::string &func_name, const std::vector<std::pair<std::string, std::string>> &pars,
                  const std::string &type,
                  const std::string &tar_reg) {
        if (func_name != "putch" && func_name != "putint" && func_name != "getint" && func_name != "getchar") {
            Current_Hot.clear();
            Save_All();
            std::vector<int> tmp_pars;
            for (const auto &par: pars) {
                if (!is_string_number(par.second)) {
                    Add_Params(par.second, par.first);
                } else {
                    int num = Get_Reg_Tmp();
                    mips_print_check("li " + register_num[num] + ", " + par.second + "\n");
                    tmp_pars.push_back(num);
                }
            }
            mips_print_check("addi $sp, $sp, " + std::to_string(alloca_space) + "\n");
            for (int i = 0; i < tmp_pars.size(); ++i) {
                state[i] = false;
            }
            mips_print_check("jal " + func_name + "\n");
            mips_print_check("subi $sp, $sp, " + std::to_string(alloca_space) + "\n");
            mips_print_check("lw $ra, -4($sp)\n");
            for (int i = 0; i < register_num.size(); ++i) {
                state[i] = false;
            }
            if (!tar_reg.empty()) {
                if (All_Registers.find(tar_reg) == All_Registers.end()) {
                    Current_Hot.push_back(0);
                    Get_Free_Register(tar_reg);
                }
                if (Get_Register_Position(tar_reg) != "$v0") {
                    mips_print_check("addu " + Get_Register_Position(tar_reg) + ", $v0, $zero\n");
                }
            }
        } else if (func_name == "putch" || func_name == "putint") {
            ClearA0();
            ClearV0();
            const auto &par = pars[0];
            if (!is_string_number(par.second)) {
                int i = 2;
                if (par.first.find('*') != std::string::npos || par.first.find("i32") != std::string::npos) {
                    if (All_Registers[par.second].current_place == "Register") {
                        mips_print_check("addu " + register_num[i] + ", " +
                                   register_num[All_Registers[par.second].real_register] + ", $zero\n");
                    } else {
                        mips_print_check(
                                "lw " + register_num[i] + ", " + std::to_string(All_Registers[par.second].offset) +
                                "($sp)\n");
                    }
                    if (par.first.find('*') != std::string::npos) {
                        mips_print_check(
                                "addi " + register_num[i] + ", " + register_num[i] + ", " +
                                std::to_string(-alloca_space) + "\n");
                    }
                } else {
                    if (All_Registers[par.second].current_place == "Register") {
                        mips_print_check("addu " + register_num[i] + ", " +
                                   register_num[All_Registers[par.second].real_register] + ", $zero\n");
                    } else {
                        mips_print_check(
                                "lb " + register_num[i] + ", " + std::to_string(All_Registers[par.second].offset) +
                                "($sp)\n");
                    }
                }
            } else {
                mips_print_check("li $a0, " + par.second + "\n");
            }
            if (func_name == "putch") {
                mips_print_check("li $v0, 11\n");
            } else {
                mips_print_check("li $v0, 1\n");
            }
            mips_print_check("syscall\n");
        } else if (func_name == "getchar") {
            ClearV0();
            mips_print_check("li $v0, 12\n");
            mips_print_check("syscall\n");
            if (!tar_reg.empty()) {
                if (All_Registers.find(tar_reg) == All_Registers.end()) {
                    Current_Hot.push_back(0);
                    Get_Free_Register(tar_reg);
                }
                if (Get_Register_Position(tar_reg) != "$v0") {
                    mips_print_check("addu " + Get_Register_Position(tar_reg) + ", $v0, $zero\n");
                }
            }
        } else if (func_name == "getint") {
            ClearV0();
            mips_print_check("li $v0, 5\n");
            mips_print_check("syscall\n");
            if (!tar_reg.empty()) {
                if (All_Registers.find(tar_reg) == All_Registers.end()) {
                    Get_Free_Register(tar_reg);
                }
                if (Get_Register_Position(tar_reg) != "$v0") {
                    mips_print_check("addu " + Get_Register_Position(tar_reg) + ", $v0, $zero\n");
                }
            }
        }
    }

    //函数返回
    void ret_ope(const std::string &type, const std::string &content) {
        Current_Hot.clear();
        if (type.find("void") != std::string::npos) {
            mips_print_check("jr $ra\n");
        } else {
            if (is_string_number(content)) {
                mips_print_check("addi $v0, $zero, " + content + "\n");
            } else {
                if (All_Registers[content].current_place == "Stack") {
                    if (type.find("i32") != std::string::npos) {
                        mips_print_check("lw $v0, " + Get_Register_Position(content) + "\n");
                    } else {
                        mips_print_check("lb $v0, " + Get_Register_Position(content) + "\n");
                    }
                } else {
                    mips_print_check("addu $v0, $zero, " + Get_Register_Position(content) + "\n");
                }
            }
            mips_print_check("jr $ra\n");
        }
    }

    void trunc_ope(const std::string &from_reg, const std::string &to_reg, const std::string &from_type,
                   const std::string &to_type) {
        Current_Hot.clear();
        Get_Free_Register(to_reg);
        Current_Hot.push_back(All_Registers[to_reg].real_register);
        int num = 30;
        std::string from_reg_real;
        if (!is_string_number(from_reg)) {
            Load_Data(from_reg);
            Current_Hot.push_back(All_Registers[from_reg].real_register);
            from_reg_real = register_num[All_Registers[from_reg].real_register];
        } else {
            num = Get_Reg_Tmp();
            Current_Hot.push_back(num);
            mips_print_check("li " + register_num[num] + ", " + from_reg + "\n");
            from_reg_real = register_num[num];
        }
        mips_print_check("andi " + register_num[All_Registers[to_reg].real_register] + ", " +
                   from_reg_real + ", 0xFF\n");
        if (num != 30) {
            state[num] = false;
        }
    }

    void zext_ope(const std::string &from_reg, const std::string &to_reg, const std::string &from_type,
                  const std::string &to_type) {
        Current_Hot.clear();
        Get_Free_Register(to_reg);
        Current_Hot.push_back(All_Registers[to_reg].real_register);
        if (All_Registers[from_reg].current_place == "Stack") {
            if (judge_type(from_type) == 4) {
                mips_print_check("lw " + register_num[All_Registers[to_reg].real_register] + ", " +
                           std::to_string(All_Registers[from_reg].offset) + "($sp)\n");
            } else {
                mips_print_check("lb " + register_num[All_Registers[to_reg].real_register] + ", " +
                           std::to_string(All_Registers[from_reg].offset) + "($sp)\n");
                mips_print_check("andi " + register_num[All_Registers[to_reg].real_register] + ", " +
                           register_num[All_Registers[to_reg].real_register] + ", 0xFF\n");
            }
        } else {
            mips_print_check("andi " + register_num[All_Registers[to_reg].real_register] + ", " +
                       register_num[All_Registers[from_reg].real_register] + ", 0xFF" + "\n");
        }
    }

    //无条件跳转
    void unconditional_jump(const std::string &label) {
        std::string real_label;
        for (int i = 1; i < label.size(); ++i) {
            real_label += label[i];
        }
        mips_print_check("j " + real_label + "\n");
    }

    //条件跳转
    void conditional_jump(const std::string &reg, const std::string &label1, const std::string &label2) {
        std::string real_label1, real_label2;
        for (int i = 1; i < label2.size(); ++i) {
            real_label2 += label2[i];
        }
        for (int i = 1; i < label1.size(); ++i) {
            real_label1 += label1[i];
        }
        Load_Data(reg);
        mips_print_check("beq " + register_num[All_Registers[reg].real_register] + ", $zero, " + real_label2 + "\n");
        mips_print_check("j " + real_label1 + "\n");
    }

    void icmp_ope(const std::string &tar_reg, const std::string &ope, const std::string &left_arg,
                  const std::string &right_arg) {
        Current_Hot.clear();
        Get_Free_Register(tar_reg);
        Current_Hot.push_back(All_Registers[tar_reg].real_register);
        std::string left_arg_reg, right_arg_reg;
        int num1 = 30, num2 = 30;
        if (!is_string_number(left_arg)) {
            Load_Data(left_arg);
            Current_Hot.push_back(All_Registers[left_arg].real_register);
            left_arg_reg = register_num[All_Registers[left_arg].real_register];
        } else {
            num1 = Get_Reg_Tmp();
            Current_Hot.push_back(num1);
            mips_print_check("li " + register_num[num1] + ", " + left_arg + "\n");
            left_arg_reg = register_num[num1];
        }
        if (!is_string_number(right_arg)) {
            Load_Data(right_arg);
            Current_Hot.push_back(All_Registers[right_arg].real_register);
            right_arg_reg = register_num[All_Registers[right_arg].real_register];
        } else {
            num2 = Get_Reg_Tmp();
            Current_Hot.push_back(num2);
            mips_print_check("li " + register_num[num2] + ", " + right_arg + "\n");
            right_arg_reg = register_num[num2];
        }
        if (ope == "slt") {
            mips_print_check("slt " + register_num[All_Registers[tar_reg].real_register] + ", " + left_arg_reg + ", " +
                       right_arg_reg + "\n");
        } else if (ope == "sgt") {
            mips_print_check("slt " + register_num[All_Registers[tar_reg].real_register] + ", " + right_arg_reg + ", " +
                       left_arg_reg + "\n");
        } else if (ope == "eq") {
            mips_print_check("beq " + left_arg_reg + ", " + right_arg_reg + ", Assign1_" + std::to_string(icmp_cnt) + "\n");
            mips_print_check("li " + register_num[All_Registers[tar_reg].real_register] + ", 0\n");
            mips_print_check("j AssignEnd_" + std::to_string(icmp_cnt) + "\n");
            mips_print_check("Assign1_" + std::to_string(icmp_cnt) + ":\n");
            mips_print_check("li " + register_num[All_Registers[tar_reg].real_register] + ", 1\n");
            mips_print_check("AssignEnd_" + std::to_string(icmp_cnt) + ":\n");
            icmp_cnt++;
        } else if (ope == "ne") {
            mips_print_check("bne " + left_arg_reg + ", " + right_arg_reg + ", Assign1_" + std::to_string(icmp_cnt) + "\n");
            mips_print_check("li " + register_num[All_Registers[tar_reg].real_register] + ", 0\n");
            mips_print_check("j AssignEnd_" + std::to_string(icmp_cnt) + "\n");
            mips_print_check("Assign1_" + std::to_string(icmp_cnt) + ":\n");
            mips_print_check("li " + register_num[All_Registers[tar_reg].real_register] + ", 1\n");
            mips_print_check("AssignEnd_" + std::to_string(icmp_cnt) + ":\n");
            icmp_cnt++;
        } else if (ope == "sge") {
            mips_print_check("bge " + left_arg_reg + ", " + right_arg_reg + ", Assign1_" + std::to_string(icmp_cnt) + "\n");
            mips_print_check("li " + register_num[All_Registers[tar_reg].real_register] + ", 0\n");
            mips_print_check("j AssignEnd_" + std::to_string(icmp_cnt) + "\n");
            mips_print_check("Assign1_" + std::to_string(icmp_cnt) + ":\n");
            mips_print_check("li " + register_num[All_Registers[tar_reg].real_register] + ", 1\n");
            mips_print_check("AssignEnd_" + std::to_string(icmp_cnt) + ":\n");
            icmp_cnt++;
        } else if (ope == "sle") {
            mips_print_check("ble " + left_arg_reg + ", " + right_arg_reg + ", Assign1_" + std::to_string(icmp_cnt) + "\n");
            mips_print_check("li " + register_num[All_Registers[tar_reg].real_register] + ", 0\n");
            mips_print_check("j AssignEnd_" + std::to_string(icmp_cnt) + "\n");
            mips_print_check("Assign1_" + std::to_string(icmp_cnt) + ":\n");
            mips_print_check("li " + register_num[All_Registers[tar_reg].real_register] + ", 1\n");
            mips_print_check("AssignEnd_" + std::to_string(icmp_cnt) + ":\n");
            icmp_cnt++;
        }
        if (num1 != 30) {
            state[num1] = false;
        }
        if (num2 != 30) {
            state[num2] = false;
        }
    }

    //需要计算偏移量的数组地址加载
    void need_offset_ptr(const std::string &tar_reg, const std::string &from_ptr, const std::string &offset,
                         const std::string &type) {
        Current_Hot.clear();
        if (is_string_number(offset)) {
            int offset_to_num = std::stoi(offset);
            alloca_space -= 4;
            //从全局数组中加载数据
            if (from_ptr.find('@') != std::string::npos) {
                All_Registers[tar_reg] = Register_Match(tar_reg, "FStack",
                                                        alloca_space,
                                                        0, 4);
                int num = Get_Reg_Tmp();
                Current_Hot.push_back(num);
                mips_print_check("li " + register_num[num] + ", " +
                           std::to_string(global_pool.All_Registers[from_ptr].offset + 4 * offset_to_num) + "\n");
                int num2 = Get_Reg_Tmp();
                Current_Hot.push_back(num2);
                mips_print_check("subu " + register_num[num2] + ", $fp, $sp\n");
                mips_print_check("addu " + register_num[num] + ", " + register_num[num] + ", " + register_num[num2] + "\n");
                mips_print_check(
                        "sw " + register_num[num] + ", " + std::to_string(All_Registers[tar_reg].offset) + "($sp)\n");
                state[num] = false;
                state[num2] = false;
            } else {
                int absolute_offset;
                absolute_offset = offset_to_num * 4 + All_Registers[from_ptr].offset;
                All_Registers[tar_reg] = Register_Match(tar_reg, "FStack", alloca_space, 0, 4);
                int num = Get_Reg_Tmp();
                Current_Hot.push_back(num);
                mips_print_check("li " + register_num[num] + ", " + std::to_string(absolute_offset) + "\n");
                mips_print_check(
                        "sw " + register_num[num] + ", " + std::to_string(All_Registers[tar_reg].offset) + "($sp)\n");
                state[num] = false;
            }
        } else {
            alloca_space -= 4;
            //从全局数组中加载数据
            if (from_ptr.find('@') != std::string::npos) {
                All_Registers[tar_reg] = Register_Match(tar_reg, "FStack",
                                                        alloca_space,
                                                        0, 4);
                int num = Get_Reg_Tmp();
                Current_Hot.push_back(num);
                mips_print_check("li " + register_num[num] + ", " +
                           std::to_string(global_pool.All_Registers[from_ptr].offset) + "\n");
                int num2 = Get_Reg_Tmp();
                Current_Hot.push_back(num2);
                Load_Data(offset);
                Current_Hot.push_back(All_Registers[offset].real_register);
                int num3 = Get_Reg_Tmp();
                Current_Hot.push_back(num3);
                mips_print_check("li " + register_num[num3] + ", 4\n");
                mips_print_check(
                        "mul " + register_num[num2] + ", " + register_num[num3] + ", " + Get_Register_Position(offset) +
                        "\n");
                mips_print_check("addu " + register_num[num] + ", " + register_num[num] + ", " + register_num[num2] + "\n");
                int num4 = Get_Reg_Tmp();
                Current_Hot.push_back(num4);
                mips_print_check("subu " + register_num[num4] + ", $fp, $sp\n");
                mips_print_check("addu " + register_num[num] + ", " + register_num[num] + ", " + register_num[num4] + "\n");
                mips_print_check(
                        "sw " + register_num[num] + ", " + std::to_string(All_Registers[tar_reg].offset) + "($sp)\n");
                state[num] = false;
                state[num2] = false;
                state[num3] = false;
                state[num4] = false;
            } else {
                All_Registers[tar_reg] = Register_Match(tar_reg, "FStack", alloca_space, 0, 4);
                int absolute_offset;
                absolute_offset = All_Registers[from_ptr].offset;
                int num = Get_Reg_Tmp();
                Current_Hot.push_back(num);
                mips_print_check("li " + register_num[num] + ", " +
                           std::to_string(absolute_offset) + "\n");
                int num2 = Get_Reg_Tmp();
                Current_Hot.push_back(num2);
                Load_Data(offset);
                Current_Hot.push_back(All_Registers[offset].real_register);
                int num3 = Get_Reg_Tmp();
                Current_Hot.push_back(num3);
                mips_print_check("li " + register_num[num3] + ", 4\n");
                mips_print_check("mul " + register_num[num2] + ", " + register_num[num3] + ", " +
                           Get_Register_Position(offset) + "\n");
                mips_print_check("addu " + register_num[num] + ", " + register_num[num] + ", " + register_num[num2] + "\n");
                mips_print_check(
                        "sw " + register_num[num] + ", " + std::to_string(All_Registers[tar_reg].offset) + "($sp)\n");
                state[num] = false;
                state[num2] = false;
                state[num3] = false;
            }
        }
    }

    //不需要计算偏移量的数组地址加载
    void not_need_offset_ptr(const std::string &tar_reg, const std::string &from_ptr, const std::string &offset,
                             const std::string &type) {
        Current_Hot.clear();
        alloca_space -= 4;
        All_Registers[tar_reg] = Register_Match(tar_reg, "FStack", alloca_space, 0, 4);
        if (is_string_number(offset)) {
            Current_Hot.clear();
            int num = Get_Reg_Tmp();
            Current_Hot.push_back(num);
            int offset_to_num = std::stoi(offset);
            int offsetx = offset_to_num * 4;
            mips_print_check("li " + register_num[num] + ", " + std::to_string(offsetx) + "\n");
            Load_Data(from_ptr);
            Current_Hot.push_back(All_Registers[from_ptr].real_register);
            mips_print_check("addu " + register_num[num] + ", " + register_num[num] + ", " +
                       register_num[All_Registers[from_ptr].real_register] + "\n");
            mips_print_check("sw " + register_num[num] + ", " + std::to_string(All_Registers[tar_reg].offset) + "($sp)\n");
            state[num] = false;
        } else {
            Current_Hot.clear();
            int num = Get_Reg_Tmp();
            Current_Hot.push_back(num);
            Load_Data(offset);
            Current_Hot.push_back(All_Registers[offset].real_register);
            mips_print_check("li " + register_num[num] + ", 4\n");
            mips_print_check("mul " + register_num[num] + ", " + register_num[num] + ", " + Get_Register_Position(offset) +
                       "\n");
            Load_Data(from_ptr);
            Current_Hot.push_back(All_Registers[from_ptr].real_register);
            mips_print_check("addu " + register_num[num] + ", " + register_num[num] + ", " +
                       register_num[All_Registers[from_ptr].real_register] + "\n");
            mips_print_check("sw " + register_num[num] + ", " + std::to_string(All_Registers[tar_reg].offset) + "($sp)\n");
        }
    }

    //腾空$v0寄存器
    void ClearV0() {
        state[0]=false;
        for (const auto &Register: All_Registers) {
            if (Register.second.current_place == "Register") {
                if (register_num[Register.second.real_register] == "$v0") {
                    std::string Virtual_Name = Register.second.virtual_register;
                    alloca_space -= 4;
                    int space = All_Registers.find(Virtual_Name)->second.space;
                    if (space == 4) {
                        mips_print_check("sw $v0, "+std::to_string(alloca_space)+"($sp)\n");
                    } else {
                        mips_print_check("sb $v0, "+std::to_string(alloca_space)+"($sp)\n");
                    }
                    All_Registers[Virtual_Name].real_register = 30;
                    All_Registers[Virtual_Name].current_place = "Stack";
                    All_Registers[Virtual_Name].offset = alloca_space;
                    return;
                }
            }
        }
    }

    //腾空$a0寄存器
    void ClearA0() {
        state[2]=false;
        for (const auto &Register: All_Registers) {
            if (Register.second.current_place == "Register") {
                if (register_num[Register.second.real_register] == "$a0") {
                    std::string Virtual_Name = Register.second.virtual_register;
                    alloca_space -= 4;
                    int space = All_Registers.find(Virtual_Name)->second.space;
                    if (space == 4) {
                        mips_print_check("sw $a0, "+std::to_string(alloca_space)+"($sp)\n");
                    } else {
                        mips_print_check("sb $a0, "+std::to_string(alloca_space)+"($sp)\n");
                    }
                    All_Registers[Virtual_Name].real_register = 30;
                    All_Registers[Virtual_Name].current_place = "Stack";
                    All_Registers[Virtual_Name].offset = alloca_space;
                    return;
                }
            }
        }
    }


    void print_all() {
        for (const auto &it: All_Registers) {
            printf("%s %d %s %s\n", it.first.c_str(), it.second.offset, it.second.current_place.c_str(),
                   register_num[it.second.real_register].c_str());
        }
    }
    
    bool check(){
        bool tmp_state[24];
        for(int i=0;i<24;++i){
            tmp_state[i]=false;
        }
        for(auto it:All_Registers){
            if(All_Registers[it.first].current_place=="Register"){
                if(tmp_state[All_Registers[it.first].real_register]){
                    return false;
                }
                else{
                    tmp_state[All_Registers[it.first].real_register]=true;
                }
            }
        }
        return true;
    }

private:
    std::map<std::string, Register_Match> All_Registers;
    int alloca_space;
};

//最终代码生成
void target_generate();

//将llvm中间代码逐行分割
void split_llvm_string();

//打印数据段
void print_data_segment();

//将全局变量也存到寄存器中
void handle_Global_Var(const std::string &llvm_line);

//处理开辟空间的情况
void handle_alloca(const std::string &llvm_line);

//处理store的情况
void handle_store(const std::string &llvm_line);

//处理load变量的情况
void handle_load(const std::string &llvm_line);

//处理加减乘除模
void handle_exp(const std::string &llvm_line);

//处理函数声明
void func_decl(const std::string &llvm_line);

//处理函数调用
void handle_call(const std::string &llvm_line);

//处理库函数
void print_declare_func();

//处理函数返回
void handle_return(const std::string &llvm_line);

//处理i32转i8
void handle_trunc(const std::string &llvm_line);

//处理i8转i32
void handle_zext(const std::string &llvm_line);

//处理跳转
void handle_br(const std::string &llvm_line);

//处理icmp
void handle_icmp(const std::string &llvm_line);

//处理从数组中获取元素
void handle_getelementptr(const std::string &llvm_line);

#endif //COMPILER_TARGET_CODE_H
