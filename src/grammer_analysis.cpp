#include"grammer_analysis.h"
#include"function.h"
#include"lexical_analysis.h"
#include"getfirst.h"

//存储函数的参数
std::vector<std::vector<std::string>> strs;

//存储函数名栈，以便在函数嵌套调用的时候保留，第一维为当前所在函数层级的起始位置
std::vector<std::pair<int, std::string>> funcnames;

//当前对应下面字典的深度列表
int CurrentIndex = 0;

//测试用的
std::unordered_map<int, std::vector<Register_Pair>> funcname_pars_match;

std::string cur_def_func_name;

std::vector<std::string> exp_params;

std::vector<int> values;

//当前的变量名称（在lval中使用，暂存当前变量）
std::string var_name;

//在声明函数的时候需要给函数的形参开辟空间，这里先暂存一下形参的名称
std::vector<std::string> func_f_params;


//存储And和Or标签序号
int And_Or_Cnt = 0;

//Or和And的深度
int Or_Depth;
int And_Depth;

std::vector<int> or_depth_vector;
std::vector<int> and_depth_vector;

//当前记录的if的层级
int if_depth = 0;
//当前记录的for的层级
int for_depth = 0;

//当前存储的层级的数组
std::vector<int> if_depth_vector;
//当前存储的for层级的数组
std::vector<int> for_depth_vector;

void CompUnit() {
    Add_String("declare i32 @getint()\n", true);
    Add_String("declare i32 @getchar()\n", true);
    Add_String("declare void @putint(i32)\n", true);
    Add_String("declare void @putch(i32)\n", true);
    Add_String("declare void @putstr(i8*)\n", true);

    word_match fir = getsym();
    word_match_list ans = gettmpsym();
    word_match sec = ans.fir;
    word_match third = ans.sec;
    std::vector<std::string> tmps;

    //这里直接定义一个全局变量，用来存储需要判断0还是1的情况
    Add_String("@judge_var = dso_local global i1 0\n", true);

    while (true) {
        if (fir.word == "int") {
            if (sec.word == "main") {
                MainFuncDef();
                break;
            }
        }
        if (sec.category == "IDENFR" && third.word == "(") {
            FuncDef();
        } else {
            Decl();

        }
        fir = inputword;
        ans = gettmpsym();
        sec = ans.fir;
        third = ans.sec;
    }
    traverse_tree();
    grammer_print("CompUnit");
}

void Decl() {
    if (inputword.word == "const") {
        ConstDecl();
    } else {
        VarDecl();
    }
}

void ConstDecl() {
    if (inputword.word == "const") {
        isConst = true;
        getsym();
        if (isBType(inputword)) {
            BType();
        }
        ConstDef();
        while (inputword.word == ",") {
            getsym();
            curtype = remove_end_array(curtype);
            if (isConstDef(inputword)) {
                ConstDef();
            }
        }
        if (inputword.word == ";") {
            getsym();
        } else {
            error_print(lastline, 'i');
        }
    }
    grammer_print("ConstDecl");
}

void BType() {
    if (isBType(inputword)) {
        if (isConst) {
            curtype = "const" + inputword.word;
        } else {
            curtype = inputword.word;
        }
        getsym();
    }
}

void ConstDef() {
    if (inputword.category == "IDENFR") {
        curname = inputword.word;
        arraylength = 0;
        getsym();
        if (inputword.word != "=") {
            if (inputword.word == "[") {
                getsym();
                if (isConstExp(inputword)) {
                    exp_params.clear();
                    ConstExp();
                    arraylength = cal_exp(exp_params);
                    curtype += "array";
                }
                if (inputword.word == "]") {
                    getsym();
                } else {
                    error_print(lastline, 'k');
                }
            }
        }
        if (inputword.word == "=") {
            getsym();
            ConstInitVal();
        } else {
            std::vector<std::string> tmps;
            AddSymble(curtype, curname, arraylength, tmps, values);
            Define_Varible(curtype, curscope->get_symbol(curname)->get_var_reg(), arraylength);
        }
    }
    grammer_print("ConstDef");
}

