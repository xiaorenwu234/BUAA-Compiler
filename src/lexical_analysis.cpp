//
// Created by xht  on 2024/9/20.
//

#include"lexical_analysis.h"
#include"function.h"
#include"category.h"
#include"getfirst.h"

int count=0;
int cur_index=0;

//上一个非终结符出现的行数
int lastline=1;

std::string curscope_funcname;

word_match paramword;

word_match getsym(int needprint) {
    if(needprint){
        lastline=line;
    }
    word_match returnmatch;
    std::string num;
    clearToken();
    if(pointer>=input_length){
        normal_print(inputword.category,inputword.word);
        return returnmatch;
    }

    if(needprint&&cur_index>0) {
        normal_print(inputword.category, inputword.word);
    }

    while (is_space(input[pointer]) || is_newline(input[pointer],needprint) || is_tab(input[pointer])||input[pointer]=='\r') {
        pointer++;
    }

    //判断是否是变量
    if (is_nondigit(input[pointer])) {
        while (is_nondigit(input[pointer]) || is_digit(input[pointer])) {
            cattoken(input[pointer]);
            pointer++;
        }
        int result = classification();
        num = category[result];
    }

        //判断是否是数字
    else if (is_digit(input[pointer])) {
        while (is_digit(input[pointer])) {
            cattoken(input[pointer]);
            pointer++;
        }
        num = category[2];
    }

        //判断是否是字符
    else if (input[pointer] == '\'') {
        cattoken(input[pointer]);
        pointer++;
        if (input[pointer] == '\\') {
            cattoken(input[pointer]);
            pointer++;
        }
        cattoken(input[pointer]);
        pointer++;
        if (input[pointer] == '\'') {
            cattoken(input[pointer]);
            pointer++;
            num = category[4];
        }
    }

        //判断是否是字符串
    else if (input[pointer] == '"') {
        cattoken(input[pointer]);
        pointer++;
        while (input[pointer] != '"') {
            if(input[pointer]=='\\'){
                cattoken(input[pointer]);
                pointer++;
            }
            cattoken(input[pointer]);
            pointer++;
        }
        cattoken(input[pointer]);
        pointer++;
        num = category[3];
    }

        // ! 和 !=
    else if (input[pointer] == '!') {
        cattoken(input[pointer]);
        pointer++;
        if (input[pointer] == '=') {
            cattoken(input[pointer]);
            num = category[32];
            pointer++;
        } else {
            num = category[13];
        }
    }

        // &&
    else if (input[pointer] == '&') {
        cattoken(input[pointer]);
        pointer++;
        if (input[pointer] == '&') {
            cattoken(input[pointer]);
            pointer++;
            num = category[14];
        } else {
            cattoken('&');
            num=category[14];
            if(needprint){
                error_print(line, 'a');
            }
        }
    }
        // ||
    else if (input[pointer] == '|') {
        cattoken(input[pointer]);
        pointer++;
        if (input[pointer] == '|') {
            cattoken(input[pointer]);
            pointer++;
            num = category[15];
        } else {
            cattoken('|');
            num=category[15];
            if(needprint){
                error_print(line, 'a');
            }
        }
    }

        // +
    else if (input[pointer] == '+') {
        cattoken(input[pointer]);
        pointer++;
        num = category[21];
    }

        // -
    else if (input[pointer] == '-') {
        cattoken(input[pointer]);
        pointer++;
        num = category[22];
    }

        // *
    else if (input[pointer] == '*') {
        cattoken(input[pointer]);
        pointer++;
        num = category[24];
    }

        // /
    else if (input[pointer] == '/') {
        cattoken(input[pointer]);
        pointer++;
        num = category[25];
    }

        // %
    else if (input[pointer] == '%') {
        cattoken(input[pointer]);
        pointer++;
        num = category[26];
    }

        // < 和 <=
    else if (input[pointer] == '<') {
        cattoken(input[pointer]);
        pointer++;
        if (input[pointer] == '=') {
            cattoken(input[pointer]);
            num = category[28];
            pointer++;
        } else {
            num = category[27];
        }
    }

        // > 和 >=
    else if (input[pointer] == '>') {
        cattoken(input[pointer]);
        pointer++;
        if (input[pointer] == '=') {
            cattoken(input[pointer]);
            num = category[30];
            pointer++;
        } else {
            num = category[29];
        }
    }

        // = 和 ==
    else if (input[pointer] == '=') {
        cattoken(input[pointer]);
        pointer++;
        if (input[pointer] == '=') {
            cattoken(input[pointer]);
            num = category[31];
            pointer++;
        } else {
            num = category[33];
        }
    }

        // ;
    else if (input[pointer] == ';') {
        cattoken(input[pointer]);
        pointer++;
        num = category[34];
    }

        // ,
    else if (input[pointer] == ',') {
        cattoken(input[pointer]);
        pointer++;
        num = category[35];
    }

        // (
    else if (input[pointer] == '(') {
        cattoken(input[pointer]);
        pointer++;
        num = category[36];
    }

        // )
    else if (input[pointer] == ')') {
        cattoken(input[pointer]);
        pointer++;
        num = category[37];
    }

        // [
    else if (input[pointer] == '[') {
        cattoken(input[pointer]);
        pointer++;
        num = category[38];
    }

        // ]
    else if (input[pointer] == ']') {
        cattoken(input[pointer]);
        pointer++;
        num = category[39];
    }

        // {
    else if (input[pointer] == '{') {
        cattoken(input[pointer]);
        pointer++;
        num = category[40];
    }

        // }
    else if (input[pointer] == '}') {
        cattoken(input[pointer]);
        pointer++;
        num = category[41];
    }

    else{
        pointer++;
    }

    returnmatch.word = token;
    returnmatch.category = num;

    inputword.word=returnmatch.word;
    inputword.category=returnmatch.category;
    cur_index++;

    return returnmatch;
}


