#include <iostream>
#include <string>
using namespace std;

int main()
{
    string s = "hello,world";
    s.erase(0, 5);
    cout << s << endl;
}