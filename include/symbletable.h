//
// Created by xht  on 2024/10/11.
//

#ifndef COMPILER_SYMBOLTABLE_H
#define COMPILER_SYMBOLTABLE_H

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "function.h"
#include<queue>
#include<intermediate_code.h>
#include<calculate_exp.h>


extern bool hasreturned;
extern bool returnsomething;
extern int blockdepth;
extern bool hasreturned_lastline;

extern bool is_in_for;

// 类型基类
class Type {
public:
    virtual ~Type() = default;

    virtual std::string get_type_name() const = 0;

    //对应的类型占据的空间
    int space_;

    int get_space() const {
        return space_;
    }
};

// 基本类型 - int 和 char
class BasicType : public Type {
public:
    static std::shared_ptr<BasicType> get_int_type() {
        static std::shared_ptr<BasicType> int_type(new BasicType("Int"));
        int_type->space_ = 4;
        return int_type;
    }

    static std::shared_ptr<BasicType> get_char_type() {
        static std::shared_ptr<BasicType> char_type(new BasicType("Char"));
        char_type->space_ = 1;
        return char_type;
    }

    static std::shared_ptr<BasicType> get_const_int_type() {
        static std::shared_ptr<BasicType> const_int_type(new BasicType("ConstInt"));
        const_int_type->space_ = 4;
        return const_int_type;
    }

    static std::shared_ptr<BasicType> get_const_char_type() {
        static std::shared_ptr<BasicType> const_char_type(new BasicType("ConstChar"));
        const_char_type->space_ = 1;
        return const_char_type;
    }

    static std::shared_ptr<BasicType> get_intptr_type() {
        static std::shared_ptr<BasicType> intptr_type(new BasicType("IntArrayPtr"));
        intptr_type->space_ = 4;
        return intptr_type;
    }

    static std::shared_ptr<BasicType> get_charptr_type() {
        static std::shared_ptr<BasicType> charptr_type(new BasicType("CharArrayPtr"));
        charptr_type->space_ = 4;
        return charptr_type;
    }

    std::string get_type_name() const override {
        return name_;
    }

private:
    explicit BasicType(const std::string &name) : name_(name) {}

    std::string name_;
};

// 函数类型 - int 、 char 和 void
class FuncType : public Type {
    //对于func类型的type来说，0偏移存储的是返回值
public:
    static std::shared_ptr<FuncType> get_int_func_type() {
        static std::shared_ptr<FuncType> int_func_type(new FuncType("IntFunc"));
        int_func_type->space_ = 4;
        return int_func_type;
    }

    static std::shared_ptr<FuncType> get_char_func_type() {
        static std::shared_ptr<FuncType> char_func_type(new FuncType("CharFunc"));
        char_func_type->space_ = 4;
        return char_func_type;
    }

    static std::shared_ptr<FuncType> get_void_func_type() {
        static std::shared_ptr<FuncType> void_func_type(new FuncType("VoidFunc"));
        void_func_type->space_ = 0;
        return void_func_type;
    }

    std::string get_type_name() const override {
        return name_;
    }

private:
    explicit FuncType(const std::string &name) : name_(name) {}

    std::string name_;
};

// 数组类型
class ArrayType : public Type {
public:
    ArrayType(std::shared_ptr<Type> element_type, unsigned int size)
            : element_type_(std::move(element_type)), size_(size) {
        space_ = size * element_type_->space_;
        if (size == 0) {
            space_ = element_type_->space_;
        }
    }

    std::string get_type_name() const override {
        return element_type_->get_type_name() + "Array";
    }

    unsigned int get_type_size() const {
        return size_;
    }

private:
    std::shared_ptr<Type> element_type_;
    unsigned int size_;
};

// 符号基类
class Symbol {
public:
    Symbol(std::string name, std::shared_ptr<Type> type, std::vector<int> values, bool global)
            : name_(std::move(name)), type_(std::move(type)), values_(std::move(values)), global_(global) {
        if (global_ || type_->get_type_name().find("Func") != std::string::npos) {
            Symbol_Reg = name_;
        } else {
            Symbol_Reg = std::to_string(register_cnt);
            register_cnt++;
        }
    }

    //修改当前symbol的类型，在将i*转换成i**的时候使用
    void ChangeType(std::shared_ptr<Type> type) {
        type_ = type;
    }

    virtual ~Symbol() = default;

    std::string get_name() const {
        return name_;
    }