void ConstInitVal() {
    values.clear();
    if (inputword.word == "{") {
        getsym();
        if (inputword.word == "}") {
            getsym();
        } else {
            exp_params.clear();
            ConstExp();
            values.push_back(cal_exp(exp_params));
            while (inputword.word == ",") {
                getsym();
                exp_params.clear();
                ConstExp();
                values.push_back(cal_exp(exp_params));
            }
            if (inputword.word == "}") {
                getsym();
            }
        }
        std::vector<std::string> tmps;
        AddSymble(curtype, curname, arraylength, tmps, values);
        Define_Varible(curtype, curscope->get_symbol(curname)->get_var_reg(), arraylength);
        if (curscope->get_symbol(curname)->get_type_name().find("Char") != std::string::npos) {
            std::vector<int> char_values;
            char_values.reserve(values.size());
            for (auto it: values) {
                char_values.push_back(static_cast<char>(it));
            }
            Array_Init_Const(curscope->get_symbol(curname)->get_var_reg(),
                             curscope->get_symbol(curname)->get_type_name(), arraylength,
                             curscope->get_symbol(curname)->get_global(), char_values);
        } else {
            Array_Init_Const(curscope->get_symbol(curname)->get_var_reg(),
                             curscope->get_symbol(curname)->get_type_name(), arraylength,
                             curscope->get_symbol(curname)->get_global(), values);
        }
    } else if (inputword.category == "STRCON") {
        for (int i = 1; i < inputword.word.size() - 1; ++i) {
            if (inputword.word[i] == '\\') {
                if (inputword.word[i] == 'n') {
                    values.push_back(static_cast<int>('\n'));
                } else if (inputword.word[i] == '\\') {
                    values.push_back(static_cast<int>('\\'));
                } else if (inputword.word[i] == 'v') {
                    values.push_back(static_cast<int>('\v'));
                } else if (inputword.word[i] == 't') {
                    values.push_back(static_cast<int>('\t'));
                } else if (inputword.word[i] == 'a') {
                    values.push_back(static_cast<int>('\a'));
                } else if (inputword.word[i] == 'b') {
                    values.push_back(static_cast<int>('\b'));
                } else if (inputword.word[i] == '\'') {
                    values.push_back(static_cast<int>('\''));
                } else if (inputword.word[i] == '\"') {
                    values.push_back(static_cast<int>('\"'));
                } else if (inputword.word[i] == '0') {
                    values.push_back(static_cast<int>('\0'));
                } else if (inputword.word[i] == 'f') {
                    values.push_back(static_cast<int>('\\'));
                }
                i++;
            } else {
                values.push_back(static_cast<int>(inputword.word[i]));
            }
        }
        values.push_back(0);
        getsym();
        std::vector<std::string> tmps;
        AddSymble(curtype, curname, arraylength, tmps, values);
        Define_Varible(curtype, curscope->get_symbol(curname)->get_var_reg(), arraylength);
        Array_Init_Const(curscope->get_symbol(curname)->get_var_reg(), curscope->get_symbol(curname)->get_type_name(),
                         arraylength,
                         curscope->get_symbol(curname)->get_global(), values);
    } else {
        exp_params.clear();
        ConstExp();
        cal_exp(exp_params);
        values.push_back(cal_exp(exp_params));
        std::vector<std::string> tmps;

        AddSymble(curtype, curname, arraylength, tmps, values);
        Define_Varible(curtype, curscope->get_symbol(curname)->get_var_reg(), arraylength);
        if (curscope->get_symbol(curname)->get_type_name().find("Char") != std::string::npos) {
            char tmp_value = static_cast<char>(cal_exp(exp_params));
            Assign_Varible_Const_NotArray(curscope->get_symbol(curname)->get_var_reg(),
                                          curscope->get_symbol(curname)->get_type_name(),
                                          curscope->get_symbol(curname)->get_global(), tmp_value);
        } else {
            Assign_Varible_Const_NotArray(curscope->get_symbol(curname)->get_var_reg(),
                                          curscope->get_symbol(curname)->get_type_name(),
                                          curscope->get_symbol(curname)->get_global(), cal_exp(exp_params));
        }

    }
    grammer_print("ConstInitVal");
}

void VarDecl() {
    isConst = false;
    BType();
    VarDef();
    while (inputword.word == ",") {
        getsym();
        curtype = remove_end_array(curtype);
        if (isVarDef(inputword)) {
            VarDef();
        }
    }
    if (inputword.word == ";") {
        getsym();
    } else {
        error_print(lastline, 'i');
    }
    grammer_print("VarDecl");
}

void VarDef() {
    if (inputword.category == "IDENFR") {
        curname = inputword.word;
        arraylength = 0;
        getsym();
        if (inputword.word == "[") {
            getsym();
            if (isConstExp(inputword)) {
                exp_params.clear();
                ConstExp();
                arraylength = cal_exp(exp_params);
                curtype += "array";
            }
            if (inputword.word == "]") {
                getsym();
            } else {
                error_print(lastline, 'k');
            }
        }

        std::vector<std::string> tmps;
        AddSymble(curtype, curname, arraylength, tmps, values);
        Define_Varible(curtype, curscope->get_symbol(curname)->get_var_reg(), arraylength);
        if (inputword.word == "=") {
            getsym();
            InitVal();
        }
        grammer_print("VarDef");
    }
}

void InitVal() {
    if (inputword.word == "{") {
        getsym();
        if (inputword.word == "}") {
            getsym();
        } else {
            std::vector<Register_Pair> reg_values;
            reg_values.push_back(Exp());
            while (inputword.word == ",") {
                getsym();
                reg_values.push_back(Exp());
            }
            if (inputword.word == "}") {
                getsym();
            }
            Array_Init_Var(curscope->get_symbol(curname)->get_var_reg(), curscope->get_symbol(curname)->get_type_name(),
                           arraylength,
                           curscope->get_symbol(curname)->get_global(), reg_values);
        }
    } else if (inputword.category == "STRCON") {
        std::vector<Register_Pair> reg_values;
        for (int i = 1; i < inputword.word.size() - 1; ++i) {
            int tmpnum;
            if (inputword.word[i] == '\\') {
                if (inputword.word[i + 1] == 'a') {
                    tmpnum = 7;
                }
                if (inputword.word[i + 1] == 'b') {
                    tmpnum = 8;
                }
                if (inputword.word[i + 1] == 't') {
                    tmpnum = 9;
                }
                if (inputword.word[i + 1] == 'n') {
                    tmpnum = 10;
                }
                if (inputword.word[i + 1] == 'v') {
                    tmpnum = 11;
                }
                if (inputword.word[i + 1] == 'f') {
                    tmpnum = 12;
                }
                if (inputword.word[i + 1] == '\"') {
                    tmpnum = 34;
                }
                if (inputword.word[i + 1] == '\'') {
                    tmpnum = 39;
                }
                if (inputword.word[i + 1] == '\\') {
                    tmpnum = 92;
                }
                if (inputword.word[i + 1] == '0') {
                    tmpnum = 0;
                }
                i++;
            } else {
                tmpnum = inputword.word[i];
            }
            reg_values.emplace_back(std::to_string(tmpnum), "Char");
        }
        reg_values.emplace_back(std::to_string(0), "Char");
        Array_Init_Var(curscope->get_symbol(curname)->get_var_reg(), curscope->get_symbol(curname)->get_type_name(),
                       arraylength,
                       curscope->get_symbol(curname)->get_global(), reg_values);
        getsym();
    } else {
        std::string tmpx = curscope->get_symbol(curname)->get_global() ? "@" : "%";
        Register_Pair leftregister(tmpx + curscope->get_symbol(curname)->get_var_reg(),
                                   curscope->get_symbol(curname)->get_type_name());
        Register_Pair registerPair = Exp();
        Assign_Varible_Not_Array(leftregister, registerPair);
    }
    grammer_print("InitVal");
}

