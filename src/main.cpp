#include"../include/function.h"
#include <fstream>
#include <sstream>
#include<cstdio>
#include<iostream>
#include "grammer_analysis.h"

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

int main() {
    std::remove("error.txt");
    std::remove("symbol.txt");
    std::remove("parser.txt");
    std::remove("llvm_ir.txt");

    std::ifstream inputFile("testfile.txt");
    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    input = buffer.str();

    input= removeComments(input);

    inputFile.close();
    input_length = int(input.size());


    pointer = 0;
    while (pointer < input_length) {
        getsym_first_time();
    }

    pointer = 0;
    line=1;
    CompUnit();

    std::string file1 = "error.txt";
    std::string file2 = "ans.txt";

    //compareFiles(file1, file2);
    return 0;
}
