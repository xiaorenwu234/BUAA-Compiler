//
// Created by 13069 on 2024/11/6.
//
#include<intermediate_code.h>

bool is_global = true;

int register_cnt = 1;

std::vector<std::string> assign_strs;

int store_register_cnt = 0;

int get_cur_num(const std::string &input_str) {
    int ans = 0;
    for (int i = 0; i < input_str.size(); ++i) {
        if (input_str[i] == '%') {
            int num = 0;
            for (int j = i + 1; i < input_str.size(); ++j) {
                if (input_str[j] > '9' || input_str[j] < '0') {
                    i = j - 1;
                    break;
                } else {
                    num = num * 10 + input_str[j] - '0';
                }
            }
            ans = ans > num ? ans : num;
        }
    }
    return ans;
}

void Define_Function(const std::string &FuncType, const std::string &FuncName) {
    Add_String("define ");
    if (FuncType == "int") {
        Add_String("dso_local i32 ", true);
    } else if (FuncType == "void") {
        Add_String("dso_local void ", true);
    } else {
        Add_String("dso_local i8 ", true);
    }
    Add_String("@" + FuncName, true);
    register_cnt = 0;
}

void Define_Function_Params(const std::string &ParamType, const std::string &ParamName) {
    if (ParamType == "int") {
        Add_String("i32 ", true);
    } else if (ParamType == "char") {
        Add_String("i8 ", true);
    } else if (ParamType == "intarray") {
        Add_String("i32* ", true);
    } else if (ParamType == "chararray") {
        Add_String("i8* ", true);
    }
    Add_String("%" + ParamName, true);
}

void Add_String(const std::string &str, bool need_print) {
    if (need_print || !is_global) {
        llvm_print(str);
    } else {
        assign_strs.push_back(str);
    }
}

void Define_Varible(const std::string &VarType, const std::string &VarName, int ArrayLength) {
    if (VarType == "int" || VarType == "constint" || VarType == "Int" || VarType == "ConstInt") {
        std::string tmpx = is_global ? "@" : "%";
        std::string global_str = is_global ? "dso_local global " : "alloca ";
        Add_String(tmpx + VarName, true);
        Add_String(" = ", true);
        Add_String(global_str, true);
        Add_String("i32", true);
        tmpx = is_global ? " 0\n" : "\n";
        Add_String(tmpx, true);
    } else if (VarType == "char" || VarType == "constchar" || VarType == "Char" || VarType == "ConstChar") {
        std::string tmpx = is_global ? "@" : "%";
        std::string global_str = is_global ? "dso_local global " : "alloca ";
        Add_String(tmpx + VarName, true);
        Add_String(" = ", true);
        Add_String(global_str, true);
        Add_String("i8", true);
        tmpx = is_global ? " 0\n" : "\n";
        Add_String(tmpx, true);
    } else if (VarType == "chararray" || VarType == "constchararray" || VarType == "CharArray" ||
               VarType == "ConstCharArray") {
        std::string tmpx = is_global ? "@" : "%";
        std::string global_str = is_global ? "dso_local global " : "alloca ";
        std::string to_zero = is_global ? "zeroinitializer" : "";
        Add_String(tmpx + VarName, true);
        Add_String(" = ", true);
        Add_String(global_str, true);
        Add_String(" [ " + std::to_string(ArrayLength) + " x i8 ] " + to_zero + "\n", true);
    } else if (VarType == "intarray" || VarType == "constintarray" || VarType == "IntArray" ||
               VarType == "ConstIntArray") {
        std::string tmpx = is_global ? "@" : "%";
        std::string global_str = is_global ? "dso_local global " : "alloca ";
        std::string to_zero = is_global ? "zeroinitializer" : "";
        Add_String(tmpx + VarName, true);
        Add_String(" = ", true);
        Add_String(global_str, true);
        Add_String(" [ " + std::to_string(ArrayLength) + " x i32 ] " + to_zero + "\n", true);
    }
}

void
Assign_Varible_Const_NotArray(const std::string &VarName, const std::string &VarType, bool global_state, int value) {
    std::string tmpx = global_state ? "@" : "%";
    if (VarType.find("Ptr") != std::string::npos) {
        Register_Pair registerPair(tmpx + VarName, VarType);
        registerPair = LoadPtr(registerPair);
        std::string typex = VarType.find("Int") != std::string::npos ? "i32" : "i8";
        Add_String(
                "store " + typex + " " + std::to_string(value) + " , " + typex + "* " + registerPair.Register + "\n");
    } else {
        std::string typex = (VarType == "ConstInt" || VarType == "Int") ? "i32" : "i8";
        Add_String("store " + typex + " " + std::to_string(value) + " , " + typex + "* " + tmpx + VarName + "\n");
    }
}