void FuncDef() {
    is_global = false;
    blockdepth = 0;
    //暂存当前函数的类型，以便在后面判断是否需要添加一个ret void
    std::string tmp_func_type;
    //生成中间代码的函数类型
    std::string func_type = inputword.word;
    tmp_func_type = inputword.word;
    FuncType();
    if (inputword.category == "IDENFR") {
        //生成中间代码的函数名称
        std::string func_name = inputword.word;

        curname = inputword.word;
        curscope_funcname = inputword.word;
        Define_Function(func_type, func_name);

        getsym();
        std::vector<std::string> params;
        if (inputword.word == "(") {
            int startpointer = pointer;
            paramword = try_read();
            params = preread_params();
            pointer = startpointer;
        }
        if (curscope != nullptr && curscope->get_symbol(curname) != nullptr) {
            AddSymble(curtype, curname, 0, params, values);
            curname += "already_defined";
            curscope_funcname = curname;
            AddSymble(curtype, curname, 0, params, values);
        } else {
            AddSymble(curtype, curname, 0, params, values);
        }
        cur_def_func_name = curname;
    }
    if (curscope == nullptr) {
        curscope = std::make_shared<SymbolTable>();
    }
    curscope = curscope->push_scope();
    Add_String(" ( ");
    if (inputword.word == "(") {
        getsym();
        if (inputword.word == ")") {
            getsym();
        } else {
            if (isFuncFParams(inputword)) {
                func_f_params.clear();
                FuncFParams();
            }
            if (inputword.word == ")") {
                getsym();
            } else {
                error_print(lastline, 'j');
            }
        }
        Add_String(" ) {\n");
        register_cnt++;
        for (const auto &it: func_f_params) {
            std::string FormRegister = curscope->get_symbol(it)->get_var_reg();
            curscope->get_symbol(it)->Refresh_Register();
            Move_FParams(FormRegister, curscope->get_symbol(it)->get_var_reg(),
                         curscope->get_symbol(it)->get_type_name());
        }
        func_f_params.clear();
        Block();
        if (tmp_func_type == "void") {
            Add_String("ret void\n", true);
        } else if (tmp_func_type == "int") {
            Add_String("ret i32 0\n", true);
        } else if (tmp_func_type == "char") {
            Add_String("ret i8 0\n", true);
        }
        Add_String("}\n\n");
        is_global = true;


        if (hasreturned_lastline) {
            judge_return(hasreturned, returnsomething, curscope_funcname);
        } else {
            hasreturned = false;
            judge_return(hasreturned, returnsomething, curscope_funcname);
        }
    }
    grammer_print("FuncDef");
}

void MainFuncDef() {
    is_global = false;
    blockdepth = 0;
    if (inputword.word == "int") {
        getsym();
        curscope_funcname = inputword.word;
        std::vector<std::string> params;
        AddSymble("intfunc", "main", 0, params, values);
        cur_def_func_name = "main";
        if (inputword.word == "main") {
            getsym();
        }
        if (inputword.word == "(") {
            getsym();
        }
        if (inputword.word == ")") {
            getsym();
        } else {
            error_print(lastline, 'j');
        }
        if (curscope == nullptr) {
            curscope = curscope = std::make_shared<SymbolTable>();
        }
        if (curscope == nullptr) {
            curscope = std::make_shared<SymbolTable>();
        }
        curscope = curscope->push_scope();

        Define_Function("int", "main");
        Add_String(" (");
        Add_String(" ) {\n");
        Print_Global();
        register_cnt++;
        Block();
        Add_String("ret i32 0\n", true);
        Add_String("}\n\n");
        is_global = true;
        if (hasreturned_lastline) {
            judge_return(hasreturned, returnsomething, curscope_funcname);
        } else {
            hasreturned = false;
            judge_return(hasreturned, returnsomething, curscope_funcname);
        }
    }
    grammer_print("MainFuncDef");
}

void FuncType() {
    if (inputword.word == "int" || inputword.word == "void" || inputword.word == "char") {
        curtype = inputword.word + "func";
        getsym();
    }
    grammer_print("FuncType");
}

void FuncFParams() {
    FuncFParam();

    while (inputword.word == ",") {
        llvm_print(" , ");
        getsym();
        curtype = remove_end_array(curtype);
        FuncFParam();
    }
    grammer_print("FuncFParams");
}

