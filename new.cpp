#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <iomanip>
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

// 检查字符串是否只包含数字  获取页码
bool is_only_digit(const std::string &str)
{
    return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
}
// 找页码
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
}

// 找前一行和当前行的内容
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

// 找章节  碰到chapter关键词就+1 ，直到lineCount行
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
}

// 加载书籍信息 同时做完查找将结果放入books中
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
            if (line.find(keyword) != string::npos) // 如果这一行包含关键词
            {
                // index = line.find(keyword); // 找到关键词的位置
                //  找页码和章节和上下文
                page = getPage(file, lineCount);
                string context = getContext(file, lineCount);
                int chapter = getChapter(file, lineCount);
                books.push_back(Book{filename, {{context, page, chapter}}}); // 添加到books中
            }
        }
        file.close();
    }
    return books;
}

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
        "./book/J.K. Rowling - Quidditch Through the Ages.txt",
        "./book/J.K. Rowling - The Tales of Beedle the Bard.txt",*/
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