//有目标类型的Zext
Register_Pair Zext_Register_Target(const Register_Pair &registerPair, const std::string &target) {
    Register_Pair return_register("%" + std::to_string(register_cnt), target);
    if (registerPair.Type == "Bool") {
        if (target == "Char") {
            Add_String("%" + std::to_string((register_cnt)) + " = zext i1 " + registerPair.Register + " to i8\n");
        } else if (target == "Int") {
            Add_String("%" + std::to_string((register_cnt)) + " = zext i1 " + registerPair.Register + " to i32\n");
        }
    } else {
        Add_String("%" + std::to_string((register_cnt)) + " = zext i8 " + registerPair.Register + " to i32\n");
    }
    register_cnt++;
    return return_register;
}

Register_Pair Trunc_Register_Bool(const Register_Pair &registerPair) {
    Register_Pair return_register("%" + std::to_string(register_cnt), "Bool");
    if (registerPair.Type != "Bool") {
        if (registerPair.Type.find("Char") != std::string::npos) {
            Add_String("%" + std::to_string((register_cnt)) + " = trunc i8 " + registerPair.Register + " to i1\n");
        } else if (registerPair.Type.find("Int") != std::string::npos) {
            Add_String("%" + std::to_string((register_cnt)) + " = trunc i32 " + registerPair.Register + " to i1\n");
        }
        register_cnt++;
    }
    return return_register;
}

Register_Pair LoadPtr(const Register_Pair &registerPair) {
    Register_Pair return_register;
    if (registerPair.Type == "IntPtr") {
        Add_String("%" + std::to_string(register_cnt) + " = load i32 , i32* " + registerPair.Register + "\n");
        return_register.Type = "Int";
        return_register.Register = "%" + std::to_string(register_cnt);
        register_cnt++;
    } else {
        Add_String("%" + std::to_string(register_cnt) + " = load i8 , i8* " + registerPair.Register + "\n");
        return_register.Type = "Char";
        return_register.Register = "%" + std::to_string(register_cnt);
        register_cnt++;
    }
    return return_register;
}