void FuncFParam() {
    isConst = false;
    BType();
    if (inputword.category == "IDENFR") {
        curname = inputword.word;
        func_f_params.push_back(inputword.word);
        arraylength = 0;
        getsym();
        if (inputword.word == "[") {
            getsym();
            if (inputword.word == "]") {
                getsym();
            } else {
                error_print(lastline, 'k');
            }
            curtype += "array";
        }
    }
    std::vector<std::string> tmps;
    AddSymble(curtype, curname, arraylength, tmps, values);
    Define_Function_Params(curtype, curscope->get_symbol(curname)->get_var_reg());
    grammer_print("FuncFParam");
}

void Block() {
    if (inputword.word == "{") {
        getsym();
        while (inputword.word != "}") {
            BlockItem();
        }
        if (inputword.word == "}") {
            getsym();
        }
    }
    if (curscope != nullptr) {
        curscope = curscope->pop_scope();
    }
    grammer_print("Block");
}

void BlockItem() {
    hasreturned_lastline = false;
    if (inputword.word == "const" || inputword.word == "int" || inputword.word == "char") {
        Decl();
    } else {
        Stmt();
    }
}

void Stmt() {
    hasreturned_lastline = false;
    //左侧等待赋值的寄存器
    Register_Pair tmp_register;

    if (inputword.word == "{") {
        if (curscope == nullptr) {
            curscope = std::make_shared<SymbolTable>();
        }
        curscope = curscope->push_scope();

        Block();
    } else if (inputword.word == "if") {
        if_depth++;
        if_depth_vector.push_back(if_depth);
        getsym();
        if (inputword.word == "(") {
            getsym();
            Register_Pair registerPair;
            if (isCond(inputword)) {
                Cond();
                registerPair = Load_Varible("judge_var", "Bool", true);
            }

            If_Jump(registerPair, if_depth_vector[if_depth_vector.size() - 1],
                    "%then" + std::to_string(if_depth_vector[if_depth_vector.size() - 1]),
                    "%else" + std::to_string(if_depth_vector[if_depth_vector.size() - 1]));

            Add_String("then" + std::to_string(if_depth_vector[if_depth_vector.size() - 1]) + ":\n");
            if (inputword.word == ")") {
                getsym();
                blockdepth++;
                Stmt();
                blockdepth--;
                Add_String("br label %end" + std::to_string(if_depth_vector[if_depth_vector.size() - 1]) + "\n");
                Add_String("else" + std::to_string(if_depth_vector[if_depth_vector.size() - 1]) + ":\n");
                if (inputword.word == "else") {
                    getsym();
                    blockdepth++;
                    Stmt();
                    blockdepth--;
                }
            } else {
                error_print(lastline, 'j');
                blockdepth++;
                Stmt();
                blockdepth--;
                if (inputword.word == "else") {
                    getsym();
                    blockdepth++;
                    Stmt();
                    blockdepth--;
                }
            }
            Add_String("br label %end" + std::to_string(if_depth_vector[if_depth_vector.size() - 1]) +
                       "\n");
            Add_String("end" + std::to_string(if_depth_vector[if_depth_vector.size() - 1]) + ":\n");
            if_depth_vector.pop_back();
        }
    } else if (inputword.word == "for") {
        for_depth++;
        for_depth_vector.push_back(for_depth);
        is_in_for = true;
        getsym();
        if (inputword.word == "(") {
            getsym();
            if (inputword.word != ";") {
                if (isForStmt(inputword)) {
                    ForStmt();
                }
            }
            Add_String("br label %for_judge" +
                       std::to_string(for_depth_vector[for_depth_vector.size() - 1]) + "\n");
            if (inputword.word == ";") {
                getsym();
            } else {
                error_print(lastline, 'i');
            }
            Add_String("for_judge" + std::to_string(for_depth_vector[for_depth_vector.size() - 1]) + ":\n");
            if (inputword.word != ";") {
                if (isCond(inputword)) {
                    Cond();
                    Register_Pair registerPair = Load_Varible("judge_var", "Bool", true);
                    Add_String("br i1 " + registerPair.Register + " , label %for_body" +
                               std::to_string(for_depth_vector[for_depth_vector.size() - 1]) + " , label %for_end" +
                               std::to_string(for_depth_vector[for_depth_vector.size() - 1]) + "\n");
                }
            } else {
                Add_String("br label %for_body" +
                           std::to_string(for_depth_vector[for_depth_vector.size() - 1]) + "\n");
            }
            if (inputword.word == ";") {
                getsym();
            } else {
                error_print(lastline, 'i');
            }
            Add_String("for_update" + std::to_string(for_depth_vector[for_depth_vector.size() - 1]) + ":\n");
            if (inputword.word != ")") {
                if (isForStmt(inputword)) {
                    ForStmt();
                }
            }
            Add_String("br label %for_judge" + std::to_string(for_depth_vector[for_depth_vector.size() - 1]) + "\n");
            if (inputword.word == ")") {
                getsym();
            } else {
                error_print(lastline, 'j');
            }
            blockdepth++;
            Add_String("for_body" + std::to_string(for_depth_vector[for_depth_vector.size() - 1]) + ":\n");
            Stmt();
            blockdepth--;
            Add_String("br label %for_update" + std::to_string(for_depth_vector[for_depth_vector.size() - 1]) + "\n");
            Add_String("for_end" + std::to_string(for_depth_vector[for_depth_vector.size() - 1]) + ":\n");
            for_depth_vector.pop_back();
            is_in_for = false;
        }
    } else if (inputword.word == "break") {
        if (!is_in_for) {
            error_print(line, 'm');
        }
        getsym();
        if (inputword.word == ";") {
            getsym();
        } else {
            error_print(lastline, 'i');
        }
        Add_String("br label %for_end" + std::to_string(for_depth_vector[for_depth_vector.size() - 1]) + "\n");
        register_cnt++;
    } else if (inputword.word == "continue") {
        if (!is_in_for) {
            error_print(line, 'm');
        }
        getsym();
        if (inputword.word == ";") {
            getsym();
        } else {
            error_print(lastline, 'i');
        }
        Add_String("br label %for_update" + std::to_string(for_depth_vector[for_depth_vector.size() - 1]) + "\n");
        register_cnt++;
    } else if (inputword.word == "return") {
        getsym();
        if (inputword.word != ";") {
            hasreturned = true;
            returnsomething = true;
            if (blockdepth == 0) {
                hasreturned_lastline = true;
            }
            judge_return(hasreturned, returnsomething, curscope_funcname);
            word_match_list ans = gettmpsym();
            if ((isExp(inputword) && ans.fir.word != "=") ||
                (!isUnaryOp(inputword) && ans.fir.category == "IDENFR" && ans.sec.word != "=")) {
                Register_Pair registerPair = Exp();
                Func_Return(curscope->get_symbol(cur_def_func_name)->get_type_name(), registerPair);
            }
        }
        if (inputword.word == ";") {
            if (blockdepth == 0) {
                hasreturned = true;
                returnsomething = false;
                hasreturned_lastline = true;
            }
            getsym();
            if (curscope->get_symbol(cur_def_func_name)->get_type_name() == "VoidFunc") {
                Add_String("ret void\n");
            }
        } else {
            error_print(lastline, 'i');
        }
        //这里不知道为什么ret需要加一个才能可以
        register_cnt++;
    } else if (inputword.word == "printf") {
        getsym();
        if (inputword.word == "(") {
            getsym();
        }
        int num = count_print(inputword.word);
        if (inputword.category == "STRCON") {
            std::string FormatString = inputword.word;
            std::vector<Register_Pair> OutputPars;
            getsym();
            while (inputword.word == ",") {
                getsym();
                OutputPars.push_back(Exp());
                num--;
            }
            if (num) {
                error_print(line, 'l');
            }
            if (inputword.word == ")") {
                getsym();
                if (inputword.word == ";") {
                    getsym();
                } else {
                    error_print(lastline, 'i');
                }
            } else {
                error_print(lastline, 'j');
            }
            Print(FormatString, OutputPars);
        }
    } else {
        word_match_list ans = gettmpsym();
        int startpointer = pointer;
        bool flag = false;
        while (input[startpointer] != '\n') {
            if (input[startpointer] == '=') {
                flag = true;
                break;
            }
            startpointer++;
        }
        if (flag) {
            bool tmpflag = false;
            if (curscope->get_symbol(inputword.word) != nullptr) {
                tmpflag = true;
            }
            std::string classname;
            if (tmpflag) {
                classname = curscope->get_symbol(inputword.word)->get_type_name();
            }
            tmp_register = LVal();
            if (inputword.word == "=") {
                if (start_with(classname, "Const")) {
                    error_print(line, 'h');
                }
                getsym();
                if (inputword.word == "getint") {
                    getsym();
                    if (inputword.word == "(") {
                        getsym();
                        if (inputword.word == ")") {
                            getsym();
                            if (inputword.word == ";") {
                                getsym();
                            } else {
                                error_print(lastline, 'i');
                            }
                        } else {
                            error_print(lastline, 'j');
                        }
                    }
                    std::vector<std::string> tmp_types;
                    std::vector<Register_Pair> tmp_pars;
                    Register_Pair registerPair = Call_Func("getint", "IntFunc", tmp_types, tmp_pars);

                    Assign_Varible_Not_Array(tmp_register, registerPair);
                } else if (inputword.word == "getchar") {
                    getsym();
                    if (inputword.word == "(") {
                        getsym();
                        if (inputword.word == ")") {
                            getsym();
                            if (inputword.word == ";") {
                                getsym();
                            } else {
                                error_print(lastline, 'i');
                            }
                        } else {
                            error_print(lastline, 'j');
                        }
                    }
                    std::vector<std::string> tmp_types;
                    std::vector<Register_Pair> tmp_pars;
                    Register_Pair registerPair = Call_Func("getchar", "IntFunc", tmp_types, tmp_pars);
                    Assign_Varible_Not_Array(tmp_register, registerPair);
                } else {
                    Register_Pair registerPair = Exp();
                    Assign_Varible_Not_Array(tmp_register, registerPair);
                    if (inputword.word == ";") {
                        getsym();
                    } else {
                        error_print(lastline, 'i');
                    }
                }
            }
        } else {
            if (inputword.word != ";") {
                if (isExp(inputword)) {
                    Exp();
                }
            }
            if (inputword.word == ";") {
                getsym();
            } else {
                error_print(lastline, 'i');
            }
        }
    }
    grammer_print("Stmt");
}

