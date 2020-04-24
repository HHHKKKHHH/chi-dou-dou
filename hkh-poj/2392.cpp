#include <iostream>
#include <algorithm>
#include <cstdio>
#include <cstring>
using namespace std;
#define MAXV 410    //石头种类的最大值
#define MAXM 40100 //代表能到达的最大高度（也是DP数组的大小）

typedef struct
{
    int height, //单个石头的高度
    maxH, //最高高度
    count;//总共有几个
} Blocks;

Blocks v[MAXV];

int cmp(Blocks x, Blocks y)
{
    return x.maxH < y.maxH;
}


int dp[MAXM]; //用作DP储存数组
int user[MAXM];//标记此高度某种石头已经用的数量

int main()
{
    int i, typeNum, j, Max;
    while (cin>>typeNum)
    {
        for (i = 1; i <= typeNum; i++)
        {
            cin>>v[i].height;
            cin>>v[i].maxH;
            cin>>v[i].count;
        }
        //最石头种类按可达最大高度升序排序，可达高度小的必须先进
        sort(v + 1, v + typeNum + 1, cmp);
        memset(dp, 0, sizeof(dp));
        dp[0] = 1;
        Max = 0; //赋值为0，高度可能为零
        //从可达高度最小的开始遍历
        for (i = 1; i <= typeNum; i++)
        {
            //每次重新初始化已使用数记录数组（因为石头换了一种）
            memset(user, 0, sizeof(user));
            for (j = v[i].height; j <= v[i].maxH; j++)
            {
                if (!dp[j] //如果此高度还未达到最优解
                && dp[j - v[i].height] //且此石头能放上去（即减去此石头高度已经有石头垫着了）
                && user[j - v[i].height] + 1 <= v[i].count)//且石头还没用完
                {
                    dp[j] = 1;//标记此高度已经确定最优解
                    user[j] = user[j - v[i].height] + 1;//标记到这个高度用了多少这种石头
                    if (j > Max)
                        Max = j;
                }
            }
        }
        cout<<Max<<endl;
    }
    return 0;
}