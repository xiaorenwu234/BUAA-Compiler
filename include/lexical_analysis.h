//
// 词法分析
//

#ifndef LEXICAL_ANALYSIS_H
#define LEXICAL_ANALYSIS_H

#include<string>
#include<vector>

//单词匹配
struct word_match {
    std::string word;
    std::string category;
};
//预读取的两个单词
struct word_match_list {
    word_match fir;
    word_match sec;
};

extern word_match paramword;

word_match getsym(int needprint = 1);

//预读取两个单词
word_match_list gettmpsym();

//提前读取一遍获取token总数
void getsym_first_time();

//预读取一个单词
word_match try_read();

std::vector<std::string> preread_params();

std::string preread_single_param();

//token总数
extern int count;
//当前遍历到的token
extern int cur_index;
//上一个非终结符出现的行数
extern int lastline;


extern std::string curscope_funcname;

#endif
