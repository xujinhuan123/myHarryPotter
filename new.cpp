#include <iostream>
#include <string>
#include <vector>
using namespace std;

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
        "./book/J.K.Rowling - HP 0 - Harry Potter Prequel.txt"};
    string keyword;
    cout << "Enter keyword: ";
    cin >> keyword;

    cout << "序号\t" << "人名/地名\t" << "页码\t" << "章节\t" << "书名" << endl;

    return 0;
}