//
// Created by xht  on 2024/10/11.
//

#include"symbletable.h"
#include<intermediate_code.h>

std::shared_ptr<SymbolTable> curscope = nullptr;

std::string curtype;

bool isConst = false;

std::string curname;

int arraylength = 0;

std::string stringexp;

std::shared_ptr<SymbolTable> rootscope = nullptr;

bool hasreturned = false;

bool returnsomething = false;
int blockdepth = 0;
bool hasreturned_lastline = false;

std::string funcname;

bool is_in_for = false;

void AddSymble(const std::string &type, const std::string &name, int length, const std::vector<std::string> &params,
               std::vector<int> values) {
    if (curscope == nullptr) {
        curscope = std::make_shared<SymbolTable>();
        rootscope = curscope;
    }
    if (type == "int") {
        std::shared_ptr<Symbol> symbol = std::make_shared<VariableSymbol>(name, BasicType::get_int_type(), values,
                                                                          is_global);
        curscope->add_Symbol(symbol);
    } else if (type == "char") {
        std::shared_ptr<Symbol> symbol = std::make_shared<VariableSymbol>(name, BasicType::get_char_type(), values,
                                                                          is_global);
        curscope->add_Symbol(symbol);
    } else if (type == "constint") {
        std::shared_ptr<Symbol> symbol = std::make_shared<VariableSymbol>(name, BasicType::get_const_int_type(), values,
                                                                          is_global);
        curscope->add_Symbol(symbol);
    } else if (type == "constchar") {
        std::shared_ptr<Symbol> symbol = std::make_shared<VariableSymbol>(name, BasicType::get_const_char_type(),
                                                                          values, is_global);
        curscope->add_Symbol(symbol);
    } else if (type == "intfunc") {
        std::vector<std::shared_ptr<Type>> types;
        for (const std::string &it: params) {
            if (it == "int") {
                types.push_back(BasicType::get_int_type());
            }
            if (it == "char") {
                types.push_back(BasicType::get_char_type());
            }
            if (it == "intarray") {
                types.push_back(std::make_shared<ArrayType>(BasicType::get_int_type(), 0));
            }
            if (it == "chararray") {
                types.push_back(std::make_shared<ArrayType>(BasicType::get_char_type(), 0));
            }
        }
        std::shared_ptr<Symbol> symbol = std::make_shared<FunctionSymbol>(name, FuncType::get_int_func_type(), types,
                                                                          is_global);

        curscope->add_Symbol(symbol);
    } else if (type == "charfunc") {
        std::vector<std::shared_ptr<Type>> types;
        for (const std::string &it: params) {
            if (it == "int") {
                types.push_back(BasicType::get_int_type());
            }
            if (it == "char") {
                types.push_back(BasicType::get_char_type());
            }
            if (it == "intarray") {
                types.push_back(std::make_shared<ArrayType>(BasicType::get_int_type(), 0));
            }
            if (it == "chararray") {
                types.push_back(std::make_shared<ArrayType>(BasicType::get_char_type(), 0));
            }
        }
        std::shared_ptr<Symbol> symbol = std::make_shared<FunctionSymbol>(name, FuncType::get_char_func_type(), types,
                                                                          is_global);
        curscope->add_Symbol(symbol);
    } else if (type == "voidfunc") {

        std::vector<std::shared_ptr<Type>> types;
        for (const std::string &it: params) {
            if (it == "int") {
                types.push_back(BasicType::get_int_type());
            }
            if (it == "char") {
                types.push_back(BasicType::get_char_type());
            }
            if (it == "intarray") {
                types.push_back(std::make_shared<ArrayType>(BasicType::get_int_type(), 0));
            }
            if (it == "chararray") {
                types.push_back(std::make_shared<ArrayType>(BasicType::get_char_type(), 0));
            }
        }
        std::shared_ptr<Symbol> symbol = std::make_shared<FunctionSymbol>(name, FuncType::get_void_func_type(), types,
                                                                          is_global);
        curscope->add_Symbol(symbol);
    } else if (type == "intarray") {
        std::shared_ptr<Type> int_type = BasicType::get_int_type();
        std::shared_ptr<Type> array_type = std::make_shared<ArrayType>(int_type, length);
        std::shared_ptr<Symbol> symbol = std::make_shared<VariableSymbol>(name, array_type, values, is_global);
        curscope->add_Symbol(symbol);
    } else if (type == "chararray") {
        std::shared_ptr<Type> char_type = BasicType::get_char_type();
        std::shared_ptr<Type> array_type = std::make_shared<ArrayType>(char_type, length);
        std::shared_ptr<Symbol> symbol = std::make_shared<VariableSymbol>(name, array_type, values, is_global);
        curscope->add_Symbol(symbol);
    } else if (type == "constintarray") {
        std::shared_ptr<Type> const_int_type = BasicType::get_const_int_type();
        std::shared_ptr<Type> array_type = std::make_shared<ArrayType>(const_int_type, length);
        std::shared_ptr<Symbol> symbol = std::make_shared<VariableSymbol>(name, array_type, values, is_global);
        curscope->add_Symbol(symbol);
    } else if (type == "constchararray") {
        std::shared_ptr<Type> const_char_type = BasicType::get_const_char_type();
        std::shared_ptr<Type> array_type = std::make_shared<ArrayType>(const_char_type, length);
        std::shared_ptr<Symbol> symbol = std::make_shared<VariableSymbol>(name, array_type, values, is_global);
        curscope->add_Symbol(symbol);
    }
}

