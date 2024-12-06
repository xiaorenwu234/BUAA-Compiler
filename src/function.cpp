//
// Created by xht  on 2024/9/20.
//
#include "function.h"
#include<iostream>
#include <fstream>


bool is_digit(char x) {
    return x <= '9' && x >= '0';
}

bool is_nondigit(char x) {
    return (x <= 'z' && x >= 'a') || (x <= 'Z' && x >= 'A') || x == '_';
}


std::string token;

word_match inputword;

void clearToken() {
    token = "";
}

bool is_space(char x) {
    return x == ' ';
}

int line = 1;

bool is_newline(char x, bool needAdd) {
    if (x == '\n') {
        if (needAdd) {
            line++;
        }
        return true;
    }
    return false;
}

std::string input;
int input_length = 0;
int pointer = 0;

bool is_tab(char x) {
    return x == '\t';
}

void cattoken(char x) {
    token += x;
}

int classification() {
    if (token == "main") {
        return 5;
    }
    if (token == "const") {
        return 6;
    }
    if (token == "int") {
        return 7;
    }
    if (token == "char") {
        return 8;
    }
    if (token == "break") {
        return 9;
    }
    if (token == "continue") {
        return 10;
    }
    if (token == "if") {
        return 11;
    }
    if (token == "else") {
        return 12;
    }
    if (token == "!") {
        return 13;
    }
    if (token == "&&") {
        return 14;
    }
    if (token == "||") {
        return 15;
    }
    if (token == "for") {
        return 16;
    }
    if (token == "getint") {
        return 17;
    }
    if (token == "getchar") {
        return 18;
    }
    if (token == "printf") {
        return 19;
    }
    if (token == "return") {
        return 20;
    }
    if (token == "+") {
        return 21;
    }
    if (token == "-") {
        return 22;
    }
    if (token == "void") {
        return 23;
    }
    if (token == "*") {
        return 24;
    }
    if (token == "/") {
        return 25;
    }
    if (token == "%") {
        return 26;
    }
    if (token == "<") {
        return 27;
    }
    if (token == "<=") {
        return 28;
    }
    if (token == ">") {
        return 29;
    }
    if (token == ">=") {
        return 30;
    }
    if (token == "==") {
        return 31;
    }
    if (token == "!=") {
        return 32;
    }
    if (token == "=") {
        return 33;
    }
    if (token == ";") {
        return 34;
    }
    if (token == ",") {
        return 35;
    }
    if (token == "(") {
        return 36;
    }
    if (token == ")") {
        return 37;
    }
    if (token == "[") {
        return 38;
    }
    if (token == "]") {
        return 39;
    }
    if (token == "{") {
        return 40;
    }
    if (token == "}") {
        return 41;
    }
    return 1;
}

void error_print(int line_num, char error_num) {
    std::ofstream outputFile("error.txt", std::ios::app);
    std::string str_line_num = std::to_string(line_num);
    std::string output = str_line_num + " " + error_num + '\n';
    outputFile << output;
    outputFile.close();
}


void normal_print(const std::string &cate, const std::string &data) {
    std::ofstream outputFile("parser.txt", std::ios::app);
    std::string output = cate + " " + data + '\n';
    outputFile << output;
    outputFile.close();
}


void grammer_print(const std::string &data) {
    std::ofstream outputFile("parser.txt", std::ios::app);
    std::string output = "<" + data + ">" + '\n';
    outputFile << output;
    outputFile.close();
}

void semantics_print(int depth, const std::string &var, const std::string &type) {
    std::ofstream outputFile("symbol.txt", std::ios::app);
    std::string output = std::to_string(depth) + " " + var + " " + type + "\n";
    outputFile << output;
    outputFile.close();
}

void llvm_print(const std::string &str) {
    std::ofstream outputFile("llvm_ir.txt", std::ios::app);
    outputFile << str;
    outputFile.close();
}

void mips_print(const std::string &str) {
    std::ofstream outputFile("mips.txt", std::ios::app);
    outputFile << str;
    outputFile.close();
}

void optimize_print(const std::string &str) {
    std::ofstream outputFile("optimized_llvm.txt", std::ios::app);
    outputFile << str;
    outputFile.close();
}