Register_Pair Handle_Exp(Register_Pair LeftArg, Register_Pair RightArg, const std::string &ope) {
    Register_Pair registerPair;
    registerPair.Type = "Int";
    if (LeftArg.Type.find("Ptr") != std::string::npos) {
        LeftArg = LoadPtr(LeftArg);
    }
    if (RightArg.Type.find("Ptr") != std::string::npos) {
        RightArg = LoadPtr(RightArg);
    }
    if (ope == "+") {
        if (LeftArg.Type == "Char" || LeftArg.Type == "ConstChar") {
            LeftArg = Zext_Register(LeftArg);
        }
        if (RightArg.Type == "Char" || RightArg.Type == "ConstChar") {
            RightArg = Zext_Register(RightArg);
        }
        Add_String(
                "%" + std::to_string(register_cnt) + " =  add i32 " + LeftArg.Register + " , " + RightArg.Register +
                "\n");
    } else if (ope == "-") {
        if (LeftArg.Type == "Char" || LeftArg.Type == "ConstChar") {
            LeftArg = Zext_Register(LeftArg);
        }
        if (RightArg.Type == "Char" || RightArg.Type == "ConstChar") {
            RightArg = Zext_Register(RightArg);
        }
        Add_String(
                "%" + std::to_string(register_cnt) + " =  sub i32 " + LeftArg.Register + " , " + RightArg.Register +
                "\n");
    } else if (ope == "*") {
        if (LeftArg.Type == "Char" || LeftArg.Type == "ConstChar") {
            LeftArg = Zext_Register(LeftArg);
        }
        if (RightArg.Type == "Char" || RightArg.Type == "ConstChar") {
            RightArg = Zext_Register(RightArg);
        }
        Add_String(
                "%" + std::to_string(register_cnt) + " =  mul i32 " + LeftArg.Register + " , " + RightArg.Register +
                "\n");
    } else if (ope == "/") {
        if (LeftArg.Type == "Char" || LeftArg.Type == "ConstChar") {
            LeftArg = Zext_Register(LeftArg);
        }
        if (RightArg.Type == "Char" || RightArg.Type == "ConstChar") {
            RightArg = Zext_Register(RightArg);
        }
        Add_String(
                "%" + std::to_string(register_cnt) + " =  sdiv i32 " + LeftArg.Register + " , " + RightArg.Register +
                "\n");
    } else if (ope == "%") {
        if (LeftArg.Type == "Char" || LeftArg.Type == "ConstChar") {
            LeftArg = Zext_Register(LeftArg);
        }
        if (RightArg.Type == "Char" || RightArg.Type == "ConstChar") {
            RightArg = Zext_Register(RightArg);
        }
        Add_String(
                "%" + std::to_string(register_cnt) + " =  srem i32 " + LeftArg.Register + " , " + RightArg.Register +
                "\n");
    } else if (ope == "!") {
        std::string tmpx;
        if (RightArg.Type.find("Char") != std::string::npos) {
            tmpx = "i8";
        } else if (RightArg.Type == "Bool") {
            tmpx = "i1";
        } else {
            tmpx = "i32";
        }
        registerPair.Type = "Bool";
        Add_String("%" + std::to_string(register_cnt) + " = icmp eq " + tmpx + " " + RightArg.Register + " , 0\n",
                   true);
    } else if (ope == "||") {
        if (LeftArg.Type != "Bool") {
            LeftArg = To_Bool(LeftArg);
        }
        if (RightArg.Type != "Bool") {
            RightArg = To_Bool(RightArg);
        }
        Add_String(
                "%" + std::to_string(register_cnt) + " = or i1 " + LeftArg.Register + " , " + RightArg.Register +
                "\n");
        registerPair.Type = "Bool";
    } else if (ope == "&&") {
        if (LeftArg.Type != "Bool") {
            LeftArg = To_Bool(LeftArg);
        }
        if (RightArg.Type != "Bool") {
            RightArg = To_Bool(RightArg);
        }
        Add_String(
                "%" + std::to_string(register_cnt) + " = and i1 " + LeftArg.Register + " , " + RightArg.Register +
                "\n");
        registerPair.Type = "Bool";
    } else if (ope == "==" || ope == "!=" || ope == ">" || ope == "<" || ope == "<=" || ope == ">=") {
        std::string tmpx;
        if (ope == "==") {
            tmpx = "eq";
        } else if (ope == "!=") {
            tmpx = "ne";
        } else if (ope == ">") {
            tmpx = "sgt";
        } else if (ope == "<") {
            tmpx = "slt";
        } else if (ope == ">=") {
            tmpx = "sge";
        } else if (ope == "<=") {
            tmpx = "sle";
        }
        if (LeftArg.Type == "ConstInt" || LeftArg.Type == "Int") {
            if (RightArg.Type == "ConstInt" || RightArg.Type == "Int") {
                Add_String(
                        "%" + std::to_string(register_cnt) + " = icmp " + tmpx + " i32 " + LeftArg.Register + " , " +
                        RightArg.Register + "\n");
            } else {
                RightArg = Zext_Register(RightArg);
                Add_String(
                        "%" + std::to_string(register_cnt) + " = icmp " + tmpx + " i32 " + LeftArg.Register + " , " +
                        RightArg.Register + "\n");
            }
        } else if (LeftArg.Type == "ConstChar" || LeftArg.Type == "Char") {
            if (RightArg.Type == "ConstInt" || RightArg.Type == "Int") {
                LeftArg = Zext_Register(LeftArg);
                Add_String(
                        "%" + std::to_string(register_cnt) + " = icmp " + tmpx + " i32 " + LeftArg.Register + " , " +
                        RightArg.Register + "\n");
            } else if (RightArg.Type == "Bool") {
                RightArg = Zext_Register_Target(RightArg, "Char");
                Add_String(
                        "%" + std::to_string(register_cnt) + " = icmp " + tmpx + " i8 " + LeftArg.Register + " , " +
                        RightArg.Register + "\n");
            } else {
                Add_String(
                        "%" + std::to_string(register_cnt) + " = icmp " + tmpx + " i8 " + LeftArg.Register + " , " +
                        RightArg.Register + "\n");
            }
        } else {
            if (RightArg.Type == "Bool") {
                Add_String(
                        "%" + std::to_string(register_cnt) + " = icmp " + tmpx + " i1 " + LeftArg.Register + " , " +
                        RightArg.Register + "\n");
            } else if (RightArg.Type == "Char" || RightArg.Type == "ConstChar") {
                LeftArg = Zext_Register_Target(LeftArg, "Char");
                Add_String(
                        "%" + std::to_string(register_cnt) + " = icmp " + tmpx + " i8 " + LeftArg.Register + " , " +
                        RightArg.Register + "\n");
            } else {
                LeftArg = Zext_Register(LeftArg);
                Add_String(
                        "%" + std::to_string(register_cnt) + " = icmp " + tmpx + " i32 " + LeftArg.Register + " , " +
                        RightArg.Register + "\n");
            }
        }
        registerPair.Type = "Bool";
    }
    registerPair.Register = "%" + std::to_string(register_cnt);
    register_cnt++;
    return registerPair;
}

