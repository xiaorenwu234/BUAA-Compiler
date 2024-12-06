//
// 杂项函数
//

#ifndef FUNCTION_H
#define FUNCTION_H

#include<string>
#include<lexical_analysis.h>
#include<stack>
#include <sstream>
#include<regex>

bool is_digit(char x);

bool is_nondigit(char x);

bool is_space(char x);

bool is_newline(char x, bool needAdd);

bool is_tab(char x);

extern std::string token;
extern word_match inputword;

//遍历到的行数
extern int line;

//输入内容
extern std::string input;
//输入内容长度
extern int input_length;
//指针
extern int pointer;

//清空token
void clearToken();

//连接token
void cattoken(char x);

//分类
int classification();

//输出错误
void error_print(int line_num, char error_num);

//正常输出
void normal_print(const std::string &cate, const std::string &data);

//语法输出
void grammer_print(const std::string &data);

//llvm中间代码输出
void llvm_print(const std::string &str);

//mips目标代码输出
void mips_print(const std::string &str);

//优化代码输出
void optimize_print(const std::string &str);

//清除注释
std::string removeComments(const std::string &code);

//语义输出
void semantics_print(int depth, const std::string &var, const std::string &type);

std::string remove_end_array(std::string curtype);

bool is_string_number(const std::string &str);

bool is_string_char(const std::string &str);

bool end_with(const std::string &str, const std::string &suffix);

bool start_with(const std::string &str, const std::string &prefix);

int count_print(const std::string &formatstring);

//将行中的内容按照空格分割，返回一个vector
std::vector<std::string> Split_Line(const std::string &input_line,char split_char);

//判断当前的类型占据的空间是4还是1字节
int judge_type(const std::string &str);

//子字符串替换
std::string replaceSubstring(const std::string &str, const std::string &old_sub, const std::string &new_sub);

#endif