std::string removeComments(const std::string &code) {
    std::string result; // 存储去掉注释后的代码
    bool inBlockComment = false; // 判断是否在块注释中
    bool inLineComment = false; // 判断是否在行注释中
    bool inString = false; // 判断是否在字符串中
    std::stack<bool> blockCommentStack; // 用于处理嵌套的块注释

    for (size_t i = 0; i < code.length(); ++i) {
        // 检查是否在字符串中
        if (!inBlockComment && !inLineComment && code[i] == '\"') {
            inString = !inString; // 切换字符串状态
            result += code[i]; // 保留引号
            continue;
        }

        // 如果在字符串中，直接添加到结果
        if (inString) {
            result += code[i];
            continue;
        }

        // 检查行注释
        if (!inBlockComment && !inLineComment && code[i] == '/' && i + 1 < code.length() && code[i + 1] == '/') {
            inLineComment = true;
            continue; // 跳过当前字符
        }

        // 检查块注释开始
        if (!inLineComment && code[i] == '/' && i + 1 < code.length() && code[i + 1] == '*') {
            inBlockComment = true;
            blockCommentStack.push(true); // 入栈
            i++; // 跳过下一个字符
            continue; // 跳过当前字符
        }

        // 检查块注释结束
        if (inBlockComment && code[i] == '*' && i + 1 < code.length() && code[i + 1] == '/') {
            if (!blockCommentStack.empty()) {
                blockCommentStack.pop(); // 出栈
            }
            if (blockCommentStack.empty()) {
                inBlockComment = false; // 结束块注释
            }
            i++; // 跳过下一个字符
            continue; // 跳过当前字符
        }

        // 检查行注释结束
        if (inLineComment && code[i] == '\n') {
            inLineComment = false; // 结束行注释
        }

        // 如果不在任何注释中，添加当前字符到结果中
        if (!inBlockComment && !inLineComment) {
            result += code[i];
        }
    }

    return result;
}

//如果后面有array，就去掉
std::string remove_end_array(std::string curtype) {
    std::string suffix = "array";
    if (curtype.size() >= suffix.size() &&
        curtype.substr(curtype.size() - suffix.size()) == suffix) {
        return curtype.substr(0, curtype.size() - suffix.size());
    }
    return curtype;
}

bool is_string_number(const std::string &str) {
    if (str.empty()) return false;
    size_t start = 0;
    if (str[0] == '-') {
        start = 1;
    }
    for (size_t i = start; i < str.size(); i++) {
        if (!std::isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

bool is_string_char(const std::string &str) {
    if (str[0] != '\'' || str[2] != '\'') {
        return false;
    }
    return true;
}

bool end_with(const std::string &str, const std::string &suffix) {
    if (suffix.size() > str.size()) {
        return false;
    }
    return str.rfind(suffix) == (str.size() - suffix.size());
}

bool start_with(const std::string &str, const std::string &prefix) {
    if (prefix.size() > str.size()) {
        return false;
    }
    return str.find(prefix) == 0;
}


int countSubstring(const std::string &str, const std::string &substr) {
    int count = 0;
    std::size_t pos = str.find(substr);  // 查找子字符串的位置
    while (pos != std::string::npos) {
        count++;
        pos = str.find(substr, pos + 1);  // 从上次位置之后继续查找
    }
    return count;
}

int count_print(const std::string &formatstring) {
    return countSubstring(formatstring, "%c") + countSubstring(formatstring, "%d");
}

std::string trim(const std::string &str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos) return "";  // 如果没有非空字符，返回空字符串
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, last - first + 1);
}

std::vector<std::string> Split_Line(const std::string &input_line, char split_char) {
    std::vector<std::string> result;
    std::string tmp;
    for (int i = 0; i < input_line.size(); ++i) {
        if (input_line[i] == split_char && !tmp.empty()) {
            if (!trim(tmp).empty()) {
                result.push_back(trim(tmp));
            }
            tmp.clear();
            continue;
        }
        if (input_line[i] == split_char) {
            continue;
        }
        tmp += input_line[i];
    }
    if (!trim(tmp).empty()) {
        result.push_back(trim(tmp));
    }
    return result;
}


int judge_type(const std::string &str) {
    if (str.find("i32") != std::string::npos || str.find('*') != std::string::npos) {
        return 4;
    }
    return 1;
}


std::string replaceSubstring(const std::string &str, const std::string &old_sub, const std::string &new_sub) {
    size_t pos = 0;
    std::string result=str;
    while ((pos = str.find(old_sub, pos)) != std::string::npos) {
        result.replace(pos, old_sub.length(), new_sub);
        pos += new_sub.length();
    }
    return result;
}


//std::string replaceSubstring(const std::string &str, const std::string &old_sub, const std::string &new_sub) {
//    std::string result;
//    auto elements= Split_Line(str,' ');
//    for(int i=0;i<elements[i].size();++i){
//        if(elements[i]==old_sub){
//            elements[i]=new_sub;
//        }
//        result+=elements[i];
//    }
//    return result;
//}