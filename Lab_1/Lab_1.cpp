#include <iostream>
#include <fstream>
#include<string>
#include <unordered_map>
#include <iomanip>
using namespace std;

//token
typedef struct token {
    string token_name;
    string lexeme;
}Token;


/*
预处理模块
*/
string Pre_process(string filename)
{
    std::ifstream file(filename,ios::in);
    std::string line;
    std::string result;  //处理结果

    bool inBlockComment = false; // 是否在块注释内

    //每次读取一行
    while (std::getline(file, line)) {   
        std::size_t pos = 0;
        std::size_t len = line.length();

        while (pos < len) {
            if (inBlockComment) {
                std::size_t endPos = line.find("*/", pos);
                if (endPos != std::string::npos) {
                    inBlockComment = false; // 块注释结束
                    pos = endPos + 2;
                }
                else {
                    break; // 忽略块注释内的内容
                }
            }
            else{
                std::size_t lineCommentPos = line.find("//", pos);
                std::size_t blockCommentPos = line.find("/*", pos);

                if (lineCommentPos != std::string::npos) {
                    result += line.substr(pos, lineCommentPos - pos);
                    break; // 忽略行注释后面的内容
                }
                else if (blockCommentPos != std::string::npos) {
                    result += line.substr(pos, blockCommentPos - pos);
                    inBlockComment = true; // 进入块注释
                    pos = blockCommentPos + 2;
                }
                else {
                    result += line.substr(pos);  //读取pos之后的所有字符
                    break;
                }
            }
        }
        result += "\n";   //加入换行符
    }
    file.close();
    return result;
}


/*
* 辅助识别模块
*/
// 判断字符是否是数字
bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

// 判断字符是否是字母
bool isLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// 判断字符是否是空白字符（包括换行符、制表符和空格）
bool isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n';
}


/*
属性表模块
*/
std::unordered_map<std::string, std::string> propertyTable;

//初始化属性表
void InitpropertyTable()
{
    //添加表项
    propertyTable["if"] = "IF";
    propertyTable["int"] = "INT";
    propertyTable["float"] = "FLOAT";
    propertyTable["return"] = "RETURN";
    propertyTable["main"] = "MAIN";
    propertyTable["while"] = "WHILE";
    propertyTable["cin"] = "CIN";
    propertyTable["cout"] = "COUT";
    propertyTable["end"] = "END";
}

/*
token 读取模块
*/
Token getToken(char* start, char* end,int state)
{
    Token temp;
    temp.lexeme = string(start, end);  //为token赋值

    //运算符识别
    std::unordered_map<std::string, std::string> oTable;
    oTable[">"] = "GRE";
    oTable[">="] = "GEQ";
    oTable["<"] = "LSS";
    oTable["="] = "ASSIGN";
    oTable["+"] = "PLUS";
    oTable["+="] = "PLUSSIGN";
    oTable["/"] = "DIV";
    oTable["++"] = "INCREMENT";
    oTable["<<"] = "OSTREAM";
    oTable[">>"] = "ISTREAM";

    if (state == 5 || state == 6 || state == 7 || state == 8 || state == 9 || state == 10
        || state == 11 || state == 12 || state == 13 || state == 14)
        temp.token_name = oTable.find(temp.lexeme)->second;    //查询运算符的词法单元名
    else if (state == 4)
    {
        auto it = propertyTable.find(temp.lexeme);
        if (it == propertyTable.end())
            temp.token_name = "ID";    //标识符
        else
            temp.token_name = it->second;  //关键字
    }
    else if (state == 1)
        temp.token_name = "INTEGER"; //整数
    else if (state == 3)
        temp.token_name = "DECIMAL"; //小数
    else
        temp.token_name = "SEPARATOR";  //分界符

    return temp;  //返回token
}

