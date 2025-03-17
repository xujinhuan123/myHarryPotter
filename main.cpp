#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <regex>
using namespace std;

struct PageInfo
{
    int page_number;
    size_t page_start; // 页起始位置（含）
    size_t page_end;   // 页结束位置（含）
};

struct Book
{
    string filename;
    string content;
    string content_lower;
    vector<size_t> line_breaks;
    vector<PageInfo> book_pages;
};

// 页码行识别函数
vector<PageInfo> findPageMarkers(const string &content)
{
    vector<PageInfo> pages;
    regex page_re(R"(^\s*\d+\s*$)");
    smatch match;
    size_t pos = 0;
    int current_page = 1;
    size_t page_start = 0;

    // 先处理第一页（可能没有页码行）
    bool found_first_marker = false;

    while (pos < content.size())
    {
        size_t line_end = content.find('\n', pos);
        if (line_end == string::npos)
            line_end = content.size();

        string line = content.substr(pos, line_end - pos);

        if (regex_match(line, match, page_re))
        {
            // 发现页码行时的处理
            int page_num = stoi(line);

            if (!found_first_marker)
            {
                // 处理第一个页码行之前的内容为第1页
                pages.push_back({current_page, 0, pos - 1});
                found_first_marker = true;
                current_page = page_num;
                page_start = line_end + 1;
            }
            else
            {
                // 结束前一页
                pages.back().page_end = pos - 1;
                // 开始新页
                pages.push_back({page_num, line_end + 1, 0});
            }
        }

        pos = line_end + 1;
    }

    // 处理最后一页
    if (pages.empty())
    {
        pages.push_back({1, 0, content.size() - 1});
    }
    else
    {
        pages.back().page_end = content.size() - 1;
    }

    return pages;
}

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

        // 预处理页码信息
        vector<PageInfo> book_pages = findPageMarkers(content);

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

        // books.push_back({filename, content, content_lower, line_breaks});
        books.push_back({filename, content, content_lower, line_breaks, book_pages});
    }
    return books;
}

// 改进的页码查找逻辑
int findPageNumber(const vector<PageInfo> &pages, size_t pos)
{
    int left = 0;
    int right = pages.size() - 1;

    while (left <= right)
    {
        int mid = left + (right - left) / 2;
        if (pos < pages[mid].page_start)
        {
            right = mid - 1;
        }
        else if (pos > pages[mid].page_end)
        {
            left = mid + 1;
        }
        else
        {
            return pages[mid].page_number;
        }
    }
    return -1;
}

struct MatchResult
{
    string filename;
    int line_number; // 章节
    size_t pos;      // 页码
    int page_number; // 页码
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
            // 查找关键词所在页码
            int page_num = findPageNumber(book.book_pages, pos);
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

            if (page_num == -1)
            {
                cerr << "WARNING: Unmatched position " << pos
                     << " in file " << book.filename << endl;
            }
            results.push_back({book.filename, line_num + 1, pos, page_num, context});
            pos += key_lower.length();
        }
    }
    return results;
}

void printResults(const vector<MatchResult> &results, string keyword)
{
    int count = 0; // 记录找到的每一个
    for (int i = 0; i < results.size(); i++)
    {
        count++;
        cout << count << "\t"
             << keyword << "\t\t"
             << (results[i].page_number != -1 ? to_string(results[i].page_number) : "N/A") << "\t"
             << results[i].line_number << "\t"
             << results[i].filename << "\n";
    }
    /*
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
            */
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
    cout << "序号\t" << "人名/地名\t" << "页码\t" << "章节\t" << "书名" << endl;
    printResults(results, keyword);

    return 0;
}