int depth = 1;

void depth_search(const std::shared_ptr<SymbolTable> &now) {
    if (now == nullptr) {
        return;
    }
    depth++;
    for (const std::shared_ptr<Symbol> &item: now->get_all_symbols()) {
        semantics_print(depth, item->get_name(), item->get_type_name());
    }
    for (const std::shared_ptr<SymbolTable> &item: now->get_history()) {
        depth_search(item);
    }
}

void traverse_tree() {
    std::queue<std::shared_ptr<SymbolTable>> _queue;
    if (curscope == nullptr) {
        return;
    }
    for (const std::shared_ptr<Symbol> &item: curscope->get_all_symbols()) {
        if (item->get_name() == "main") {
            continue;
        }
        semantics_print(depth, item->get_name(), item->get_type_name());
    }
    for (const std::shared_ptr<SymbolTable> &item: curscope->get_history()) {
        depth_search(item);
    }
}

void judge_params(const std::vector<std::shared_ptr<Symbol>> &pars, const std::string &funcname) {
    if (rootscope == nullptr) {
        return;
    }
    for (const std::shared_ptr<Symbol> &item: rootscope->get_all_symbols()) {
        if (item->get_name() == funcname) {
            if (!end_with(item->get_type_name(), "Func")) {
                error_print(line, 'c');
            }
            std::shared_ptr<FunctionSymbol> funcitem = std::dynamic_pointer_cast<FunctionSymbol>(item);
            if (pars.size() != funcitem->get_all_params().size()) {
                error_print(line, 'd');
                return;
            }
            for (int i = 0; i < pars.size(); ++i) {
                if (pars[i]->get_type_name() != (funcitem->get_all_params()[i])->get_type_name()) {
                    if ((pars[i]->get_type_name() == "Int" &&
                         (funcitem->get_all_params()[i])->get_type_name() == "Char") ||
                        (pars[i]->get_type_name() == "Char" &&
                         (funcitem->get_all_params()[i])->get_type_name() == "Int")) {
                        continue;
                    }
                    error_print(line, 'e');
                    return;
                }
            }
            return;
        }
    }
}


void judge_return(bool hasreturn, bool returnsomething, const std::string &cur_funcname) {
    if (rootscope == nullptr) {
        return;
    }
    std::string tmps;
    for (const std::shared_ptr<Symbol> &item: rootscope->get_all_symbols()) {
        if (item->get_name() == cur_funcname) {
            if (hasreturn && returnsomething && item->get_type_name() == "VoidFunc") {
                error_print(line, 'f');
            } else if (!hasreturn && item->get_type_name() != "VoidFunc") {
                error_print(lastline, 'g');
            }
        }
    }
}