void ForStmt() {
    bool tmpflag = false;
    if (curscope->get_symbol(inputword.word) != nullptr) {
        tmpflag = true;
    }
    std::string classname;
    if (tmpflag) {
        classname = curscope->get_symbol(inputword.word)->get_type_name();
    }
    Register_Pair LeftRegister = LVal();
    if (inputword.word == "=") {
        if (start_with(classname, "Const")) {
            error_print(line, 'h');
        }
        getsym();
    }
    Register_Pair registerPair = Exp();
    Assign_Varible_Not_Array(LeftRegister, registerPair);
    grammer_print("ForStmt");
}

Register_Pair Exp() {
    Register_Pair left_register = AddExp();
    grammer_print("Exp");
    return left_register;
}

void Cond() {
    LOrExp();
    grammer_print("Cond");
}

Register_Pair LVal(bool is_in_exp) {
    Register_Pair left_register;

    std::string tmpx = curscope->get_symbol(inputword.word)->get_global() ? "@" : "%";
    left_register.Register = tmpx + curscope->get_symbol(inputword.word)->get_var_reg();
    left_register.Type = curscope->get_symbol(inputword.word)->get_type_name();

    std::string array_name_tmp;
    var_name = inputword.word;
    array_name_tmp = inputword.word;
    Register_Pair element_index;
    if (inputword.category == "IDENFR") {
        exp_params.push_back(inputword.word);
        if (!strs.empty()) {
            strs[strs.size() - 1].push_back(inputword.word);
        }
        if (curscope == nullptr || !curscope->exist_in_scope(inputword.word)) {
            error_print(line, 'c');
        }
        getsym();
    }
    if (inputword.word == "[") {
        int formal_length = (int) exp_params.size();
        auto it = exp_params;
        exp_params.push_back(inputword.word);
        if (!strs.empty()) {
            strs[strs.size() - 1].push_back(inputword.word);
        }
        getsym();
        element_index = Exp();
        if (inputword.word == "]") {
            exp_params.push_back(inputword.word);
            if (!strs.empty()) {
                strs[strs.size() - 1].push_back(inputword.word);
            }
            getsym();
        } else {
            exp_params.push_back("]");
            error_print(lastline, 'k');
        }
        int now_length = (int) exp_params.size();
        std::string last_element = it[formal_length - 1];
        for (int i = formal_length; i < now_length; ++i) {
            last_element += exp_params[i];
        }
        it[formal_length - 1] = last_element;
        exp_params = it;
        int Array_Length;

        if (curscope->get_symbol(array_name_tmp)->get_type_name().find("Ptr") == std::string::npos) {
            Array_Length = (int) (std::dynamic_pointer_cast<ArrayType>(
                    curscope->get_symbol(array_name_tmp)->get_type()))->get_type_size();
        } else {
            Array_Length = 0;
        }

        left_register = Load_Current_Array_Element(curscope->get_symbol(array_name_tmp)->get_var_reg(),
                                                   curscope->get_symbol(array_name_tmp)->get_type_name(),
                                                   curscope->get_symbol(array_name_tmp)->get_global(), Array_Length,
                                                   element_index);
    } else {
        if (is_in_exp) {
            if (curscope->get_symbol(var_name)->get_type_name().find("Array") == std::string::npos) {
                left_register = Load_Varible(curscope->get_symbol(var_name)->get_var_reg(),
                                             curscope->get_symbol(var_name)->get_type_name(),
                                             curscope->get_symbol(var_name)->get_global());
            } else {
                int Array_Length = 0;
                if (curscope->get_symbol(var_name)->get_type_name().find("Ptr") == std::string::npos) {
                    Array_Length = (int) (std::dynamic_pointer_cast<ArrayType>(
                            curscope->get_symbol(var_name)->get_type()))->get_type_size();
                }
                left_register = Load_Varible_Array(curscope->get_symbol(var_name)->get_var_reg(),
                                                   curscope->get_symbol(var_name)->get_type_name(),
                                                   curscope->get_symbol(var_name)->get_global(), Array_Length);
            }
        }
    }
    grammer_print("LVal");
    return left_register;
}

