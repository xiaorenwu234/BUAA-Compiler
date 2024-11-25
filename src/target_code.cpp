//
// Created by 13069 on 2024/11/17.
//
#include<target_code.h>

bool state[24];

std::vector<std::string> llvm_result;

//全局变量
std::vector<std::string> global_data;

std::vector<int> Current_Hot;

int icmp_cnt = 0;

std::map<int, std::string> register_num = {
        {0,  "$v0"},
        {1,  "$v1"},
        {2,  "$a0"},
        {3,  "$a1"},
        {4,  "$a2"},
        {5,  "$a3"},
        {6,  "$t0"},
        {7,  "$t1"},
        {8,  "$t2"},
        {9,  "$t3"},
        {10, "$t4"},
        {11, "$t5"},
        {12, "$t6"},
        {13, "$t7"},
        {14, "$t8"},
        {15, "$t9"},
        {16, "$s0"},
        {17, "$s1"},
        {18, "$s2"},
        {19, "$s3"},
        {20, "$s4"},
        {21, "$s5"},
        {22, "$s6"},
        {23, "$s7"},
};

//寄存器池
Register_Pool registerPool;

Global_Pool global_pool;

void target_generate() {
    split_llvm_string();
    int llvm_line_cnt = (int) llvm_result.size();
    bool in_func = false;
    for (int i = 5; i < llvm_line_cnt; ++i) {
        if (llvm_result[i].find('{') != std::string::npos) {
            in_func = true;
        } else if (llvm_result[i].find('}') != std::string::npos) {
            in_func = false;
        }
            //全局变量
        else if (llvm_result[i].find('@') != std::string::npos && llvm_result[i].find('{') == std::string::npos &&
                 !in_func) {
            handle_Global_Var(llvm_result[i]);
        }
        if (llvm_result[i].find("main") != std::string::npos) {
            break;
        }
    }
    in_func = false;

    print_data_segment();
    mips_print("\n.text\n\n");
    print_declare_func();
    for (int i = 5; i < llvm_line_cnt; ++i) {
        //定义函数
        if (llvm_result[i].find("define") != std::string::npos) {
            registerPool.Clear_Pool();
            auto elements = Split_Line(llvm_result[i], ' ');
            std::string func_name;
            for (int j = 1; j < elements[3].size(); ++j) {
                func_name += elements[3][j];
            }
            std::string params_string;
            for (int j = 0; j < llvm_result[i].size(); ++j) {
                if (llvm_result[i][j] == '(') {
                    for (int w = j + 1; w < llvm_result[i].size(); ++w) {
                        if (llvm_result[i][w] == ')') {
                            break;
                        }
                        params_string += llvm_result[i][w];
                    }
                    break;
                }
            }
            std::vector<std::string> params;
            params = Split_Line(params_string, ',');
            for (const std::string &param: params) {
                std::vector<std::string> type_name = Split_Line(param, ' ');
                if (type_name[0].find("i32") != std::string::npos || type_name[0].find("i8") != std::string::npos) {
                    registerPool.Get_Free_Register(type_name[1], 4);
                } else {
                    registerPool.Get_Free_Register(type_name[1], 1);
                }
            }
        }
        if (llvm_result[i].find('{') != std::string::npos) {
            in_func = true;
            func_decl(llvm_result[i]);
        } else if (llvm_result[i].find('}') != std::string::npos) {
            in_func = false;
            mips_print("\n");
        }
        if (llvm_result[i].find('@') != std::string::npos && llvm_result[i].find('{') == std::string::npos &&
            !in_func) {
            continue;
        }
        if (in_func && llvm_result[i].find('{') == std::string::npos) {
            if (llvm_result[i].find("alloca") != std::string::npos) {
                handle_alloca(llvm_result[i]);
            } else if (llvm_result[i].find("store") != std::string::npos) {
                handle_store(llvm_result[i]);
            } else if (llvm_result[i].find("load") != std::string::npos) {
                handle_load(llvm_result[i]);
            } else if (llvm_result[i].find("call") != std::string::npos) {
                handle_call(llvm_result[i]);
            } else if (llvm_result[i].find("add") != std::string::npos ||
                       llvm_result[i].find("sub") != std::string::npos ||
                       llvm_result[i].find("mul") != std::string::npos ||
                       llvm_result[i].find("sdiv") != std::string::npos ||
                       llvm_result[i].find("srem") != std::string::npos) {
                handle_exp(llvm_result[i]);

            } else if (llvm_result[i].find("ret") != std::string::npos) {
                handle_return(llvm_result[i]);
            } else if (llvm_result[i].find(':') != std::string::npos) {
                mips_print(llvm_result[i] + "\n");
            } else if (llvm_result[i].find("trunc") != std::string::npos) {
                handle_trunc(llvm_result[i]);
            } else if (llvm_result[i].find("zext") != std::string::npos) {
                handle_zext(llvm_result[i]);
            } else if (llvm_result[i].find("br") != std::string::npos) {
                handle_br(llvm_result[i]);
            } else if (llvm_result[i].find("icmp") != std::string::npos) {
                handle_icmp(llvm_result[i]);
            } else if (llvm_result[i].find("getelementptr") != std::string::npos) {
                handle_getelementptr(llvm_result[i]);
            }
        }
    }
}

