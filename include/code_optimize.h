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

//����������
extern std::vector<std::vector<std::string>> stream;

//llvm�Ŀ�����
extern std::vector<std::string> llvm_lines;

//���������
extern std::vector<std::string> optimized_lines;

//�м�����Ż�
void code_optimize();

//��һ���Ż���ȥ����������load
void delete_load(std::vector<std::string> stream_lines);

#endif //COMPILER_CODE_OPTIMIZE_H