Register_Pair PrimaryExp() {
    Register_Pair left_register;
    if (inputword.word == "(") {
        exp_params.push_back(inputword.word);
        if (!strs.empty()) {
            strs[strs.size() - 1].push_back(inputword.word);
        }
        getsym();
        left_register = Exp();
        if (inputword.word == ")") {
            if (!strs.empty()) {
                strs[strs.size() - 1].push_back(inputword.word);
            }
            exp_params.push_back(inputword.word);
            getsym();
        } else {
            error_print(lastline, 'j');
        }
    } else if (inputword.category == "INTCON") {
        exp_params.push_back(inputword.word);
        left_register = Register_Pair(inputword.word, "Int");
        Number();
    } else if (inputword.category == "CHRCON") {
        exp_params.push_back(inputword.word);
        int ascii = inputword.word[1];
        int tmplength= inputword.word.size();
        if (inputword.word[1] == '\\') {
            if (inputword.word[tmplength-2] == '\\') {
                ascii = 92;
            } else if (inputword.word[tmplength-2] == 't') {
                ascii = 9;
            } else if (inputword.word[tmplength-2] == 'n') {
                ascii = 10;
            } else if (inputword.word[tmplength-2] == '\'') {
                ascii = 39;
            } else if (inputword.word[tmplength-2] == '\"') {
                ascii = 34;
            } else if (inputword.word[tmplength-2] == 'b') {
                ascii = 8;
            } else if (inputword.word[tmplength-2] == 'f') {
                ascii = 12;
            } else if (inputword.word[tmplength-2] == 'v') {
                ascii = 11;
            } else if (inputword.word[tmplength-2] == '0') {
                ascii = 0;
            } else if (inputword.word[tmplength-2] == 'a') {
                ascii = 7;
            } else {
                ascii = inputword.word[2];
            }
        }
        left_register = Register_Pair(std::to_string(ascii), "Int");
        Character();
    } else {
        left_register = LVal(true);
    }
    grammer_print("PrimaryExp");
    return left_register;
}

void Number() {
    if (inputword.category == "INTCON") {
        if (!strs.empty()) {
            strs[strs.size() - 1].push_back(inputword.word);
        }
        getsym();
    }
    grammer_print("Number");
}

