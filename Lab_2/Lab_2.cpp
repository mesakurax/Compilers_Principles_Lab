#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include<iomanip>
#include <stack>
using namespace std;

/*
预测分析表构造模块
*/
// 定义产生式的数据结构
struct Production {
    char left;      // 产生式左部
    string right;   // 产生式右部
};

// 从文件中读取文法并存储到unordered_map中
unordered_map<char, vector<string>> readGrammarFromFile(const string& filename) {
    unordered_map<char, vector<string>> grammar;

    ifstream file(filename);    // 打开文件
    string line;

    while (getline(file, line)) {    // 逐行读取文件内容
        stringstream ss(line);       // 使用字符串流读取每行内容
        char nonTerminal;
        char arrow;                 // 用于存储箭头符号
        string production;
        ss >> nonTerminal >> arrow>>arrow;

        // 读取产生式右边的多个部分，使用竖线符号 "|" 分隔
        while (getline(ss, production, '|')) {
            grammar[nonTerminal].push_back(production);    // 将产生式添加到unordered_map中对应的非终结符下
        }
        
    }

    return grammar;
}

// 检查非终结符是否已存在
 bool nonTerminalExists(const unordered_map<char, vector<string>> grammar, char nonTerminal) {
     return grammar.find(nonTerminal) != grammar.end();
 }