    std::string get_type_name() const {
        return type_->get_type_name();
    }

    std::vector<int> get_values() const {
        return values_;
    }

    std::shared_ptr<Type> get_type() const {
        return type_;
    }

    int get_space() const {
        return space_;
    }

    bool get_global() const {
        return global_;
    }

    std::string get_var_reg() const {
        return Symbol_Reg;
    }

    void Refresh_Register() {
        Symbol_Reg = std::to_string(register_cnt);
        register_cnt++;
        if(type_->get_type_name().find("Array")!=std::string::npos){
            if(type_->get_type_name().find("Int")!=std::string::npos){
                ChangeType(BasicType::get_intptr_type());
            }
            else{
                ChangeType(BasicType::get_charptr_type());
            }
        }
    }

protected:
    std::string name_;
    std::shared_ptr<Type> type_;
    std::vector<int> values_;
    //符号的大小
    int space_;
    bool global_;
    //当前符号对应的寄存器计数，计数的值为当前的名字加上当前的寄存器
    std::string Symbol_Reg;
};

// 变量符号类
class VariableSymbol : public Symbol {
public:
    VariableSymbol(const std::string &name, std::shared_ptr<Type> type, std::vector<int> values, bool global)
            : Symbol(name, std::move(type), std::move(values), global) {
        space_ = type_->get_space();
    }
};

// 函数符号类
class FunctionSymbol : public Symbol {
public:
    FunctionSymbol(const std::string &name, std::shared_ptr<Type> type, std::vector<std::shared_ptr<Type>> params,
                   bool global)
            : Symbol(name, std::move(type), std::vector<int>(), global), params_(std::move(params)) {
        space_ = type_->get_space();
        for (auto it: params_) {
            space_ += it->get_space();
        }
    }

    const std::vector<std::shared_ptr<Type>> &get_all_params() const {
        return params_;
    }

private:
    std::vector<std::shared_ptr<Type>> params_;
};

class SymbolTable : public std::enable_shared_from_this<SymbolTable> {
public:
    std::shared_ptr<SymbolTable> push_scope() {
        auto new_table = std::make_shared<SymbolTable>();
        new_table->father_ = shared_from_this();
        return new_table;
    }

    std::shared_ptr<SymbolTable> pop_scope() {
        if (father_) {
            father_->history_.push_back(shared_from_this());  // 保留当前符号表到父作用域的历史中
            return father_;
        }
        return nullptr;
    }

    bool exist_in_scope(const std::string &name) {
        return get_symbol(name) != nullptr;
    }

    bool add_Symbol(const std::shared_ptr<Symbol> &symbol) {
        if (_symbols.find(symbol->get_name()) != _symbols.end()) {
            error_print(line, 'b');
            return false;
        }
        _symbols[symbol->get_name()] = symbol;
        vec_symbols_.push_back(symbol);
        return true;
    }

    std::shared_ptr<Symbol> get_symbol(const std::string &name) {
        auto it = _symbols.find(name);
        if (it != _symbols.end()) {
            return it->second;
        } else if (father_) {
            return father_->get_symbol(name);  // 递归查找父符号表
        } else {
            return nullptr;
        }
    }

    std::vector<std::shared_ptr<Symbol>> get_all_symbols() const {
        return vec_symbols_;
    }

    const std::vector<std::shared_ptr<SymbolTable>> &get_history() const {
        return history_;
    }

private:
    std::unordered_map<std::string, std::shared_ptr<Symbol>> _symbols;
    std::vector<std::shared_ptr<Symbol>> vec_symbols_;
    std::shared_ptr<SymbolTable> father_;
    std::vector<std::shared_ptr<SymbolTable>> history_;
};

// 外部声明
extern std::shared_ptr<SymbolTable> curscope;
extern std::string curtype;
extern std::string curname;
extern int arraylength;
extern bool isConst;
extern std::string stringexp;
extern std::string funcname;

void AddSymble(const std::string &type, const std::string &name, int length, const std::vector<std::string> &params,
               std::vector<int> values);

void traverse_tree();

void judge_params(const std::vector<std::shared_ptr<Symbol>> &pars, const std::string &funcname);

void judge_return(bool hasreturned, bool returnsomething, const std::string &cur_funcname);

std::string judge_class(const std::vector<std::string> &strs);

int cal_exp(std::vector<std::string> values);

#endif //COMPILER_SYMBOLTABLE_H