Register_Pair Load_Varible(const std::string &VarName, const std::string &VarType, bool global) {
    Register_Pair registerPair("%" + std::to_string(register_cnt), VarType);
    if (VarType == "Bool") {
        std::string tmpx = global ? "@" : "%";
        Add_String("%" + std::to_string(register_cnt) + " = load i1 , i1* " + tmpx + VarName + "\n");
    } else if (VarType == "ConstInt" || VarType == "Int" || VarType == "IntPtr") {
        std::string tmpx = global ? "@" : "%";
        Add_String("%" + std::to_string(register_cnt) + " = load i32 , i32* " + tmpx + VarName + "\n");
    } else {
        std::string tmpx = global ? "@" : "%";
        Add_String("%" + std::to_string(register_cnt) + " = load i8 , i8* " + tmpx + VarName + "\n");
    }
    register_cnt++;
    return registerPair;
}

Register_Pair Load_Varible_Array(const std::string &VarName, const std::string &VarType, bool global, int ArrayLength) {
    Register_Pair registerPair("%" + std::to_string(register_cnt), VarType);
    if (VarType.find("Ptr") != std::string::npos) {
        if (VarType.find("Int") != std::string::npos) {
            std::string tmpx = global ? "@" : "%";
            Add_String("%" + std::to_string(register_cnt) + " = load i32* , i32** " + tmpx + VarName + "\n");
        } else if (VarType.find("Char") != std::string::npos) {
            std::string tmpx = global ? "@" : "%";
            Add_String("%" + std::to_string(register_cnt) + " = load i8* , i8** " + tmpx + VarName + "\n");
        }
    } else {
        if (VarType.find("Int") != std::string::npos) {
            std::string tmpx = global ? "@" : "%";
            Add_String(
                    "%" + std::to_string(register_cnt) + " = getelementptr inbounds [ " + std::to_string(ArrayLength) +
                    " x i32 ] , [ " + std::to_string(ArrayLength) + " x i32 ]* " + tmpx + VarName +
                    " , i32 0 , i32 0\n");
        } else if (VarType.find("Char") != std::string::npos) {
            std::string tmpx = global ? "@" : "%";
            Add_String(
                    "%" + std::to_string(register_cnt) + " = getelementptr inbounds [ " + std::to_string(ArrayLength) +
                    " x i8 ] , [ " + std::to_string(ArrayLength) + " x i8 ]* " + tmpx + VarName + " , i32 0 , i32 0\n");
        }
    }
    register_cnt++;
    return registerPair;
}

void
Assign_Varible_Not_Array(Register_Pair leftPair,
                         Register_Pair registerPair) {
    if (registerPair.Type.find("Ptr") != std::string::npos) {
        registerPair = LoadPtr(registerPair);
    }
    if (leftPair.Type.find("Int") != std::string::npos && registerPair.Type.find("Char") != std::string::npos) {
        registerPair = Zext_Register(registerPair);
        Add_String("store i32 " + registerPair.Register + " , i32* " + leftPair.Register + "\n");
    } else if (leftPair.Type.find("Char") != std::string::npos && registerPair.Type.find("Int") != std::string::npos) {
        registerPair = Trunc_Register(registerPair);
        Add_String("store i8 " + registerPair.Register + " , i8* " + leftPair.Register + "\n");
    } else if (leftPair.Type.find("Int") != std::string::npos) {
        Add_String("store i32 " + registerPair.Register + " , i32* " + leftPair.Register + "\n");
    } else {
        Add_String("store i8 " + registerPair.Register + " , i8* " + leftPair.Register + "\n");
    }
}

Register_Pair Zext_Register(const Register_Pair &registerPair) {
    Register_Pair return_register("%" + std::to_string(register_cnt), "Int");
    if (registerPair.Type == "Bool") {
        Add_String("%" + std::to_string((register_cnt)) + " = zext i1 " + registerPair.Register + " to i32\n");
    } else {
        Add_String("%" + std::to_string((register_cnt)) + " = zext i8 " + registerPair.Register + " to i32\n");
    }
    register_cnt++;
    return return_register;
}