// 合并左公共因子
 unordered_map<char, vector<string>> mergeFactor(unordered_map<char, vector<string>> grammar)
 {
     unordered_map<char, vector<string>> newGrammar;
     bool flag = false;

     // 遍历文法中的每个非终结符
     for (const auto& entry : grammar)
     {
         char nonTerminal = entry.first;
         const vector<string>& productions = entry.second;
         
         vector<string> commonFactorProductions;  // 存储具有公共左因子的产生式
         vector<string> nonCommonFactorProductions;  // 存储不具有公共左因子的产生式

         // 只有1个产生式退出
         if (productions.size() == 1)
         {
             newGrammar[nonTerminal] = productions;
             break;
         }

         // 检查是否具有左公共因子的产生式
         bool commonFactor = false;
         for (int i=0;i<productions.size()-1;i++)
         {
             char firstchar = productions[i][0];
             for (int j = i + 1; j < productions.size(); j++)
             {
                 if (firstchar == productions[j][0]) //存在有公共左共因子
                 {
                     commonFactor = true;
                     break;
                 }
             }
             if (commonFactor)  //存储具有左公共因子的产生式和无左公共因子的产生式
             {
                 for (int k = 0; k < productions.size(); k++){
                     if (firstchar == productions[k][0])
                     {
                         commonFactorProductions.push_back(productions[k]);
                     }
                     else
                         nonCommonFactorProductions.push_back(productions[k]);
                 }
                 break;
             }     
         }

         //不存在左公共因子
         if (!commonFactor)
             newGrammar[nonTerminal] = productions;
         else
         {
             flag = true;  //需要进行递归
             string commonPrefix = commonFactorProductions[0];  //找到最长公共前缀
             for (const string& production : commonFactorProductions)
             {
                 size_t prefixLen = 0;
                 while (prefixLen < commonPrefix.length() && prefixLen < production.length() && commonPrefix[prefixLen] == production[prefixLen])
                 {
                     prefixLen++;
                 }
                 commonPrefix = commonPrefix.substr(0, prefixLen);
             }

             char newNonTerminal = 'A';  // 新的非终结符
             while (nonTerminalExists(grammar, newNonTerminal) || nonTerminalExists(newGrammar, newNonTerminal))
             {
                 newNonTerminal++;
             }
             newGrammar[nonTerminal].push_back(commonPrefix + newNonTerminal);  //加入合并项
             for (const string& production : commonFactorProductions)  //加入新项
             {
                 string newProduction = production.substr(commonPrefix.length());
                 if (newProduction.empty())
                 {
                     newProduction = "_";  // 使用_表示空产生式
                 }
                 newGrammar[newNonTerminal].push_back(newProduction);
             }
             for (const string& production : nonCommonFactorProductions)  //加入无公因子项
                 newGrammar[nonTerminal].push_back(production);
         }   
     }

     if (!flag)
         return newGrammar;
     else
         return mergeFactor(newGrammar); //直至没有左公共因子
 }

 // 消除左递归
 unordered_map<char, vector<string>> eLeftRecursion(unordered_map<char, vector<string>> grammar)
 {
     unordered_map<char, vector<string>> newGrammar;
     // 遍历文法中的每个非终结符
     for (const auto& entry : grammar)
     {
         char nonTerminal = entry.first;
         const vector<string>& productions = entry.second;

         size_t p = -1;
         for (int i = 0; i < productions.size(); i++)
         {
             if (productions[i][0] == nonTerminal) { //左递归
                 p = i;
                 break;
             }
         }

         if(p==-1)
             newGrammar[nonTerminal]=productions;
         else
         {
             char newNonTerminal = 'A';  // 新的非终结符
             while (nonTerminalExists(grammar, newNonTerminal) || nonTerminalExists(newGrammar, newNonTerminal))
             {
                 newNonTerminal++;
             }
             for (int i = 0; i < productions.size(); i++)
             {
                 if (i != p)
                     newGrammar[nonTerminal].push_back(productions[i] + newNonTerminal);
             }

             newGrammar[newNonTerminal].push_back(productions[p].substr(1) + newNonTerminal);
             newGrammar[newNonTerminal].push_back("_");    
         }
     }

     return newGrammar;
 }

 //FIRST集合
 unordered_map<char, unordered_set<char>> calculateFirstSet(const unordered_map<char, vector<string>>& grammar) {
     unordered_map<char, unordered_set<char>> firstSet;

     // 初始化每个非终结符的FIRST集合为空集
     for (const auto& entry : grammar) {
         char nonTerminal = entry.first;
         firstSet[nonTerminal] = unordered_set<char>();
     }


     bool updated;
     do {
         updated = false;

         // 遍历每个非终结符的产生式
         for (const auto& entry : grammar) {
             char nonTerminal = entry.first;
             const vector<string>& productions = entry.second;
             
             for (const string& production : productions) {
                 // 如果产生式为空，则将空符号添加到非终结符的FIRST集合中
                 if (production == "_") {
                     if (firstSet[nonTerminal].insert('_').second) {
                         updated = true;
                     }
                 }
                 // 如果产生式以终结符开头，则将该终结符添加到非终结符的FIRST集合中
                 else if (!isupper(production[0]) && production[0] != '_') {
                     if (firstSet[nonTerminal].insert(production[0]).second) {
                         updated = true;
                     }
                 }
                 // 如果产生式以非终结符开头，则将该非终结符的FIRST集合中的符号添加到非终结符的FIRST集合中
                 else if (isupper(production[0])) {
                     int i = 0;
                     while (i < production.length()) {
                         char symbol = production[i];
                         if (isupper(symbol)) {
                             bool hasEpsilon = false;
                             for (char firstSymbol : firstSet[symbol]) {
                                 if (firstSymbol != '_') {
                                     if (firstSet[nonTerminal].insert(firstSymbol).second) {
                                         updated = true;
                                     }
                                 }
                                 else {
                                     hasEpsilon = true;
                                 }
                             }
                             if (!hasEpsilon) {
                                 break;
                             }
                         }
                         else {
                             if (firstSet[nonTerminal].insert(symbol).second) {
                                 updated = true;
                             }
                             break;
                         }
                         i++;
                     }
                 }
             }
         }

     } while (updated);

     return firstSet;
 }

 unordered_map<string, unordered_set<char>> calculateFirstSet_LL1(const unordered_map<char, vector<string>>& grammar) {
     unordered_map<string, unordered_set<char>> firstSet;

     // 初始化每个产生式规则的FIRST集合为空集合
     for (const auto& entry : grammar) {
         char nonTerminal = entry.first;
         const vector<string>& productions = entry.second;

         for (const string& production : productions) {
             firstSet[string(1, nonTerminal) + "->" + production] = unordered_set<char>();
         }
     }

     bool updated;
     do {
         updated = false;

         // 遍历每个产生式规则
         for (const auto& entry : grammar) {
             char nonTerminal = entry.first;
             const vector<string>& productions = entry.second;

             for (const string& production : productions) {
                 // 如果产生式为空，则将空符号_添加到产生式规则的FIRST集合中
                 if (production == "_") {
                     if (firstSet[string(1, nonTerminal) + "->" + production].insert('_').second) {
                         updated = true;
                     }
                 }
                 // 如果产生式以终结符开头，则将该符号添加到产生式规则的FIRST集合中
                 else if (!isupper(production[0]) && production[0] != '_') {
                     if (firstSet[string(1, nonTerminal) + "->" + production].insert(production[0]).second) {
                         updated = true;
                     }
                 }
                 // 如果产生式以非终结符开头，则将该非终结符的FIRST集合中的符号（除空符号外）添加到产生式规则的FIRST集合中
                 else if (isupper(production[0])) {
                     int i = 0;
                     while (i < production.length()) {
                         char symbol = production[i];
                         if (isupper(symbol)) {
                             bool hasEpsilon = false;
                             for (const string& prod : grammar.at(symbol)) {
                                 for (char firstSymbol : firstSet[string(1, symbol) + "->" + prod]) {
                                     if (firstSymbol != '_') {
                                         if (firstSet[string(1, nonTerminal) + "->" + production].insert(firstSymbol).second) {
                                             updated = true;
                                         }
                                     }
                                     else {
                                         hasEpsilon = true;
                                     }
                                 }
                             }
                             if (!hasEpsilon) {
                                 break;
                             }
                         }
                         else {
                             if (firstSet[string(1, nonTerminal) + "->" + production].insert(symbol).second) {
                                 updated = true;
                             }
                             break;
                         }
                         i++;
                     }
                 }
             }
         }
     } while (updated);

     return firstSet;
 }

 //FOLLOW集合
 unordered_map<char, unordered_set<char>> calculateFollowSet(const unordered_map<char, vector<string>>& grammar, const unordered_map<char, unordered_set<char>>& firstSet) {
     unordered_map<char, unordered_set<char>> followSet;

     // 初始化每个非终结符的Follow集合为空集合
     for (const auto& entry : grammar) {
         char nonTerminal = entry.first;
         followSet[nonTerminal] = unordered_set<char>();
     }

     // 设置开始符号的Follow集合为'$'
     char startSymbol = grammar.begin()->first;
     followSet[startSymbol].insert('$');

     bool updated;
     do {
         updated = false;

         // 遍历每个非终结符的产生式规则
         for (const auto& entry : grammar) {
             char nonTerminal = entry.first;
             const vector<string>& productions = entry.second;

             for (const string& production : productions) {
                 // 遍历产生式中的每个符号
                 for (int i = 0; i < production.length(); i++) {
                     char symbol = production[i];

                     // 如果符号是一个非终结符
                     if (isupper(symbol)) {
                         // 如果符号后面还有字符
                         if (i + 1 < production.length()) {
                             char nextSymbol = production[i + 1];

  
                             // Case 1: 如果后一个符号是一个终结符，则将其添加到当前符号的Follow集合中
                             if (!isupper(nextSymbol) && nextSymbol != '_') {
                                 if (followSet[symbol].insert(nextSymbol).second) {
                                     updated = true;
                                 }
                             }
                             // Case 2: 如果后一个符号是一个非终结符，则将其First集合中除空符号外的符号添加到当前符号的Follow集合中
                             else if (isupper(nextSymbol)) {
                                 for (char firstSymbol : firstSet.at(nextSymbol)) {
                                     if (firstSymbol != '_') {
                                         if (followSet[symbol].insert(firstSymbol).second) {
                                             updated = true;
                                         }
                                     }
                                 }
                             }

                             // Case 3: 如果后一个非终结符可以产生空符号，则将当前符号的Follow集合添加到产生式的后续符号的Follow集合中
                             if (isupper(nextSymbol)&&firstSet.at(nextSymbol).size() > 0 && firstSet.at(nextSymbol).count('_') == 1) {
                                 for (char followSymbol : followSet[nonTerminal]) {
                                     if (followSet[symbol].insert(followSymbol).second) {
                                         updated = true;
                                     }
                                 }
                             }
                         }
                         // Case 4: 如果符号是产生式的最后一个符号，则将产生式左侧非终结符的Follow集合添加到当前符号的Follow集合中
                         else {
                             char leftNonTerminal = nonTerminal;

                             if (followSet[leftNonTerminal].size() > 0) {
                                 for (char followSymbol : followSet[leftNonTerminal]) {
                                     if (followSet[symbol].insert(followSymbol).second) {
                                         updated = true;
                                     }
                                 }
                             }
                         }
                     }
                 }
             }
         }
     } while (updated);

     return followSet;
 }

 //LL(1)分析表构造
 unordered_map<char, unordered_map<char, vector<string>>>  constructLL1ParsingTable(const unordered_map<char, vector<string>>& grammar, const unordered_map<string, unordered_set<char>>& firstSet, const unordered_map<char, unordered_set<char>>& followSet) {
     unordered_map<char, unordered_map<char, vector<string>>> parsingTable;

     // 遍历文法中的每个产生式
     for (const auto& entry : grammar) {
         char nonTerminal = entry.first;
         const vector<string>& productions = entry.second;

         // 遍历产生式
         for (const string& production : productions) {
             // 获取产生式的FIRST集
             unordered_set<char> firstSymbols = firstSet.at(string(1, nonTerminal) + "->" + production);

             // 遍历FIRST集中的终结符
             for (char terminal : firstSymbols) {
                 // 将对应的产生式添加到分析表中
                 parsingTable[nonTerminal][terminal].push_back(string(1, nonTerminal) + "->" + production);
             }

             // 如果可以推导出空符号
             if (production=="_") {
                 // 获取产生式左部的FOLLOW集
                 unordered_set<char> followSymbols = followSet.at(nonTerminal);

                 // 遍历FOLLOW集中的终结符
                 for (char terminal : followSymbols) {
                     // 将对应的产生式添加到分析表中
                     parsingTable[nonTerminal][terminal].push_back(string(1, nonTerminal) + "->" + production);
                 }
             }
         }
     }

     return parsingTable;
 }

 bool checkVectorLength(const unordered_map<char, unordered_map<char, vector<string>>>& myMap) {
     for (const auto& outerPair : myMap) {
         for (const auto& innerPair : outerPair.second) {
             const vector<string>& vec = innerPair.second;
             if (vec.size() != 0 && vec.size() != 1) {
                 return false;
             }
         }
     }

     return true;
 }

 bool findProduction(const unordered_map<char, unordered_map<char, vector<string>>>& analysisTable, char nonTerminal, char terminal) {
     auto nonTerminalIter = analysisTable.find(nonTerminal);
     if (nonTerminalIter != analysisTable.end()) {
         auto terminalIter = nonTerminalIter->second.find(terminal);
         if (terminalIter != nonTerminalIter->second.end() && !terminalIter->second.empty()) {
             return true;
         }
     }
     return false;
 }


 //自顶向下进行语法分析
 void LL1Parsing(string input, string output, const unordered_map<char, unordered_map<char, vector<string>>>& LL_1)
 {
     fstream in(input, ios::in);
     string ss;
     in >> ss;
     in.close();

     fstream out(output, ios::app);
     stack<char> symbolStack;
     symbolStack.push('$');  // 栈底符号
     string parseTree = "S"; //语法树

     symbolStack.push('S');  // 文法开始符号
     size_t index = 0;
     char X = symbolStack.top();
     while (X != '$' && ss[index] != '$')
     {
         if (X == ss[index])
         {
             index++;
             symbolStack.pop();
         }
         else {
             if (!findProduction(LL_1, X, ss[index]))
             {
                 out << "Error!!!ignore :" << ss[index] << endl;
                 index++;
             }
             else
             {
                 string v = LL_1.at(X).at(ss[index])[0];
                 out << v << endl;
                 size_t arrowIndex = v.find("->");
                 string pro = v.substr(arrowIndex + 2);

                 if (pro == "_")
                     symbolStack.pop();
                 else
                 {
                     symbolStack.pop();
                     for (int i = pro.length() - 1; i >= 0; --i) {
                         symbolStack.push(pro[i]);
                     }
                 }
             }
         }

         X = symbolStack.top();
     }
     out << endl << endl;
     out.close();
 }