std::string judge_class(const std::vector<std::string> &strs) {
    // 1:int 2:char 3:intarray 4:chararray
    int type = 5;
    for (int i = 0; i < strs.size(); ++i) {
        auto it = strs[i];
        if (it == "+" || it == "-" || it == "*" || it == "/" || it == "%") {
            continue;
        }
        auto symbol = curscope->get_symbol(it);
        if (symbol != nullptr) {
            if (end_with(symbol->get_type_name(), "Array")) {
                if (i + 1 < strs.size() && strs[i + 1] == "[") {
                    int tmp = 1;
                    i++;
                    if (symbol->get_type_name() == "ConstCharArray" || symbol->get_type_name() == "CharArray") {
                        type = type < 2 ? type : 2;
                    } else {
                        type = 1;
                    }
                    while (tmp) {
                        if (i >= strs.size()) {
                            break;
                        }
                        if (strs[i] == "[") {
                            tmp++;
                        }
                        if (strs[i] == "]") {
                            tmp--;
                        }
                        i++;
                    }
                } else {
                    if (symbol->get_type_name() == "ConstCharArray" || symbol->get_type_name() == "CharArray") {
                        type = 4;
                    } else {
                        type = 3;
                    }
                    break;
                }
            } else if (symbol->get_type_name() == "ConstInt" || symbol->get_type_name() == "Int") {
                type = 1;
            } else if (symbol->get_type_name() == "ConstChar" || symbol->get_type_name() == "Char") {
                type = type < 2 ? type : 2;
            } else if (symbol->get_type_name() == "IntFunc") {
                type = 1;
            } else if (symbol->get_type_name() == "CharFunc") {
                type = type < 2 ? type : 2;
            }
        } else {
            if (is_string_number(it)) {
                type = 1;
            }
            if (is_string_char(it)) {
                type = type < 2 ? type : 2;
            }
        }
    }
    if (type == 1) {
        return "Int";
    } else if (type == 2) {
        return "Char";
    } else if (type == 3) {
        return "IntArray";
    } else if (type == 4) {
        return "CharArray";
    }
    return "None";
}

int cal_exp(std::vector<std::string> values) {
    std::string answer;
    std::vector<std::string> exps;
    for (auto it: values) {
        if (is_string_number(it)) {
            answer += it;
            exps.push_back(it);
        } else if (it.find('[') != std::string::npos) {
            size_t left_pos = it.find('[');
            size_t right_pos = it.rfind(']');
            //取到内层的东西
            std::string tmp_strs;
            if (left_pos != std::string::npos && right_pos != std::string::npos) {
                for (size_t i = left_pos + 1; i < right_pos; ++i) {
                    tmp_strs += it[i];
                }
                std::vector<std::string> tmp_values;

                tmp_values.push_back(tmp_strs);
                int num = cal_exp(tmp_values);
                //现在tmp_strs存的是最外层的数组名
                tmp_strs.clear();
                for (int i = 0; i < left_pos; ++i) {
                    tmp_strs += it[i];
                }
                num = curscope->get_symbol(tmp_strs)->get_values()[num];
                answer += std::to_string(num);
                exps.push_back(std::to_string(num));
            }
        } else if (it == "(" || it == ")" || it == "+" || it == "-" || it == "*" || it == "/" || it == "%" ||
                   it[0] == '\'') {
            if (it[0] == '\'') {
                if(it[1]!='\\'){
                    answer += std::to_string(static_cast<int>(it[1]));
                    exps.push_back(std::to_string(static_cast<int>(it[1])));
                }
                else {
                    if(it[2]=='a'){
                        exps.push_back(std::to_string(static_cast<int>('\a')));
                    }
                    if(it[2]=='b'){
                        exps.push_back(std::to_string(static_cast<int>('\b')));
                    }
                    if(it[2]=='t'){
                        exps.push_back(std::to_string(static_cast<int>('\t')));
                    }
                    if(it[2]=='n'){
                        exps.push_back(std::to_string(static_cast<int>('\n')));
                    }
                    if(it[2]=='v'){
                        exps.push_back(std::to_string(static_cast<int>('\v')));
                    }
                    if(it[2]=='f'){
                        exps.push_back(std::to_string(static_cast<int>('\f')));
                    }
                    if(it[2]=='\"'){
                        exps.push_back(std::to_string(static_cast<int>('\"')));
                    }
                    if(it[2]=='\''){
                        exps.push_back(std::to_string(static_cast<int>('\'')));
                    }
                    if(it[2]=='\\'){
                        exps.push_back(std::to_string(static_cast<int>('\\')));
                    }
                    if(it[2]=='0'){
                        exps.push_back(std::to_string(static_cast<int>('\0')));
                    }
                }
            } else {
                answer += it;
                exps.push_back(it);
            }
        } else {
            int var_value = curscope->get_symbol(it)->get_values()[0];
            answer += std::to_string(var_value);
            exps.push_back(std::to_string(var_value));
        }
    }
//    return eval_expr(answer);
    return Cal_Exp(exps);
}