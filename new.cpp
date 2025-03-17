#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <utility>
#include <iomanip>
#include <unordered_map>
using namespace std;

struct bookInfo // 需要的信息
{
    string context;
    int page;
    int chapter;
};

struct Book
{
    string name;
    vector<bookInfo> Info;
};

// 罗马数字页码
const unordered_map<string, int> roman_numerals = {
    {"vii", 7}, {"viii", 8}, {"xi", 11}, {"xii", 12}, {"xiii", 13}, {"xiv", 14}, {"xv", 15}, {"xvi", 16}, {"xvii", 17}};

bool is_page_number(const string &str)
{
    // 检查阿拉伯数字
    if (!str.empty() && all_of(str.begin(), str.end(), ::isdigit))
        return true;

    // 检查指定罗马数字（全小写处理）
    string lower_str;
    transform(str.begin(), str.end(), back_inserter(lower_str), ::tolower);
    return roman_numerals.count(lower_str) > 0;
}

// 检查字符串是否只包含数字  获取页码
/*
bool is_only_digit(const std::string &str)
{
    return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
}*/

// 统一转换为数字
int convert_to_number(const string &str)
{
    if (isdigit(str[0]))
        return stoi(str);

    string lower_str;
    transform(str.begin(), str.end(), back_inserter(lower_str), ::tolower);
    return roman_numerals.at(lower_str);
}

// 检查是不是合法的页码
vector<pair<int, int>> get_valid_pages(const vector<string> &content)
{
    vector<pair<int, int>> valid_pages;
    int last_page = -1;
    int last_valid_line = -2;

    for (int i = 0; i < content.size(); ++i)
    {
        if (is_page_number(content[i]))
        {
            int current_page = convert_to_number(content[i]);

            // 第一个合法页码验证
            if (valid_pages.empty())
            {
                // 向后查找整个文件确认连续性
                bool found_continuation = false;
                for (int j = i + 1; j < content.size(); ++j)
                {
                    if (is_page_number(content[j]))
                    {
                        int next_page = convert_to_number(content[j]);
                        if (next_page == current_page + 1)
                        {
                            found_continuation = true;
                            break;
                        }
                    }
                }
                if (found_continuation)
                {
                    valid_pages.emplace_back(i, current_page);
                    last_page = current_page;
                    last_valid_line = i;
                }
            }
            // 后续页码必须严格递增且行号递增
            else if (current_page == last_page + 1 && i > last_valid_line)
            {
                valid_pages.emplace_back(i, current_page);
                last_page = current_page;
                last_valid_line = i;
            }
        }
    }
    return valid_pages;
}

// 找页码
int getPage(int keywordLine, const vector<pair<int, int>> &valid_pages)
{
    // 遍历所有合法页码，找到第一个在关键词行之后的页码
    for (const auto &[line, page] : valid_pages)
    {
        if (line > keywordLine)
        {
            return page;
        }
    }
    return -1; // 未找到后续页码
}
/*
int getPage(ifstream &file, int lineCount)
{
    string line;
    while (lineCount--) // 到达指定行开始找
    {
        getline(file, line);
    }
    while (1)
    {
        getline(file, line);     // 逐行读取
        if (is_only_digit(line)) // 如果这一行是数字
        {
            return stoi(line); // 转换为int
        }
    }
}*/

// 找前一行和当前行的内容
string getContext(const vector<string> &content, int lineNum)
{
    string context;
    if (lineNum > 0)
        context += content[lineNum - 1] + "\n";
    context += content[lineNum];
    return context;
}
/*
string getContext(ifstream &file, int lineCount)
{
    string ans;
    string line;
    while (--lineCount != 1 && lineCount != 0) // 定位到上一行
    {
        getline(file, line);
    }
    getline(file, line); // 读取上一行
    ans += line;         // 添加到答案
    ans += "\n";         // 换行
    getline(file, line); // 读取当前行
    ans += line;         // 添加到答案
    return ans;
}
    */

