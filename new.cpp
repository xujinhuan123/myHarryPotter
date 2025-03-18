#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <utility>
#include <iomanip>
#include <unordered_map>
#include <locale>
#include <windows.h>
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

string UTF8ToGB(const char *str)
{
    string result;
    WCHAR *strSrc;
    LPSTR szRes;

    int i = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    strSrc = new WCHAR[i + 1];
    MultiByteToWideChar(CP_UTF8, 0, str, -1, strSrc, i);

    i = WideCharToMultiByte(CP_ACP, 0, strSrc, -1, NULL, 0, NULL, NULL);
    szRes = new CHAR[i + 1];
    WideCharToMultiByte(CP_ACP, 0, strSrc, -1, szRes, i, NULL, NULL);

    result = szRes;
    delete[] strSrc;
    delete[] szRes;

    return result;
}

// 罗马数字页码
const unordered_map<string, int> roman_numerals = {
    {"vii", 760}, {"viii", 761}, {"xi", 762}, {"xii", 763}, {"xiii", 764}, {"xiv", 765}, {"xv", 766}, {"xvi", 767}, {"xvii", 768}};

bool is_page_number(const string &str)
{
    // 阿拉伯数字验证（1-99999）
    if (!str.empty() && all_of(str.begin(), str.end(), ::isdigit))
    {
        int num = stoi(str);
        return num > 0 && num < 100000;
    }

    // 罗马数字验证（限定映射表范围）
    string lower_str = str;
    transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
    return roman_numerals.find(lower_str) != roman_numerals.end();
}

// 统一转换为数字
int convert_to_number(const string &str)
{
    try
    {
        if (isdigit(str[0]))
        {
            return stoi(str);
        }
        else
        {
            string lower_str = str;
            transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
            return roman_numerals.at(lower_str);
        }
    }
    catch (...)
    {
        return -1; // 无效页码
    }
}

// 检查是不是合法的页码
vector<pair<int, int>> get_valid_pages(const vector<string> &content)
{
    vector<pair<int, int>> valid_pages;
    int last_page = -1;
    int last_valid_line = -2;
    const int MAX_PAGE_JUMP = 10;
    bool roman_sequence_end = false; // 新增：标记罗马数字序列结束

    for (int i = 0; i < content.size(); ++i)
    {
        if (is_page_number(content[i]))
        {
            int current_page = convert_to_number(content[i]);
            bool is_roman = !isdigit(content[i][0]);

            // 处理罗马数字后的第一个阿拉伯数字
            if (roman_sequence_end && !is_roman)
            {
                // 作为新序列起点独立验证
                bool valid_new_sequence = false;

                // 验证后续是否存在连续页码
                for (int j = i + 1; j < content.size(); ++j)
                {
                    if (is_page_number(content[j]))
                    {
                        int next_page = convert_to_number(content[j]);
                        if (next_page > current_page && next_page <= current_page + MAX_PAGE_JUMP)
                        {
                            valid_new_sequence = true;
                            break;
                        }
                    }
                }

                if (valid_new_sequence)
                {
                    // 重置原有序列
                    valid_pages.emplace_back(i, current_page);
                    last_page = current_page;
                    last_valid_line = i;
                    roman_sequence_end = false; // 重置标记
                    continue;                   // 跳过后续检查
                }
            }

            // 主验证逻辑
            if (valid_pages.empty())
            {
                bool found_continuation = false;
                for (int j = i + 1; j < content.size(); ++j)
                {
                    if (is_page_number(content[j]))
                    {
                        int next_page = convert_to_number(content[j]);
                        if (next_page > current_page && next_page <= current_page + MAX_PAGE_JUMP)
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
                    roman_sequence_end = is_roman; // 记录罗马数字序列开始
                }
            }
            else if (current_page > last_page &&
                     current_page <= last_page + MAX_PAGE_JUMP &&
                     i > last_valid_line)
            {
                valid_pages.emplace_back(i, current_page);
                last_page = current_page;
                last_valid_line = i;

                // 当切换到阿拉伯数字时标记序列结束
                if (roman_sequence_end && !is_roman)
                {
                    roman_sequence_end = false;
                }
                else
                {
                    roman_sequence_end = is_roman;
                }
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

// 找前一行和当前行的内容
string getContext(const vector<string> &content, int lineNum)
{
    string context;
    if (lineNum > 0)
        context += content[lineNum - 1] + "\n";
    context += content[lineNum];
    return context;
}

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

int printResults(vector<Book> &books, string keyword)
{
    // 反向映射
    std::unordered_map<int, std::string> reverse_roman_numerals;

    // 填充反向映射
    for (const auto &pair : roman_numerals)
    {
        reverse_roman_numerals[pair.second] = pair.first;
    }
    // 打印结果
    int count = 0;
    for (int i = 0; i < books.size(); i++)
    {
        for (int j = 0; j < books[i].Info.size(); j++)
        {
            count++;
            cout << count << "\t"
                 << keyword << "\t\t";
            //<< books[i].Info[j].page << "\t"
            if (books[i].Info[j].page > 759)
            {
                auto it = reverse_roman_numerals.find(books[i].Info[j].page);
                cout << it->second << "\t";
            }
            else
                cout << books[i].Info[j].page << "\t";
            cout << books[i].Info[j].chapter << "\t"
                 << books[i].name << "\n";
        }
    }
    return count;
}

void printCOntext(int count, vector<Book> &books)
{
    cout << "上下文:" << endl;
    int index = 0;
    for (int i = 0; i < books.size(); i++)
    {
        for (int j = 0; j < books[i].Info.size(); j++)
        {
            index++;
            if (index == count) // 找到对应的上下文
            {
                cout << UTF8ToGB(books[i].Info[j].context.c_str()) << endl;
                return;
            }
        }
    }
}

int main()
{

    vector<string> filenames = {
        "./book/HP2--Harry_Potter_and_the_Chamber_of_Secrets_Book_2_.txt",
        "./book/HP7--Harry_Potter_and_the_Deathly_Hallows_Book_7_.txt",
        "./book/J.K. Rowling - HP 3 - Harry Potter and the Prisoner of Azkaban.txt",
        "./book/J.K. Rowling - HP 4 - Harry Potter and the Goblet of Fire.txt",
        "./book/J.K. Rowling - HP 6 - Harry Potter and the Half-Blood Prince.txt",
        "./book/J.K. Rowling - Quidditch Through the Ages.txt",
        "./book/J.K. Rowling - The Tales of Beedle the Bard.txt",
        "./book/J.K.Rowling - HP 0 - Harry Potter Prequel.txt"};
    // 注意最后要把../改成./ 因为在vscode中我使用的是C++ compile run 工作台在output文件夹里
    string keyword;
    cout << "Enter keyword: ";
    cin >> keyword;

    vector<Book> books = loadBooks(filenames, keyword);
    cout << "序号\t" << "人名/地名\t" << "页码\t" << "章节\t" << "书名" << endl;
    int num = printResults(books, keyword);
    if (num == 0)
    {
        cout << "No results found." << endl;
        return 0;
    }
    while (num > 0)
    {
        int count;
        cout << "Enter count: ";
        cin >> count;
        printCOntext(count, books);
        cout << "是否继续？(1/0)" << endl;
        int flag;
        cin >> flag;
        if (flag == 0)
            break;
    }
    return 0;
}