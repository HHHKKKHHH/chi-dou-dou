#include <cstdio>
#include <string.h>
#include <string>
#include <iostream>
using namespace std;

#define maxn 100
#define inf 0x3f3f3f3f

int array[maxn][maxn];
int DP[maxn][maxn][maxn];

int main()
{
    int n;
    cin >> n;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            cin >> array[i][j];
        }
    }
    memset(DP, 0, sizeof(DP));
    int res = -inf; //负无穷初始化，保证可能值都比其=大
    //i是指行，j是左起始列，k是右结束列，当前值为在ijk范围内的元素和最大值
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            int sum = 0;
            for (int k = j; k < n; k++)
            {
                sum += array[i][k];
                //比较是接着上一行进行扩展还是舍弃
                if (i > 0)
                {
                    DP[i][j][k] = max(DP[i - 1][j][k] + sum, sum);
                }
                else
                {
                    DP[i][j][k] = sum;
                }
                //取出现过的最大值
                res = max(res, DP[i][j][k]);
            }
        }
    }
    cout << res << endl;
    return 0;
}