void Character() {
    if (inputword.category == "CHRCON") {
        if (!strs.empty()) {
            strs[strs.size() - 1].push_back(inputword.word);
        }
        getsym();
    }
    grammer_print("Character");
}

Register_Pair UnaryExp() {
    word_match_list ans = gettmpsym();
    word_match fir = inputword;
    word_match sec = ans.fir;
    word_match third = ans.sec;

    //保存的临时寄存器
    Register_Pair left_register;

    if (fir.word == "+" || fir.word == "-" || inputword.word == "!") {
        int flag;
        if (inputword.word == "+") {
            flag = 0;
        }
        if (inputword.word == "-") {
            flag = 1;
        }
        if (inputword.word == "!") {
            flag = 2;
        }
        UnaryOp();
        left_register = UnaryExp();
        if (flag == 1) {
            left_register = Handle_Exp(Register_Pair("0", "Int"), left_register, "-");
        } else if (flag == 2) {
            left_register = Handle_Exp(Register_Pair("0", "Int"), left_register, "!");
        }
    } else if (fir.category == "IDENFR" && sec.word == "(") {
        funcnames.emplace_back(strs.size(), fir.word);
        std::vector<Register_Pair> tmp_reg_pars;
        tmp_reg_pars.reserve(30);
        CurrentIndex++;
        funcname_pars_match[CurrentIndex] = tmp_reg_pars;
        if (!strs.empty()) {
            strs[strs.size() - 1].push_back(inputword.word);
        }
        if (curscope == nullptr || !curscope->exist_in_scope(inputword.word)) {
            error_print(line, 'c');
        }
        getsym();
        if (inputword.word == "(") {
            getsym();
            if (inputword.word != ")") {
                FuncRParams();
            }
            if (inputword.word == ")") {
                exp_params.push_back(inputword.word);
                std::vector<std::shared_ptr<Symbol>> pars;
                for (int i = funcnames[funcnames.size() - 1].first; i < strs.size(); ++i) {
                    auto it = judge_class(strs[i]);
                    std::shared_ptr<Symbol> cursymbol;
                    if (it == "Int") {
                        cursymbol = std::make_shared<VariableSymbol>("tmp", BasicType::get_int_type(), values,
                                                                     true);
                    } else if (it == "Char") {
                        cursymbol = std::make_shared<VariableSymbol>("tmp", BasicType::get_char_type(), values,
                                                                     true);
                    } else if (it == "IntArray") {
                        cursymbol = std::make_shared<VariableSymbol>("tmp", std::make_shared<ArrayType>(
                                BasicType::get_int_type(), 0), values, true);

                    } else if (it == "CharArray") {
                        cursymbol = std::make_shared<VariableSymbol>("tmp", std::make_shared<ArrayType>(
                                BasicType::get_char_type(), 0), values, true);
                    }
                    pars.push_back(cursymbol);
                }
                std::vector<Register_Pair> par_regs_tmp;
                par_regs_tmp.clear();
                for (const auto &it: funcname_pars_match[CurrentIndex]) {
                    par_regs_tmp.push_back(it);
                }
                while (strs.size() > funcnames[funcnames.size() - 1].first) {
                    strs.pop_back();
                }
                std::vector<std::string> par_type;
                par_type.clear();
                for (const auto &it: std::dynamic_pointer_cast<FunctionSymbol>(
                        curscope->get_symbol(funcnames[funcnames.size() - 1].second))->get_all_params()) {
                    par_type.push_back(it->get_type_name());
                }

                left_register = Call_Func(funcnames[funcnames.size() - 1].second,
                                          curscope->get_symbol(funcnames[funcnames.size() - 1].second)->get_type_name(),
                                          par_type,
                                          par_regs_tmp);

//                judge_params(pars, funcnames[funcnames.size() - 1].second);
                funcnames.pop_back();
                funcname_pars_match[CurrentIndex].clear();
                CurrentIndex--;
                getsym();

            } else {
                error_print(lastline, 'j');
            }
        }
    } else {
        left_register = PrimaryExp();
    }
    grammer_print("UnaryExp");
    return left_register;
}

void UnaryOp() {
    if (inputword.word == "+" || inputword.word == "-" || inputword.word == "!") {
        exp_params.push_back(inputword.word);
        if (!strs.empty()) {
            strs[strs.size() - 1].push_back(inputword.word);
        }
        getsym();
    }
    grammer_print("UnaryOp");
}

void FuncRParams() {
    std::vector<std::string> nowvec;
    strs.push_back(nowvec);
    funcname_pars_match[CurrentIndex].push_back(Exp());
    while (inputword.word == ",") {
        getsym();
        nowvec.clear();
        strs.push_back(nowvec);
        funcname_pars_match[CurrentIndex].push_back(Exp());
    }
    grammer_print("FuncRParams");
}

Register_Pair MulExp() {
    //保存的临时寄存器
    Register_Pair left_register, right_register;
    left_register = UnaryExp();
    grammer_print("MulExp");
    while (inputword.word == "*" || inputword.word == "/" || inputword.word == "%") {
        int flag;
        if (inputword.word == "*") {
            flag = 0;
        }
        if (inputword.word == "/") {
            flag = 1;
        }
        if (inputword.word == "%") {
            flag = 2;
        }
        if (!strs.empty()) {
            strs[strs.size() - 1].push_back(inputword.word);
        }
        exp_params.push_back(inputword.word);
        getsym();
        right_register = UnaryExp();
        if (flag == 0) {
            left_register = Handle_Exp(left_register, right_register, "*");
        } else if (flag == 1) {
            left_register = Handle_Exp(left_register, right_register, "/");
        } else if (flag == 2) {
            left_register = Handle_Exp(left_register, right_register, "%");
        }
        grammer_print("MulExp");
    }
    return left_register;
}