// 输出函数
void printGrammar(const unordered_map<char, vector<string>>& grammar,string output) {
    ofstream out(output, ios::app);
    for (const auto& entry : grammar) {
        char nonTerminal = entry.first;
        const vector<string>& productions = entry.second;
      

        out << nonTerminal << " -> " <<productions[0];
        int i = 1;
        while (i<productions.size()) {
             out<<" | "<<productions[i] ;
             i++;
        }
        out<<endl;
    }
    out << endl << endl;
    out.close();
}
void printSet_1(const unordered_map<char, unordered_set<char>>& firstSet, string output) {
    ofstream out(output, ios::app);
    for (const auto& entry : firstSet) {
        char nonTerminal = entry.first;
        const unordered_set<char>& symbols = entry.second;

        out << "FOLLOW(" << nonTerminal << ") = { ";
        for (char symbol : symbols) {
            out << symbol << "  ";
        }
        out << "}\n";
    }
    out << endl << endl;
    out.close();
}
void printSet_2(const unordered_map<string, unordered_set<char>>& firstSet, string output) {
    ofstream out(output, ios::app);
    for (const auto& entry : firstSet) {
        const string& production = entry.first;
        const unordered_set<char>& firstSymbols = entry.second;

        out << "FIRST(" << production << ") = { ";
        for (char symbol : firstSymbols) {
            out << symbol << "  ";
        }
        out << "}" << endl;
    }
    out << endl << endl;
    out.close();
}
void printLL1ParsingTable(const unordered_map<char, unordered_map<char, vector<string>>>& parsingTable, string output)
{
    // 打开输出文件并追加内容
    ofstream out(output, ios::app);
    out << "+-----------------------+" << endl;
    out << "|  LL(1) Parsing Table  |" << endl;
    out << "+-----------------------+" << endl;

    // 收集所有唯一的终结符号
    unordered_set<char> terminalSymbols;

    for (const auto& row : parsingTable) {
        for (const auto& entry : row.second) {
            char terminal = entry.first;
            if (terminal != '_') {
                terminalSymbols.insert(terminal);
            }
        }
    }

    // 打印带有终结符的标题行
    out << "|     ";

    for (char terminal : terminalSymbols) {
        out << setw(11) << terminal;
    }

    out << "|" << endl;

    // 打印分隔符行
    out << "+";

    for (size_t i = 0; i < terminalSymbols.size(); ++i) {
        out << "----------+";
    }

    out << endl;

    // 打印带有非终结符和产生式规则的行
    for (const auto& row : parsingTable) {
        char nonTerminal = row.first;
        const unordered_map<char, vector<string>>& column = row.second;

        out << "|  " << nonTerminal << "  |";

        for (char terminal : terminalSymbols) {
            if (column.count(terminal) > 0) {
                const vector<string>& productionRules = column.at(terminal);

                if (productionRules.empty()) {
                    out << setw(10) << " ";
                }
                else {
                    out << setw(10) << productionRules[0];
                }
            }
            else {
                out << setw(10) << " ";
            }

            out << "|";
        }

        out << endl;

        // 打印分隔符行
        out << "+";

        for (size_t i = 0; i < terminalSymbols.size(); ++i) {
            out << "----------+";
        }

        out << endl;
    }
    out << endl << endl;
    out.close();
}