word_match_list gettmpsym() {
    int storepointer = pointer;
    std::string storestring = token;
    word_match returnmatch=inputword;
    word_match_list ans;
    ans.fir = getsym(0);
    ans.sec = getsym(0);
    cur_index-=2;
    token = storestring;
    pointer = storepointer;
    inputword=returnmatch;
    return ans;
}

word_match try_read(){
    std::string storestring = token;
    word_match returnmatch=inputword;
    word_match ans;
    ans = getsym(0);
    cur_index--;
    token = storestring;
    inputword=returnmatch;
    return ans;
}

void getsym_first_time(){
    clearToken();
    while (is_space(input[pointer]) || is_newline(input[pointer],false) || is_tab(input[pointer])||input[pointer]=='\r') {
        pointer++;
    }


    //判断是否是变量
    if (is_nondigit(input[pointer])) {
        while (is_nondigit(input[pointer]) || is_digit(input[pointer])) {
            cattoken(input[pointer]);
            pointer++;
        }
    }

        //判断是否是数字
    else if (is_digit(input[pointer])) {
        while (is_digit(input[pointer])) {
            cattoken(input[pointer]);
            pointer++;
        }
    }

        //判断是否是字符
    else if (input[pointer] == '\'') {
        cattoken(input[pointer]);
        pointer++;
        if (input[pointer] == '\\') {
            cattoken(input[pointer]);
            pointer++;
        }
        cattoken(input[pointer]);
        pointer++;
        if (input[pointer] == '\'') {
            cattoken(input[pointer]);
            pointer++;
        }
    }

        //判断是否是字符串
    else if (input[pointer] == '"') {
        cattoken(input[pointer]);
        pointer++;
        while (input[pointer] != '"') {
            if(input[pointer]=='\\'){
                cattoken(input[pointer]);
                pointer++;
            }
            cattoken(input[pointer]);
            pointer++;
        }
        cattoken(input[pointer]);
        pointer++;
    }

        // ! 和 !=
    else if (input[pointer] == '!') {
        cattoken(input[pointer]);
        pointer++;
        if (input[pointer] == '=') {
            cattoken(input[pointer]);
            pointer++;
        }
    }

        // &&
    else if (input[pointer] == '&') {
        cattoken(input[pointer]);
        pointer++;
        if (input[pointer] == '&') {
            cattoken(input[pointer]);
            pointer++;
        }
    }
        // ||
    else if (input[pointer] == '|') {
        cattoken(input[pointer]);
        pointer++;
        if (input[pointer] == '|') {
            cattoken(input[pointer]);
            pointer++;
        }
    }

        // +
    else if (input[pointer] == '+') {
        cattoken(input[pointer]);
        pointer++;
    }

        // -
    else if (input[pointer] == '-') {
        cattoken(input[pointer]);
        pointer++;
    }

        // *
    else if (input[pointer] == '*') {
        cattoken(input[pointer]);
        pointer++;
    }

        // /
    else if (input[pointer] == '/') {
        cattoken(input[pointer]);
        pointer++;
    }

        // %
    else if (input[pointer] == '%') {
        cattoken(input[pointer]);
        pointer++;
    }

        // < 和 <=
    else if (input[pointer] == '<') {
        cattoken(input[pointer]);
        pointer++;
        if (input[pointer] == '=') {
            cattoken(input[pointer]);
            pointer++;
        } else {
        }
    }

        // > 和 >=
    else if (input[pointer] == '>') {
        cattoken(input[pointer]);
        pointer++;
        if (input[pointer] == '=') {
            cattoken(input[pointer]);
            pointer++;
        } else {
        }
    }

        // = 和 ==
    else if (input[pointer] == '=') {
        cattoken(input[pointer]);
        pointer++;
        if (input[pointer] == '=') {
            cattoken(input[pointer]);
            pointer++;
        } else {
        }
    }

        // ;
    else if (input[pointer] == ';') {
        cattoken(input[pointer]);
        pointer++;
    }

        // ,
    else if (input[pointer] == ',') {
        cattoken(input[pointer]);
        pointer++;
    }

        // (
    else if (input[pointer] == '(') {
        cattoken(input[pointer]);
        pointer++;
    }

        // )
    else if (input[pointer] == ')') {
        cattoken(input[pointer]);
        pointer++;
    }

        // [
    else if (input[pointer] == '[') {
        cattoken(input[pointer]);
        pointer++;
    }

        // ]
    else if (input[pointer] == ']') {
        cattoken(input[pointer]);
        pointer++;
    }

        // {
    else if (input[pointer] == '{') {
        cattoken(input[pointer]);
        pointer++;
    }

        // }
    else if (input[pointer] == '}') {
        cattoken(input[pointer]);
        pointer++;
    }

    else{
        pointer++;
    }
    count++;
}

word_match read_word;

std::string preread_single_param(){
    read_word= try_read();
    std::string read_type;
    if(isBType(read_word)){
        read_type=read_word.word;
    }
    read_word=try_read();
    read_word=try_read();
    if(read_word.word=="["){
        read_type+="array";
        while(read_word.word!="]"){
            read_word=try_read();
        }
        read_word=try_read();
    }

    return read_type;
}

std::vector<std::string> preread_params(){
    int startpointer=pointer;
    std::vector<std::string> params_types;
    read_word= paramword;
    std::string read_type;
    if(isBType(read_word)){
        read_type=read_word.word;
    }
    read_word=try_read();
    read_word=try_read();
    if(read_word.word=="["){
        read_type+="array";
        read_word=try_read();
        if(read_word.word=="]"){
            read_word=try_read();
        }
    }
    params_types.push_back(read_type);

    while(read_word.word==","){
        params_types.push_back(preread_single_param());
    }
    pointer=startpointer;
    return params_types;
}