Register_Pair Trunc_Register(const Register_Pair &registerPair) {
    Register_Pair return_register("%" + std::to_string(register_cnt), "Char");
    Add_String("%" + std::to_string(register_cnt) + " = trunc i32 " + registerPair.Register + " to i8\n");
    register_cnt++;
    return return_register;
}

void Array_Init_Const(const std::string &VarName, const std::string &VarType, int ArrayLength, bool global,
                      const std::vector<int> &values) {
    if (VarType.find("Char") != std::string::npos) {
        std::string tmpx = global ? "@" : "%";
        for (int i = 0; i < values.size(); ++i) {
            Add_String(
                    "%" + std::to_string(register_cnt) + " = getelementptr inbounds [ " + std::to_string(ArrayLength) +
                    " x i8 ] , [ " + std::to_string(ArrayLength) + " x i8 ]* " + tmpx + VarName + " , i8 0 , i8 " +
                    std::to_string(i) + "\n");
            Add_String(
                    "store i8 " + std::to_string(values[i]) + " , i8* %" + std::to_string(register_cnt) + "\n");
            register_cnt++;
        }
    } else {
        std::string tmpx = global ? "@" : "%";
        for (int i = 0; i < values.size(); ++i) {
            Add_String(
                    "%" + std::to_string(register_cnt) + " = getelementptr inbounds [ " + std::to_string(ArrayLength) +
                    " x i32 ] , [ " + std::to_string(ArrayLength) + " x i32 ]* " + tmpx + VarName + " , i32 0 , i32 " +
                    std::to_string(i) + "\n");
            Add_String("store i32 " + std::to_string(values[i]) + " , i32* %" + std::to_string(register_cnt) +
                       "\n");
            register_cnt++;
        }
    }
}


void Array_Init_Var(const std::string &VarName, const std::string &VarType, int ArrayLength, bool global,
                    const std::vector<Register_Pair> &values) {
    if (VarType.find("Char") != std::string::npos) {
        std::string tmpx = global ? "@" : "%";
        for (int i = 0; i < values.size(); ++i) {
            Register_Pair tmp_reg = values[i];
            if (tmp_reg.Type.find("Ptr") != std::string::npos) {
                tmp_reg = LoadPtr(tmp_reg);
            }
            if (values[i].Type == "Int" || values[i].Type == "ConstInt") {
                tmp_reg = Trunc_Register(values[i]);
            }
            Add_String(
                    "%" + std::to_string(register_cnt) + " = getelementptr inbounds [ " + std::to_string(ArrayLength) +
                    " x i8 ] , [ " + std::to_string(ArrayLength) + " x i8 ]* " + tmpx + VarName + " , i32 0 , i32 " +
                    std::to_string(i) + "\n");
            Add_String("store i8 " + tmp_reg.Register + " , i8* %" + std::to_string(register_cnt) + "\n");
            register_cnt++;
        }
    } else {
        std::string tmpx = global ? "@" : "%";
        for (int i = 0; i < values.size(); ++i) {
            Register_Pair tmp_reg = values[i];
            if (tmp_reg.Type.find("Ptr") != std::string::npos) {
                tmp_reg = LoadPtr(tmp_reg);
            }
            if (values[i].Type == "Char" || values[i].Type == "Char") {
                tmp_reg = Zext_Register(values[i]);
            }
            Add_String(
                    "%" + std::to_string(register_cnt) + " = getelementptr inbounds [ " + std::to_string(ArrayLength) +
                    " x i32 ] , [ " + std::to_string(ArrayLength) + " x i32 ]* " + tmpx + VarName + " , i32 0 , i32 " +
                    std::to_string(i) + "\n");
            Add_String("store i32 " + tmp_reg.Register + " , i32* %" + std::to_string(register_cnt) + "\n");
            register_cnt++;
        }
    }
}

