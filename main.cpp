#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

struct Book
{
    string filename;
    string content;
    string content_lower;
    vector<size_t> line_breaks;
};

vector<Book> loadBooks(vector<string> &filenames)
{
    vector<Book> books;
    for (auto &filename : filenames)
    {
        ifstream file(filename);
        if (!file)
        {
            cerr << "Error opening: " << filename << endl;
            continue;
        }
        string content((istreambuf_iterator<char>(file)), {}); // 一个文件的内容
        file.close();

        string content_lower = content;
        transform(content_lower.begin(), content_lower.end(), content_lower.begin(), ::tolower); // 转换为小写

        vector<size_t> line_breaks;
        size_t last_pos = 0;
        for (size_t i = 0; i < content.size(); ++i)
        {
            if (content[i] == '\n')
            {
                line_breaks.push_back(i);
                last_pos = i + 1;
            }
        }
        if (!content.empty() && content.back() != '\n')
        {
            line_breaks.push_back(content.size());
        }
        filename.erase(0, 7); // 删除路径

        books.push_back({filename, content, content_lower, line_breaks});
    }
    return books;
}

struct MatchResult
{
    string filename;
    int line_number;
    size_t pos;
    vector<string> context;
};

vector<MatchResult> searchBooks(const vector<Book> &books, const string &keyword)
{
    vector<MatchResult> results;
    string key_lower = keyword;
    transform(key_lower.begin(), key_lower.end(), key_lower.begin(), ::tolower);
    const size_t key_len = key_lower.length();

    for (const auto &book : books)
    {
        size_t pos = 0;
        while ((pos = book.content_lower.find(key_lower, pos)) != string::npos)
        {
            // 确定行号
            auto line_it = upper_bound(book.line_breaks.begin(), book.line_breaks.end(), pos);
            int line_num = line_it - book.line_breaks.begin();

            // 获取上下文（前面一行+当前行）
            int start_line = max(0, line_num - 1);
            int end_line = min((int)book.line_breaks.size() - 1, line_num);
            vector<string> context;
            for (int i = start_line; i <= end_line; ++i)
            {
                size_t line_start = (i == 0) ? 0 : book.line_breaks[i - 1] + 1;
                size_t line_end = book.line_breaks[i];
                string line = book.content.substr(line_start, line_end - line_start);
                if (!line.empty() && line.back() == '\n')
                    line.pop_back();
                context.push_back(line);
            }

            results.push_back({book.filename, line_num + 1, pos, context});
            pos += key_len; // 移动位置继续搜索
        }
    }
    return results;
}

void printResults(const vector<MatchResult> &results)
{
    for (const auto &res : results)
    {
        cout << "File: " << res.filename << "\n"
             << "Line: " << res.line_number << "\n"
             << "Position: " << res.pos << "\n"
             << "Context:\n";
        for (const auto &line : res.context)
        {
            cout << "  " << line << "\n";
        }
        cout << "--------------------\n";
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
    auto books = loadBooks(filenames);

    string keyword;
    cout << "Enter keyword: ";
    getline(cin, keyword);

    auto results = searchBooks(books, keyword);
    printResults(results);

    return 0;
}