//
// Created by 13069 on 2024/11/29.
//

#ifndef COMPILER_CODE_OPTIMIZE_H
#define COMPILER_CODE_OPTIMIZE_H

#include<iostream>
#include<vector>
#include <fstream>
#include<function.h>
#include<unordered_map>

//基本块内容
extern std::vector<std::vector<std::string>> stream;

//llvm的块内容
extern std::vector<std::string> llvm_lines;

//输出的内容
extern std::vector<std::string> optimized_lines;

//中间代码优化
void code_optimize();

//第一个优化，去除里面多余的load
void delete_load(std::vector<std::string> stream_lines);

#endif //COMPILER_CODE_OPTIMIZE_H
