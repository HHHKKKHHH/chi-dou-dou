#include <iostream>

using namespace std;
#define max(a, b) (((a) > (b)) ? (a) : (b))
int row = 0;        //行数
int num[50000] = {}; //储存数组
int res[50000] = {}; //DP结果数组

int main()
{
    cin >> row;
    int max = row * (row + 1) / 2 - 1; //最大下标;
    for (int i = 0; i <= max; i++)
    {
        cin >> num[i];
    }
    for (int i = row; i > 0; i--)
    {
        for (int index = 0; index < i; index++)
        {
            int loc = i * (i - 1) / 2 + index ; //当前下标
            if (loc + row > max)                   //如果没有下一行，最优解为当前值
            {
                res[loc] = num[loc];
            }
            else
            {
                //递推公式，当前最优解 = 下层两可选最优解最大值 + 当前值
                res[loc] = max(res[loc + i], res[loc + i + 1]) + num[loc];
            }
        }
    }
    cout << res[0];
    return 0;
}