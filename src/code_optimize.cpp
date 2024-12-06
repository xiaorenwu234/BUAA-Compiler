//
// Created by 13069 on 2024/11/29.
//

#include <code_optimize.h>


//基本块内容
std::vector<std::vector<std::string>> stream;

//判断当前的临时寄存器是否被修改过
std::unordered_map<std::string, bool> has_changed;

//llvm的块内容
std::vector<std::string> llvm_lines;

//输出的内容
std::vector<std::string> optimized_lines;

//load进来的临时寄存器和原来的对应
std::unordered_map<std::string, std::string> load_match;

//当前读到的行数
int current_index = 0;

void split_llvm_ir() {
    std::ifstream file("llvm_ir.txt");
    std::string llvm_line;
    while (std::getline(file, llvm_line)) {
        llvm_lines.push_back(llvm_line);  // 将每一行存入vector中
    }
    file.close();
}

void code_optimize() {
    split_llvm_ir();
    std::vector<std::string> stream_lines_tmp;
    bool in_function = false;
    for (current_index = 0; current_index < llvm_lines.size(); ++current_index) {
        if (llvm_lines[current_index].find("define") != std::string::npos) {
            optimized_lines.push_back(llvm_lines[current_index]);
            in_function = true;
        } else if (!in_function) {
            optimized_lines.push_back(llvm_lines[current_index]);
        } else if (llvm_lines[current_index].find('}') != std::string::npos) {
            optimized_lines.push_back(llvm_lines[current_index]);
            in_function = false;
        } else if (llvm_lines[current_index].find(':') != std::string::npos) {
            if(!stream_lines_tmp.empty()){
                delete_load(stream_lines_tmp);
            }
            stream_lines_tmp.clear();
            optimized_lines.push_back(llvm_lines[current_index]);
        } else if ((llvm_lines[current_index].find("ret") != std::string::npos ||
                    llvm_lines[current_index].find("br") != std::string::npos) &&
                   llvm_lines[current_index].find('@') == std::string::npos) {
            delete_load(stream_lines_tmp);
            stream_lines_tmp.clear();
            optimized_lines.push_back(llvm_lines[current_index]);
            while (llvm_lines[current_index].find(':') == std::string::npos &&
                   llvm_lines[current_index].find('}') == std::string::npos) {
                current_index++;
            }
            current_index--;
        } else {
            stream_lines_tmp.push_back(llvm_lines[current_index]);
        }
    }
    for (const auto &optimizedLine: optimized_lines) {
        optimize_print(optimizedLine + '\n');
    }
}

void delete_load(std::vector<std::string> stream_lines) {
    has_changed.clear();
    load_match.clear();
    for (int i = 0; i < stream_lines.size(); ++i) {
        auto stream_line = stream_lines[i];
        if (stream_line.find("alloca") != std::string::npos) {
            optimized_lines.push_back(stream_line);
        } else if (stream_line.find("store") != std::string::npos) {
            auto elements = Split_Line(stream_line, ' ');
            has_changed[elements[elements.size() - 1]] = true;
            optimized_lines.push_back(stream_line);
        } else if (stream_line.find("load") != std::string::npos) {
            auto elements = Split_Line(stream_line, ' ');
            if (has_changed.find(elements[elements.size() - 1]) != has_changed.end()) {
                if (has_changed[elements[elements.size() - 1]] ||
                    load_match.find(elements[elements.size() - 1]) == load_match.end()) {
                    optimized_lines.push_back(stream_line);
                    load_match[elements[elements.size() - 1]] = elements[0];
                } else {
                    for (int j = i + 1; j < stream_lines.size(); ++j) {
                        stream_lines[j] = replaceSubstring(stream_lines[j]+' ', elements[0]+' ',
                                                           load_match[elements[elements.size() - 1]]+' ');
                    }
                    for (int j = current_index; j < llvm_lines.size(); ++j) {
                        if (llvm_lines[j].find('}') != std::string::npos) {
                            break;
                        }
                        llvm_lines[j] = replaceSubstring(llvm_lines[j]+' ', elements[0]+' ',
                                                         load_match[elements[elements.size() - 1]]+' ');
                    }
                }
            } else {
                optimized_lines.push_back(stream_line);
                load_match[elements[elements.size() - 1]] = elements[0];
            }
            has_changed[elements[elements.size() - 1]] = false;
        } else {
            optimized_lines.push_back(stream_line);
        }
    }
}