Register_Pair AddExp() {
    Register_Pair left_register, right_register;
    left_register = MulExp();
    grammer_print("AddExp");
    while (inputword.word == "+" || inputword.word == "-") {
        int flag = inputword.word == "+" ? 0 : 1;
        if (!strs.empty()) {
            strs[strs.size() - 1].push_back(inputword.word);
        }
        exp_params.push_back(inputword.word);
        getsym();
        right_register = MulExp();
        if (flag == 0) {
            left_register = Handle_Exp(left_register, right_register, "+");
        } else {
            left_register = Handle_Exp(left_register, right_register, "-");
        }
        grammer_print("AddExp");
    }
    return left_register;
}

Register_Pair RelExp() {
    Register_Pair left_register = AddExp();
    grammer_print("RelExp");
    while (inputword.word == "<" || inputword.word == ">" || inputword.word == "<=" || inputword.word == ">=") {
        std::string tmpstr = inputword.word;
        getsym();
        Register_Pair right_register = AddExp();
        left_register = Handle_Exp(left_register, right_register, tmpstr);
        grammer_print("RelExp");
    }
    return left_register;
}

Register_Pair EqExp() {
    Register_Pair left_register = RelExp();
    grammer_print("EqExp");
    while (inputword.word == "==" || inputword.word == "!=") {
        std::string tmpstr = inputword.word;
        getsym();
        Register_Pair right_register = RelExp();
        left_register = Handle_Exp(left_register, right_register, tmpstr);
        grammer_print("EqExp");
    }
    return left_register;
}

void LAndExp() {
    and_depth_vector.push_back(And_Depth);
    And_Depth++;
    Register_Pair left_register = EqExp();
    grammer_print("LAndExp");
    if (left_register.Type != "Bool") {
        left_register = Judge_Not_Zero(left_register);
    }
    Add_String("br i1 " + left_register.Register + " , label %Current" + std::to_string(And_Or_Cnt) + " , label %And" +
               std::to_string(and_depth_vector[and_depth_vector.size() - 1]) +
               "\n", true);
    Add_String("Current" + std::to_string(And_Or_Cnt) + ":\n", true);
    And_Or_Cnt++;
    while (inputword.word == "&&") {
        getsym();
        left_register = EqExp();
        if (left_register.Type != "Bool") {
            left_register = Judge_Not_Zero(left_register);
        }
        Add_String(
                "br i1 " + left_register.Register + " , label %Current" + std::to_string(And_Or_Cnt) + " , label %And" +
                std::to_string(and_depth_vector[and_depth_vector.size() - 1]) +
                "\n", true);
        Add_String("Current" + std::to_string(And_Or_Cnt) + ":\n", true);
        And_Or_Cnt++;
        grammer_print("LAndExp");
    }

    Load_1_Reg();
    Add_String("br label %AndEnd" + std::to_string(and_depth_vector[and_depth_vector.size() - 1]) + "\n",
               true);
    Add_String("And" + std::to_string(and_depth_vector[and_depth_vector.size() - 1]) + ":\n", true);
    Load_0_Reg();
    Add_String("br label %AndEnd" + std::to_string(and_depth_vector[and_depth_vector.size() - 1]) + "\n",
               true);
    Add_String("AndEnd" + std::to_string(and_depth_vector[and_depth_vector.size() - 1]) + ":\n", true);
    and_depth_vector.pop_back();
}

void LOrExp() {
    or_depth_vector.push_back(Or_Depth);
    Or_Depth++;
    Register_Pair left_register;
    LAndExp();
    left_register = Load_Varible("judge_var", "Bool", true);
    Add_String("br i1 " + left_register.Register + " , label %Or" +
               std::to_string(or_depth_vector[or_depth_vector.size() - 1])
               + " , label %Current" + std::to_string(And_Or_Cnt) + "\n", true);
    Add_String("Current" + std::to_string(And_Or_Cnt) + ":\n", true);
    And_Or_Cnt++;
    grammer_print("LOrExp");
    while (inputword.word == "||") {
        getsym();
        LAndExp();
        left_register = Load_Varible("judge_var", "Bool", true);
        left_register = Judge_Zero(left_register);
        Add_String("br i1 " + left_register.Register + " , label %Current" +
                   std::to_string(And_Or_Cnt) + " , label %Or" +
                   std::to_string(or_depth_vector[or_depth_vector.size() - 1]) + "\n", true);
        Add_String("Current" + std::to_string(And_Or_Cnt) + ":\n", true);
        And_Or_Cnt++;
        grammer_print("LOrExp");
    }

    Load_0_Reg();
    Add_String("br label %OrEnd" + std::to_string(or_depth_vector[or_depth_vector.size() - 1]) + "\n",
               true);
    Add_String("Or" + std::to_string(or_depth_vector[or_depth_vector.size() - 1]) + ":\n", true);
    Load_1_Reg();
    Add_String("br label %OrEnd" + std::to_string(or_depth_vector[or_depth_vector.size() - 1]) + "\n",
               true);
    Add_String("OrEnd" + std::to_string(or_depth_vector[or_depth_vector.size() - 1]) + ":\n", true);
    or_depth_vector.pop_back();
}

void ConstExp() {
    AddExp();
    grammer_print("ConstExp");
}