void handle_Global_Var(const std::string &llvm_line) {
    std::vector<std::string> elements = Split_Line(llvm_line, ' ');
    std::string element_name = elements[0];
    //数组
    if (llvm_line.find('[') != std::string::npos) {
        int size = std::stoi(elements[5]);
        global_pool.alloca_global_var(element_name, 4 * size);
    }
        //普通变量
    else {
        global_pool.alloca_global_var(element_name, 4);
    }
}

void handle_alloca(const std::string &llvm_line) {
    //声明数组
    if (llvm_line.find('[') != std::string::npos) {
        auto elements = Split_Line(llvm_line, ' ');
        std::string var_name = elements[0];
        int length = std::stoi(elements[4]);
        registerPool.Alloca_Sp(var_name, 4 * length);
    }
        //声明变量
    else {
        auto elements = Split_Line(llvm_line, ' ');
        std::string var_name = elements[0];
        registerPool.Alloca_Sp(var_name, 4);
    }
}

void handle_store(const std::string &llvm_line) {
    auto elements = Split_Line(llvm_line, ' ');
    auto from_type = elements[1];
    auto from_reg = elements[2];
    auto to_reg = elements[5];
    registerPool.store_ope(from_type, from_reg, to_reg);
}

void handle_load(const std::string &llvm_line) {
    auto elements = Split_Line(llvm_line, ' ');
    std::string target_name = elements[0];
    std::string from_name = elements[6];
    std::string type = elements[3];
    registerPool.load_ope(type, from_name, target_name);
}

void split_llvm_string() {
    std::ifstream file("llvm_ir.txt");
    std::string llvm_line;
    while (std::getline(file, llvm_line)) {
        llvm_result.push_back(llvm_line);  // 将每一行存入vector中
    }
    file.close();
}

void print_data_segment() {
    mips_print(".data\n");
    for (const auto &it: global_data) {
        mips_print(it);
    }
}

void handle_exp(const std::string &llvm_line) {
    auto elements = Split_Line(llvm_line, ' ');
    std::string result = elements[0];
    std::string left_arg = elements[4];
    std::string right_arg = elements[6];
    std::string ope = elements[2];
    registerPool.exp_ope(ope, left_arg, right_arg, result);
}

void func_decl(const std::string &llvm_line) {
    auto elements = Split_Line(llvm_line, ' ');
    std::string func_name;
    for (int i = 1; i < elements[3].size(); ++i) {
        func_name += elements[3][i];
    }
    mips_print(func_name + ":\n");
}

