#include <cmath>
#include <iostream>
#include <algorithm>
#include <cstdio>
#include <cstring>

using namespace std;

#define MAXN 1010 //�����������ֵ

//����
typedef struct island
{
    int x, y;
} island;

//���������
typedef struct
{
    double l, r;
} range;

int n;                //������
int d;                //�״ﷶΧ
island islands[MAXN]; //��������
range ranges[MAXN];

int cmp(range a, range b)
{
    return a.l < b.l;
}

//���㵺��������粻���з���false
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
        //���Ϸ�����ֱ�ӷ��� -1
        if (d < 0)
        {
            cout << "Case " << count << ": " << -1 << endl;
            continue;
        }
        //ÿ����յ�������
        memset(islands, 0, sizeof(islands));

        bool hasSolution = true;
        //���뵺�첢����ÿ������Ŀ�����
        for (int i = 0; i < n; i++)
        {
            cin >> islands[i].x;
            cin >> islands[i].y;
            hasSolution = calcRange(islands[i], ranges[i], d);
        }
        //����в��ɴ�ĵ��죬ֱ�ӷ���-1
        if (!hasSolution)
        {
            cout << "Case " << count << ": " << -1 << endl;
            continue;
        }
        sort(ranges, ranges + n, cmp);
        int res = 1;
        //�ӵڶ������쿪ʼ������ֻ��һ�������Ȼ����Ҫһ���״�
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