void test(string input1, string input2, string output)
{
    string filename = input1;  // 假设文法存储在名为CFG.txt的文件中

    unordered_map<char, vector<string>> grammar = readGrammarFromFile(filename);  // 从文件中读取文法
 
    grammar = mergeFactor(grammar);
    ofstream out1(output, ios::app);
    out1 << "合并左公共因子:" << endl;
    out1.close();
    printGrammar(grammar, output);

    grammar = eLeftRecursion(grammar);
    ofstream out2(output, ios::app);
    out2 << "消除左递归:" << endl;
    out2.close();
    printGrammar(grammar, output);


    unordered_map<string, unordered_set<char>> firstSet = calculateFirstSet_LL1(grammar);
    ofstream out3(output, ios::app);
    out3 << "FIRST 集合:" << endl;
    out3.close();
    printSet_2(firstSet, output);

    unordered_map<char, unordered_set<char>> followSet = calculateFollowSet(grammar, calculateFirstSet(grammar));
    ofstream out4(output, ios::app);
    out4 << "FOLLOW 集合:" << endl;
    out4.close();
    printSet_1(followSet, output);


    unordered_map<char, unordered_map<char, vector<string>>>  LL_1 = constructLL1ParsingTable(grammar, firstSet, followSet);
    if (checkVectorLength(LL_1))
    {
        ofstream out5(output, ios::app);
        out5 << "LL(1) 分析表:" << endl;
        out5.close();
        printLL1ParsingTable(LL_1,  output);
        ofstream out7(output, ios::app);
        out7 << "分析结果如下:" << endl;
        out7.close();
        LL1Parsing(input2, output, LL_1);
    }
    else
    {
        ofstream out6(output, ios::app);
        out6 << "该文法不是LL(1)文法！！！！" << endl;
    }
}


int main() {
    test("CFG_grammar.txt", "CFG_string.txt", "CFG_output.txt");
    test("error_grammar.txt", "error_grammar_string.txt", "error_grammar_output.txt");
    test("CFG2_grammar.txt", "error_string.txt", "error_string_output.txt");
}