Register_Pair
Load_Current_Array_Element(std::string VarName, std::string VarType, bool global, int Array_Length,
                           Register_Pair registerPair) {
    Register_Pair return_register;
    if (registerPair.Type == "Char" || registerPair.Type == "ConstChar") {
        registerPair = Zext_Register(registerPair);
    }
    if (registerPair.Type.find("Ptr") != std::string::npos) {
        registerPair = LoadPtr(registerPair);
    }
    if (VarType.find("Ptr") != std::string::npos) {
        if (VarType == "IntArrayPtr") {
            std::string tmpx = global ? "@" : "%";
            Add_String("%" + std::to_string(register_cnt) + " = load i32* , i32** " + tmpx + VarName + "\n");
            VarName = std::to_string(register_cnt);
            register_cnt++;
            VarType = "IntArray";
            return_register.Type = "Int";
        } else if (VarType == "CharArrayPtr") {
            std::string tmpx = global ? "@" : "%";
            Add_String("%" + std::to_string(register_cnt) + " = load i8* , i8** " + tmpx + VarName + "\n");
            VarName = std::to_string(register_cnt);
            register_cnt++;
            VarType = "CharArray";
            return_register.Type = "Char";
        }
    }
    if (VarType.find("Char") != std::string::npos) {
        std::string tmpx = global ? "@" : "%";
        if (Array_Length == 0) {
            Add_String(
                    "%" + std::to_string(register_cnt) + " = getelementptr inbounds i8 , i8* " + tmpx + VarName +
                    " , i32 " +
                    registerPair.Register + "\n");
        } else {
            Add_String(
                    "%" + std::to_string(register_cnt) + " = getelementptr inbounds [ " + std::to_string(Array_Length) +
                    " x i8 ] , [ " + std::to_string(Array_Length) + " x i8 ]* " + tmpx + VarName + " , i32 0 , i32 " +
                    registerPair.Register + "\n");
        }
        return_register.Type = "CharPtr";
    } else {
        std::string tmpx = global ? "@" : "%";
        if (Array_Length == 0) {
            Add_String(
                    "%" + std::to_string(register_cnt) + " = getelementptr inbounds i32 , i32* " + tmpx + VarName +
                    " , i32 " +
                    registerPair.Register + "\n");
        } else {
            Add_String(
                    "%" + std::to_string(register_cnt) + " = getelementptr inbounds [ " + std::to_string(Array_Length) +
                    " x i32 ] , [ " + std::to_string(Array_Length) + " x i32 ]* " + tmpx + VarName + " , i32 0 , i32 " +
                    registerPair.Register + "\n");
        }
        return_register.Type = "IntPtr";
    }
    return_register.Register = "%" + std::to_string(register_cnt);
    register_cnt++;
    return return_register;
}

Register_Pair Call_Func(const std::string &FuncName, const std::string &FuncType, std::vector<std::string> par_types,
                        std::vector<Register_Pair> par_regs) {
    Register_Pair registerPair;
    std::string print_str;
    print_str += "call ";
    if (FuncType == "VoidFunc") {
        print_str += "void @" + FuncName + " ( ";
        registerPair.Type = "Void";
    } else if (FuncType == "IntFunc") {
        print_str += "i32 @" + FuncName + " ( ";
        registerPair.Type = "Int";
    } else if (FuncType == "CharFunc") {
        print_str += "i8 @" + FuncName + " ( ";
        registerPair.Type = "Char";
    }
    int length = par_types.size();
    for (int i = 0; i < length; ++i) {
        if (par_types[i] == "Int" &&
            (par_regs[i].Type == "Char" || par_regs[i].Type == "ConstChar" || par_regs[i].Type == "CharPtr")) {
            if (par_regs[i].Type.find("Ptr") != std::string::npos) {
                par_regs[i] = LoadPtr(par_regs[i]);
            }
            par_regs[i] = Zext_Register(par_regs[i]);
        } else if (par_types[i] == "Char" &&
                   (par_regs[i].Type == "Int" || par_regs[i].Type == "ConstInt" || par_regs[i].Type == "IntPtr")) {
            if (par_regs[i].Type.find("Ptr") != std::string::npos) {
                par_regs[i] = LoadPtr(par_regs[i]);
            }
            par_regs[i] = Trunc_Register(par_regs[i]);
        }
        if (par_types[i] == "Int") {
            if (par_regs[i].Type.find("Ptr") != std::string::npos) {
                par_regs[i] = LoadPtr(par_regs[i]);
            }
            print_str += "i32 " + par_regs[i].Register;
        }
        if (par_types[i] == "Char") {
            if (par_regs[i].Type.find("Ptr") != std::string::npos) {
                par_regs[i] = LoadPtr(par_regs[i]);
            }
            print_str += "i8 " + par_regs[i].Register;
        }
        if (par_types[i].find("IntArray") != std::string::npos) {
            print_str += "i32* " + par_regs[i].Register;
        }
        if (par_types[i].find("CharArray") != std::string::npos) {
            print_str += "i8* " + par_regs[i].Register;
        }
        if (i != length - 1) {
            print_str += " , ";
        }
    }
    print_str += " )\n";
    if (registerPair.Type != "Void") {
        registerPair.Register = "%" + std::to_string(register_cnt);
        Add_String(registerPair.Register + " = " + print_str);
        register_cnt++;
    } else {
        Add_String(print_str);
    }
    return registerPair;
}

