#include <iostream>
#include <cstdio>
#include <iomanip>
#include <cstring>
#include <algorithm>
#include <cmath>
using namespace std;

;

const long long INF = 1000000000000;
const int N = 100010;//比100000略大，保证稳定性

struct Node
{
    long long x, y;//储存坐标
    int id;//标记信息，用于判定点集
    Node(long long x = 0, long long y = 0, int id = 0) : x(x), y(y), id(id) {}
    const bool operator<(const Node A) const //重载比较符用于直接调用sort函数
    {
        return x == A.x ? y < A.y : x < A.x;
    }
} node[2 * N];//用以储存所有点

int n;
//计算两点距离
double dis(int a, int b)
{
    return sqrt((double)((node[a].x - node[b].x) * (node[a].x - node[b].x) + (node[a].y - node[b].y) * (node[a].y - node[b].y)));
}
//递归调用函数
double solve(int l, int r)
{
    //如果递归到只有一个点，返回无穷大
    if (l == r)
        return INF;
    //位运算取中点
    int mid = (l + r) >> 1;
    double a = solve(l, mid);
    double b = solve(mid + 1, r);
    //求下层递归返回的最小距离
    double d = min(a, b);
    //归并两边的递归结果
    for (int i = mid; i >= l; --i)
    {
        //如果左边当前点已经和mid距离大于下层递归结果，break
        if (node[mid].x - node[i].x > d)
            break;
        for (int j = mid + 1; j <= r; ++j)
        {
             //如果右边当前点已经和左边当前点距离大于下层递归结果，break
            if (node[j].x - node[i].x > d)
                break;
            double tmp = dis(i, j);
            //比较两不同点集距离
            if (node[i].id != node[j].id && tmp < d)
                d = tmp;
        }
    }
    return d;
}

int main()
{
    int time;
    cin>>time;
    while (time--)
    {
        
        cin>>n;
        for (int i = 0; i < n; ++i)
        {
            cin>>node[i].x>>node[i].y;
            node[i].id = 1;
        }
        for (int i = 0; i < n; ++i)
        {
            cin>>node[i + n].x>>node[i + n].y;
            node[i + n].id = 2;
        }
        sort(node, node + 2 * n);
        double res = solve(0, 2 * n - 1);
        //不知道为啥一定要这样取才对
        printf("%.3lf\n", res);
        //cout<<fixed<<setprecision(2)<<res;
    }
}