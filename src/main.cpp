#include"../include/function.h"
#include <fstream>
#include <sstream>
#include<cstdio>
#include<iostream>
#include "grammer_analysis.h"
#include<target_code.h>

void compareFiles(const std::string &file1, const std::string &file2) {
    std::ifstream f1(file1);
    std::ifstream f2(file2);

    if (!f1.is_open() || !f2.is_open()) {
        std::cerr << "cannot open" << std::endl;
        return;
    }

    std::string line1, line2;
    int lineNumber = 1;
    bool areFilesEqual = true;

    while (std::getline(f1, line1) && std::getline(f2, line2)) {
        if (line1 != line2) {
            std::cout << "line: " << lineNumber << std::endl;
            areFilesEqual = false;
        }
        lineNumber++;
    }

    if (f1.eof() != f2.eof()) {
        std::cout << "not correct" << std::endl;
        areFilesEqual = false;
    }

    if (areFilesEqual) {
        std::cout << "correct!" << std::endl;
    }
}

// 自定义函数来读取文件内容
std::string readFile(const std::string& filePath) {
    std::ifstream inputFile(filePath);
    std::string content;
    if (inputFile) {
        std::ostringstream oss;
        oss << inputFile.rdbuf();  // 将文件内容读取到字符串流中
        content = oss.str();
    } else {
        std::cerr << "Error: Cannot open the file: " << filePath << std::endl;
    }
    return content;
}

// 自定义函数来写文件
void writeFile(const std::string& filePath, const std::string& content) {
    std::ofstream outputFile(filePath);
    if (outputFile) {
        outputFile << content;
    } else {
        std::cerr << "Error: Cannot open the file: " << filePath << std::endl;
    }
}

int main() {
    std::remove("error.txt");
    std::remove("symbol.txt");
    std::remove("parser.txt");
    std::remove("llvm_ir.txt");
    std::remove("mips.txt");

    std::ifstream inputFile("testfile.txt");
    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    input = buffer.str();

    input = removeComments(input);

    inputFile.close();
    input_length = int(input.size());


    pointer = 0;
    while (pointer < input_length) {
        getsym_first_time();
    }

    pointer = 0;
    line = 1;
    CompUnit();

    target_generate();

    // 读取 LLVM IR 文件
    std::string llvmIrContent = readFile("llvm_ir.txt");
    if (llvmIrContent.empty()) {
        return 1;  // 如果文件为空，退出
    }

    // 构造优化前后中间代码的文件名
    std::string optimizeBeforeFileName = "testfile122373330薛惠天_优化前中间代码.txt";
    std::string optimizeAfterFileName = "testfile122373330薛惠天_优化后中间代码.txt";


    // 将内容写入不同的文件
    writeFile(optimizeBeforeFileName, llvmIrContent);   // 写入优化前的内容
    writeFile(optimizeAfterFileName, llvmIrContent);  // 写入优化后的内容
    return 0;
}
