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

// �Զ��庯������ȡ�ļ�����
std::string readFile(const std::string& filePath) {
    std::ifstream inputFile(filePath);
    std::string content;
    if (inputFile) {
        std::ostringstream oss;
        oss << inputFile.rdbuf();  // ���ļ����ݶ�ȡ���ַ�������
        content = oss.str();
    } else {
        std::cerr << "Error: Cannot open the file: " << filePath << std::endl;
    }
    return content;
}

// �Զ��庯����д�ļ�
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

    // ��ȡ LLVM IR �ļ�
    std::string llvmIrContent = readFile("llvm_ir.txt");
    if (llvmIrContent.empty()) {
        return 1;  // ����ļ�Ϊ�գ��˳�
    }

    // �����Ż�ǰ���м������ļ���
    std::string optimizeBeforeFileName = "testfile122373330Ѧ����_�Ż�ǰ�м����.txt";
    std::string optimizeAfterFileName = "testfile122373330Ѧ����_�Ż����м����.txt";


    // ������д�벻ͬ���ļ�
    writeFile(optimizeBeforeFileName, llvmIrContent);   // д���Ż�ǰ������
    writeFile(optimizeAfterFileName, llvmIrContent);  // д���Ż��������
    return 0;
}