void Func_Return(const std::string &FuncType, Register_Pair registerPair) {
    if (registerPair.Type.find("Ptr") != std::string::npos) {
        registerPair = LoadPtr(registerPair);
    }
    if (FuncType == "IntFunc") {
        if (registerPair.Type == "Char" || registerPair.Type == "ConstChar") {
            registerPair = Zext_Register(registerPair);
        }
        Add_String("ret i32 " + registerPair.Register + "\n");
    } else if (FuncType == "CharFunc") {
        if (registerPair.Type == "Int" || registerPair.Type == "ConstInt") {
            registerPair = Trunc_Register(registerPair);
        }
        Add_String("ret i8 " + registerPair.Register + "\n");
    }
}

Register_Pair To_Bool(const Register_Pair &registerPair) {
    Register_Pair return_register("%" + std::to_string(register_cnt), "Bool");
    if (registerPair.Type == "Int" || registerPair.Type == "ConstInt") {
        Add_String("%" + std::to_string(register_cnt) + " = icmp ne i32 " + registerPair.Register + " , 0\n");
    }
    register_cnt++;
    return return_register;
}

void
If_Jump(Register_Pair registerPair, int depth, const std::string &Then_Register, const std::string &Else_Register) {
    if (registerPair.Type != "Bool") {
        registerPair = To_Bool(registerPair);
    }
    Add_String("br i1 " + registerPair.Register + " , label " + Then_Register + " , label " + Else_Register +
               "\n");
}

void Print_Global() {
    int num = 0;
    for (auto it: assign_strs) {
        Add_String(it);
        num = num > get_cur_num(it) ? num : get_cur_num(it);
    }
    assign_strs.clear();
    register_cnt = num;
}

void Assign_Varible_Ptr_Array(const Register_Pair &LeftRegister, const Register_Pair &RightRegister) {
    if (LeftRegister.Type.find("Int") != std::string::npos) {
        Add_String("store i32* " + RightRegister.Register + " , i32** " + LeftRegister.Register + "\n");
    } else {
        Add_String("store i8* " + RightRegister.Register + " , i8** " + LeftRegister.Register + "\n");
    }
}

void Define_Ptr(const std::string &VarType, const std::string &VarName) {
    if (VarType.find("Int") != std::string::npos) {
        Add_String("%" + VarName + " = alloca i32*\n");
    } else {
        Add_String("%" + VarName + " = alloca i8*\n");
    }
}

void Move_FParams(const std::string &FormRegister, const std::string &NewRegister, const std::string &VarType) {
    if (VarType.find("Ptr") == std::string::npos) {
        Define_Varible(VarType, NewRegister);
        Register_Pair LeftRegister("%" + FormRegister, VarType), registerPair("%" + NewRegister, VarType);
        Assign_Varible_Not_Array(registerPair, LeftRegister);
    } else {
        if (VarType.find("Int") != std::string::npos) {
            Define_Ptr("IntPtr", NewRegister);
            Register_Pair LeftRegister("%" + FormRegister, VarType), registerPair("%" + NewRegister, "IntPtr");
            Assign_Varible_Ptr_Array(registerPair, LeftRegister);
        } else {
            Define_Ptr("CharPtr", NewRegister);
            Register_Pair LeftRegister("%" + FormRegister, VarType), registerPair("%" + NewRegister, "CharPtr");
            Assign_Varible_Ptr_Array(registerPair, LeftRegister);
        }
    }
}

