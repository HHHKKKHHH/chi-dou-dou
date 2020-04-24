#include <cmath>
#include <iostream>
#include <algorithm>
#include <cstdio>
#include <cstring>

using namespace std;

#define MAXN 1010 //岛屿数的最大值

//岛屿
typedef struct island
{
    int x, y;
} island;

//岛屿可行域
typedef struct
{
    double l, r;
} range;

int n;                //岛屿数
int d;                //雷达范围
island islands[MAXN]; //岛屿数组
range ranges[MAXN];

int cmp(range a, range b)
{
    return a.l < b.l;
}

//计算岛屿可行域，如不可行返回false
bool calcRange(island i, range &r, int d)
{
    if (i.y > d || i.x < 0)
    {
        return false;
    }
    r.l = i.x - sqrt(d * d * 1.0 - i.y * i.y * 1.0);
    r.r = i.x + sqrt(d * d * 1.0 - i.y * i.y * 1.0);
    return true;
}

int main()
{
    int count = 0;
    while (cin >> n)
    {
        count++;
        cin >> d;
        if (n == 0 && d == 0)
            break;
        //不合法输入直接返回 -1
        if (d < 0)
        {
            cout << "Case " << count << ": " << -1 << endl;
            continue;
        }
        //每次清空岛屿数组
        memset(islands, 0, sizeof(islands));

        bool hasSolution = true;
        //输入岛屿并计算每个岛屿的可行域
        for (int i = 0; i < n; i++)
        {
            cin >> islands[i].x;
            cin >> islands[i].y;
            hasSolution = calcRange(islands[i], ranges[i], d);
        }
        //如果有不可达的岛屿，直接返回-1
        if (!hasSolution)
        {
            cout << "Case " << count << ": " << -1 << endl;
            continue;
        }
        sort(ranges, ranges + n, cmp);
        int res = 1;
        //从第二个岛屿开始遍历，只有一个岛屿必然是需要一个雷达
        range tmp = ranges[0];
        for (int i = 1; i < n; i++)
        {
            if (ranges[i].l > tmp.r)
            {
                res++;
                tmp = ranges[i];
            }
            else if (ranges[i].r < tmp.r)
            {
                tmp = ranges[i];
            }
        }
        cout << "Case " << count << ": " << res << endl;
    }
    return 0;
}