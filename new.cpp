#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
using namespace std;

struct Book
{
    string name;
    vector<int> pages;
};

// 检查字符串是否只包含数字  获取页码
bool is_only_digit(const std::string &str)
{
    return std::all_of(str.begin(), str.end(), ::isdigit);
}

vector<Book> loadBooks(vector<string> &filenames, string keyword)
{
    vector<Book> books;

    bool isFind = false; // 标记是否找到关键词 找到关键词以后才需要找页码
    for (auto filename : filenames)
    {
        ifstream file(filename);          // 打开每一个文件
        string line;                      // 记录每一行的字符串
        unsigned long long lineCount = 0; // 记录在找第几行
        while (getline(file, line))       // 读取每一行
        {
            lineCount++;
            if (line.find(keyword) != string::npos) // 如果这一行包含关键词
            {
                // 找页码和章节
            }
        }
    }
    return books;
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

    cout << "序号\t" << "人名/地名\t" << "页码\t" << "章节\t" << "书名" << endl;

    return 0;
}