/*
词法分析模块
*/
void LexicalAnalyzer(string input,string output)
{
    char* begin = &input[0];
    char* end = &input[0];    //设置两个指针

    int state = 0; //初态为0

    ofstream out(output, ios::out);   //输出流
    out << "预处理后代码:\n" << input;
    out << "\n\n词法分析结果如下：" << endl;
    do
    {
        switch (state){
            case 0:
            {
                if (isDigit(*end)){
                    state = 1;
                    end++;
                }
                else if (isLetter(*end) || *end == '_'){
                    state = 4;
                    end++;
                }
                else if (isWhitespace(*end)){
                    begin++; 
                    end++;
                }
                else if (*end == '>') {
                    state = 5;
                    end++;
                }
                else if (*end == '<') {
                    state = 8;
                    end++;
                }
                else if (*end == '=') {
                    state = 10;
                    end++;
                }
                else if (*end == '+') {
                    state = 11;
                    end++;
                }
                else if (*end == '/') {
                    state = 14;
                    end++;
                }
                else if (*end == '('|| *end == ')' || *end == '{' || *end == '}' 
                    || *end == ',' || *end == ';') {
                    state = 15;
                    end++;
                }
                else{
                    out << "Error!!!!!" << endl;  //报错
                    end++;
                    begin = end;
                }
                break;
            }
            case 1:
            {
                if (isDigit(*end)) {
                    state = 1;
                    end++;
                }
                else if (*end == '.') {
                    state = 2;
                    end++;
                }
                else {
                    Token temp = getToken(begin, end,state);  //获取token
                 out << "<" << std::setw(10) << temp.token_name << " , " << std::setw(7) << temp.lexeme << ">" << std::endl;
                    state = 0;
                    begin = end;  //这里相当于回退了一个字符
                }
                break;
            }
            case 2:
            {
                if (isDigit(*end)) {
                    end++;
                    state = 3;
                }
                else
                {
                    out << "Error!!!!!" << endl;  //报错
                    begin = end;
                    state = 0;
                }
                break;
            }
            case 3:
            {
                if (isDigit(*end)) {
                    end++;
                    state = 3;
                }
                else {
                    Token temp = getToken(begin, end, state);  //获取token
                 out << "<" << std::setw(10) << temp.token_name << " , " << std::setw(7) << temp.lexeme << ">" << std::endl;
                    state = 0;
                    begin = end;  //这里相当于回退了一个字符
                }
                break;
            }
            case 4:
            {
                if (isLetter(*end) || *end == '_'||isDigit(*end)) {
                    state = 4;
                    end++;
                }
                else {
                    Token temp = getToken(begin, end, state);  //获取token
                 out << "<" << std::setw(10) << temp.token_name << " , " << std::setw(7) << temp.lexeme << ">" << std::endl;
                    state = 0;             
                    begin = end;  //这里相当于回退了一个字符
                }
                break;
            }
            case 5:
            {
                if (*end == '=') {
                    state = 6;
                    end++;
                }
                else if (*end == '>') {
                    state = 7;
                    end++;
                }
                else {
                    Token temp = getToken(begin, end, state);  //获取token
                 out << "<" << std::setw(10) << temp.token_name << " , " << std::setw(7) << temp.lexeme << ">" << std::endl;
                    state = 0;
                    begin = end;  //这里相当于回退了一个字符
                }
                break;
            }
            case 6:
            {
                Token temp = getToken(begin, end, state);  //获取token
             out << "<" << std::setw(10) << temp.token_name << " , " << std::setw(7) << temp.lexeme << ">" << std::endl;
                state = 0;
                begin = end;  //这里相当于回退了一个字符
                break;
            }
            case 7:
            {
                Token temp = getToken(begin, end, state);  //获取token
             out << "<" << std::setw(10) << temp.token_name << " , " << std::setw(7) << temp.lexeme << ">" << std::endl;
                state = 0;
                begin = end;  //这里相当于回退了一个字符
                break;
            }
            case 8:
            {
                if (*end == '<') {
                    state = 9;
                    end++;
                }
                else {
                    Token temp = getToken(begin, end, state);  //获取token
                 out << "<" << std::setw(10) << temp.token_name << " , " << std::setw(7) << temp.lexeme << ">" << std::endl;
                    state = 0;
                    begin = end;  //这里相当于回退了一个字符
                }
                break;
            }
            case 9:
            {
                Token temp = getToken(begin, end, state);  //获取token
             out << "<" << std::setw(10) << temp.token_name << " , " << std::setw(7) << temp.lexeme << ">" << std::endl;
                state = 0;
                begin = end;  //这里相当于回退了一个字符
                break;
            }
            case 10:
            {
                Token temp = getToken(begin, end, state);  //获取token
             out << "<" << std::setw(10) << temp.token_name << " , " << std::setw(7) << temp.lexeme << ">" << std::endl;
                state = 0;
                begin = end;  //这里相当于回退了一个字符
                break;
            }
            case 11:
            {
                if (*end == '+') {
                    state = 12;
                    end++;
                }
                else if (*end == '=') {
                    state = 13;
                    end++;
                }
                else {
                    Token temp = getToken(begin, end, state);  //获取token
                 out << "<" << std::setw(10) << temp.token_name << " , " << std::setw(7) << temp.lexeme << ">" << std::endl;
                    state = 0;
                    begin = end;  //这里相当于回退了一个字符
                }
                break;
            }
            case 12:
            {
                Token temp = getToken(begin, end, state);  //获取token
             out << "<" << std::setw(10) << temp.token_name << " , " << std::setw(7) << temp.lexeme << ">" << std::endl;
                state = 0;
                begin = end;  //这里相当于回退了一个字符
                break;
            }
            case 13:
            {
                Token temp = getToken(begin, end, state);  //获取token
             out << "<" << std::setw(10) << temp.token_name << " , " << std::setw(7) << temp.lexeme << ">" << std::endl;
                state = 0;
                begin = end;  //这里相当于回退了一个字符
                break;
            }
            case 14:
            {
                Token temp = getToken(begin, end, state);  //获取token
             out << "<" << std::setw(10) << temp.token_name << " , " << std::setw(7) << temp.lexeme << ">" << std::endl;
                state = 0;
                begin = end;  //这里相当于回退了一个字符
                break;
            }
            case 15:
            {
                Token temp = getToken(begin, end, state);  //获取token
             out << "<" << std::setw(10) << temp.token_name << " , " << std::setw(7) << temp.lexeme << ">" << std::endl;
                state = 0;
                begin = end;  //这里相当于回退了一个字符
                break;
            }
        }
    } while (*begin != '\0');  //读取结束
    out.close();
}


int main()
{
    InitpropertyTable();  //初始化属性表

    //词法分析
    string result = Pre_process("raw_code.txt"); //消除注释
    LexicalAnalyzer(result,"LexicalAnalyzer_output.txt");

    //错误处理示意
    result = Pre_process("error.txt"); //消除注释
    LexicalAnalyzer(result,"error_Handle.txt");
}


