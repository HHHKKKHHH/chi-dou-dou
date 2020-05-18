#include <iostream>
#define max(a, b) (((a) > (b)) ? (a) : (b))
using namespace std;
int a[1010];
int dp[1010] = {};
int main()
{

    int N = 0; // 数据数
    cin >> N;
    int res = 0; //结果数据
    for (int i = 0; i < N; i++)
    {
        cin >> a[i];
    }
    for (int i = 0; i < N; i++)
    {
        //初始化当前长度为1 
        dp[i] = 1;
        //遍历当前数的前面所有数，每次判断是否可以将当前值插入其头部
        for (int j = 0; j < i; j++)
        {
            if (a[i] > a[j])//如果可以插入
            {
                //当前值权衡是否要插入，插入的话便是 j 项中子序列最后再加一个 i 项
                dp[i] = max(dp[i], dp[j] + 1);
            }
        }
    }
    for (int i = 0; i < N; i++)
    {
        if (dp[i] > res)
        {
            res = dp[i];
        }
    }
    cout << res << endl;

    return 0;
}