void Print(const std::string &FormatString, std::vector<Register_Pair> OutPars) {
    int i = 1;
    int cur_par = 0;
    int tmp;

    while (i < FormatString.size() - 1) {
        if (FormatString[i] == '\\') {
            if (i + 1 < FormatString.size()) {
                switch (FormatString[i + 1]) {
                    case 'n':
                        tmp = '\n';
                        Call_PutChar(std::to_string(tmp));
                        i += 2;
                        break;
                    case 't':
                        tmp = '\t';
                        Call_PutChar(std::to_string(tmp));
                        i += 2;
                        break;
                    case '\\':
                        tmp = '\\';
                        Call_PutChar(std::to_string(tmp));
                        i += 2;
                        break;
                    case '\"':
                        tmp = '\"';
                        Call_PutChar(std::to_string(tmp));
                        i += 2;
                        break;
                    case '\'':
                        tmp = '\'';
                        Call_PutChar(std::to_string(tmp));
                        i += 2;
                        break;
                    case '0':
                        return;
                    case 'a':
                        tmp = '\a';
                        Call_PutChar(std::to_string(tmp));
                        i += 2;
                        break;
                    case 'b':
                        tmp = '\b';
                        Call_PutChar(std::to_string(tmp));
                        i += 2;
                        break;
                    case 'v':
                        tmp = '\v';
                        Call_PutChar(std::to_string(tmp));
                        i += 2;
                        break;
                    case 'f':
                        tmp = '\f';
                        Call_PutChar(std::to_string(tmp));
                        i += 2;
                        break;
                    default:
                        i++;
                        break;
                }
            } else {
                tmp = FormatString[i];
                Call_PutChar(std::to_string(tmp));
                i++;
            }
        } else if (FormatString[i] == '%') {
            if (i + 1 < FormatString.size() - 1) {
                if (FormatString[i + 1] == 'c') {
                    if (OutPars[cur_par].Type.find("Ptr") != std::string::npos) {
                        OutPars[cur_par] = LoadPtr(OutPars[cur_par]);
                    }
                    if (OutPars[cur_par].Type.find("Int") == std::string::npos) {
                        OutPars[cur_par] = Zext_Register(OutPars[cur_par]);
                    }
                    Call_PutChar(OutPars[cur_par].Register);
                    cur_par++;
                    i += 2;
                } else if (FormatString[i + 1] == 'd') {
                    if (OutPars[cur_par].Type.find("Ptr") != std::string::npos) {
                        OutPars[cur_par] = LoadPtr(OutPars[cur_par]);
                    }
                    if (OutPars[cur_par].Type.find("Int") == std::string::npos) {
                        OutPars[cur_par] = Zext_Register(OutPars[cur_par]);
                    }
                    Call_PutInt(OutPars[cur_par].Register);
                    cur_par++;
                    i += 2;
                } else if (FormatString[i + 1] == '%') {
                    tmp = '%';
                    Call_PutChar(std::to_string(tmp));
                    i += 2;
                } else {
                    tmp = FormatString[i];
                    Call_PutChar(std::to_string(tmp));
                    i++;
                }
            } else {
                tmp = FormatString[i];
                Call_PutChar(std::to_string(tmp));
                i++;
            }
        } else {
            tmp = FormatString[i];
            Call_PutChar(std::to_string(tmp));
            i++;
        }
    }
}

void Call_PutChar(const std::string &CharName) {
    llvm_print("call void @putch ( i32 " + CharName + " )\n");
}

void Call_PutInt(const std::string &IntName) {
    llvm_print("call void @putint ( i32 " + IntName + " )\n");
}

Register_Pair Judge_Zero(Register_Pair registerPair) {
    if (registerPair.Type.find("Ptr") != std::string::npos) {
        registerPair = LoadPtr(registerPair);
    }
    Register_Pair return_register("%" + std::to_string(register_cnt), "Bool");
    std::string tmpx;
    if (registerPair.Type == "Bool") {
        tmpx = "i1";
    } else if (registerPair.Type == "Int" || registerPair.Type == "ConstInt") {
        tmpx = "i32";
    } else {
        tmpx = "i8";
    }
    Add_String(return_register.Register + " = icmp eq " + tmpx + " " + registerPair.Register + " , 0\n", true);
    register_cnt++;
    return return_register;
}

void Load_1_Reg() {
    Add_String("store i1 1 , i1* @judge_var\n");
}

void Load_0_Reg() {
    Add_String("store i1 0 , i1* @judge_var\n");
}

Register_Pair Judge_Not_Zero(Register_Pair registerPair) {
    if (registerPair.Type.find("Ptr") != std::string::npos) {
        registerPair = LoadPtr(registerPair);
    }
    Register_Pair return_register("%" + std::to_string(register_cnt), "Bool");
    std::string tmpx;
    if (registerPair.Type == "Bool") {
        tmpx = "i1";
    } else if (registerPair.Type == "Int" || registerPair.Type == "ConstInt") {
        tmpx = "i32";
    } else {
        tmpx = "i8";
    }
    Add_String(return_register.Register + " = icmp ne " + tmpx + " " + registerPair.Register + " , 0\n", true);
    register_cnt++;
    return return_register;
}