void handle_call(const std::string &llvm_line) {
    auto elements = Split_Line(llvm_line, ' ');
    std::string type;
    std::string tar_reg;
    std::string func_name;
    if (elements[1].find("void") != std::string::npos) {
        type = "void";
        for (int i = 1; i < elements[2].size(); ++i) {
            func_name += elements[2][i];
        }
    } else if (elements[1].find("i32") != std::string::npos) {
        type = "i32";
        for (int i = 1; i < elements[2].size(); ++i) {
            func_name += elements[2][i];
        }
    } else if (elements[1].find("i8") != std::string::npos) {
        type = "i8";
        for (int i = 1; i < elements[2].size(); ++i) {
            func_name += elements[2][i];
        }
    }
    if (elements[0].find('%') != std::string::npos) {
        if (elements[3].find("i32") != std::string::npos) {
            type = "i32";
        } else if (elements[3].find("i8") != std::string::npos) {
            type = "i8";
        }
        tar_reg = elements[0];
        for (int i = 1; i < elements[4].size(); ++i) {
            func_name += elements[4][i];
        }
    }
    std::string param_string;
    for (int i = 0; i < llvm_line.size(); ++i) {
        if (llvm_line[i] == '(') {
            for (int j = i + 1; j < llvm_line.size(); ++j) {
                if (llvm_line[j] == ')') {
                    break;
                }
                param_string += llvm_line[j];
            }
            break;
        }
    }
    auto pars = Split_Line(param_string, ',');
    std::vector<std::pair<std::string, std::string>> params;
    for (const auto &par: pars) {
        auto type_name = Split_Line(par, ' ');
        params.emplace_back(type_name[0], type_name[1]);
    }
    registerPool.call_ope(func_name, params, type, tar_reg);
}

void print_declare_func() {
    mips_print("entry:\n");
    mips_print("move $fp, $sp\n");
    mips_print("addi $sp, $sp, " + std::to_string(global_pool.get_alloca_space()) + "\n");
    for(int i=global_pool.get_alloca_space();i<0;i+=4){
        mips_print("sw $zero, "+std::to_string(i)+"($fp)\n");
    }
    mips_print("jal main\n");
    mips_print("li $v0, 10\n");
    mips_print("syscall\n\n");

    mips_print("getint:\n");
    mips_print("li $v0, 5\n");
    mips_print("syscall\n");
    mips_print("jr $ra\n\n");

    mips_print("getchar:\n");
    mips_print("li $v0, 12\n");
    mips_print("syscall\n");
    mips_print("jr $ra\n\n");


    mips_print("putch:\n");
    mips_print("move $a0, $v0\n");
    mips_print("li $v0, 11\n");
    mips_print("syscall\n");
    mips_print("jr $ra\n\n");

    mips_print("putint:\n");
    mips_print("move $a0, $v0\n");
    mips_print("li $v0, 1\n");
    mips_print("syscall\n");
    mips_print("jr $ra\n\n");
}

void handle_return(const std::string &llvm_line) {
    auto elements = Split_Line(llvm_line, ' ');
    registerPool.ret_ope(elements[1], elements[2]);
}

void handle_trunc(const std::string &llvm_line) {
    auto elements = Split_Line(llvm_line, ' ');
    auto to_reg = elements[0];
    auto from_reg = elements[4];
    auto from_type = elements[3];
    auto to_type = elements[6];
    registerPool.trunc_ope(from_reg, to_reg, from_type, to_type);
}

void handle_zext(const std::string &llvm_line) {
    auto elements = Split_Line(llvm_line, ' ');
    auto to_reg = elements[0];
    auto from_reg = elements[4];
    auto from_type = elements[3];
    auto to_type = elements[6];
    registerPool.zext_ope(from_reg, to_reg, from_type, to_type);
}

void handle_br(const std::string &llvm_line) {
    auto elements = Split_Line(llvm_line, ' ');
    if (elements.size() == 3) {
        registerPool.unconditional_jump(elements[2]);
    } else {
        registerPool.conditional_jump(elements[2], elements[5], elements[8]);
    }
}

void handle_icmp(const std::string &llvm_line) {
    auto elements = Split_Line(llvm_line, ' ');
    registerPool.icmp_ope(elements[0], elements[3], elements[5], elements[7]);
}

void handle_getelementptr(const std::string &llvm_line) {
    auto elements = Split_Line(llvm_line, ' ');
    if (elements.size() == 11) {
        std::string tar_reg = elements[0];
        std::string from_ptr = elements[7];
        std::string offset = elements[10];
        std::string type = elements[4];
        registerPool.not_need_offset_ptr(tar_reg, from_ptr, offset, type);
    } else {
        std::string tar_reg = elements[0];
        std::string from_ptr = elements[15];
        std::string offset = elements[21];
        std::string type = elements[7];
        registerPool.need_offset_ptr(tar_reg, from_ptr, offset, type);
    }
}