// 找章节  碰到chapter关键词就+1 ，直到lineCount行
int getChapter(const vector<string> &content, int lineNum)
{
    int chapter = 0;

    for (int i = 0; i < content.size() && i < lineNum; ++i)
    {
        // 统计逻辑保持不变
        if ((content[i].find("Chapter") != string::npos ||
             content[i].find("CHAPTER") != string::npos) &&
            content[i].find("CHAPTER ZERO") == string::npos)
        {
            chapter++;
        }
        else if (content[i].find("Chapter Zero") != string::npos)
        {
            chapter = 0;
        }
    }
    return chapter;
}
/*
int getChapter(ifstream &file, int lineCount)
{
    string line;
    int chapter = 0;
    while (lineCount--)
    {
        getline(file, line);
        if ((line.find("Chapter") != string::npos || line.find("CHAPTER") != string::npos) && line.find("CHAPTER ZERO") == string::npos) // 排除特殊章
        {
            chapter++;
        }
        else if (line.find("Chapter Zero") != string::npos)
        {
            chapter = 0;
        }
    }
    return chapter;
}*/

// 加载书籍信息 同时做完查找将结果放入books中
vector<Book> loadBooks(vector<string> &filenames, string keyword)
{
    vector<Book> books;
    for (auto &filename : filenames)
    {
        // 预加载文件内容
        vector<string> content;
        ifstream file(filename);
        string line;
        while (getline(file, line))
            content.push_back(line);
        file.close();

        // 获取有效页码列表（预处理）
        auto valid_pages = get_valid_pages(content);

        // 处理每一行
        for (int lineNum = 0; lineNum < content.size(); ++lineNum)
        {
            size_t pos = 0;
            while ((pos = content[lineNum].find(keyword, pos)) != string::npos)
            {
                // 获取关联页码
                int page = getPage(lineNum, valid_pages);

                // 获取上下文和章节（需同步修改这些函数）
                string context = getContext(content, lineNum);
                int chapter = getChapter(content, lineNum);

                books.push_back({filename, {{context, page, chapter}}});
                pos += keyword.length();
            }
        }
    }
    return books;
}
/*
vector<Book> loadBooks(vector<string> &filenames, string keyword)
{
    vector<Book> books;
    int page; // 页码
    // bool isFind = false; // 标记是否找到关键词 找到关键词以后才需要找页码
    for (auto filename : filenames)
    {
        ifstream file(filename);          // 打开每一个文件
        string line;                      // 记录每一行的字符串
        unsigned long long lineCount = 0; // 记录在找第几行
        while (getline(file, line))       // 读取每一行
        {
            lineCount++;
            // int index = 0;
            size_t pos = 0;                            // 处理一行多个
            while (line.find(keyword) != string::npos) // 如果这一行包含关键词
            {
                // index = line.find(keyword); // 找到关键词的位置
                //  找页码和章节和上下文
                page = getPage(filename, lineCount);
                string context = getContext(filename, lineCount);
                int chapter = getChapter(filename, lineCount);
                // books.push_back(Book{filename, {{context, page, chapter}}}); // 添加到books中
                books.push_back(Book{
                    filename,
                    {{context, page, chapter}}});
                pos += keyword.length(); // 更新位置
            }
        }
        file.close();
    }
    return books;
}*/

void printResults(vector<Book> &books, string keyword)
{
    // 打印结果
    int count = 0;
    for (int i = 0; i < books.size(); i++)
    {
        for (int j = 0; j < books[i].Info.size(); j++)
        {
            count++;
            cout << count << "\t"
                 << keyword << "\t\t"
                 << books[i].Info[j].page << "\t"
                 << books[i].Info[j].chapter << "\t"
                 << books[i].name << "\n";
        }
    }
}

int main()
{
    vector<string> filenames = {
        /*"./book/HP2--Harry_Potter_and_the_Chamber_of_Secrets_Book_2_.txt",
        "./book/HP7--Harry_Potter_and_the_Deathly_Hallows_Book_7_.txt",
        "./book/J.K. Rowling - HP 3 - Harry Potter and the Prisoner of Azkaban.txt",
        "./book/J.K. Rowling - HP 4 - Harry Potter and the Goblet of Fire.txt",
        "./book/J.K. Rowling - HP 6 - Harry Potter and the Half-Blood Prince.txt",
        "./book/J.K. Rowling - Quidditch Through the Ages.txt",*/
        "../book/J.K. Rowling - The Tales of Beedle the Bard.txt",
        "../book/J.K.Rowling - HP 0 - Harry Potter Prequel.txt"};
    // 注意最后要把../改成./ 因为在vscode中我使用的是C++ compile run 工作台在output文件夹里
    string keyword;
    cout << "Enter keyword: ";
    cin >> keyword;

    vector<Book> books = loadBooks(filenames, keyword);
    cout << "序号\t" << "人名/地名\t" << "页码\t" << "章节\t" << "书名" << endl;
    printResults(books, keyword);
    return 0;
}