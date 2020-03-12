#include <iostream>
#include <cstdio>
#include <iomanip>
#include <cstring>
#include <algorithm>
#include <cmath>
using namespace std;

;

const long long INF = 1000000000000;
const int N = 100010;//��100000�Դ󣬱�֤�ȶ���

struct Node
{
    long long x, y;//��������
    int id;//�����Ϣ�������ж��㼯
    Node(long long x = 0, long long y = 0, int id = 0) : x(x), y(y), id(id) {}
    const bool operator<(const Node A) const //���رȽϷ�����ֱ�ӵ���sort����
    {
        return x == A.x ? y < A.y : x < A.x;
    }
} node[2 * N];//���Դ������е�

int n;
//�����������
double dis(int a, int b)
{
    return sqrt((double)((node[a].x - node[b].x) * (node[a].x - node[b].x) + (node[a].y - node[b].y) * (node[a].y - node[b].y)));
}
//�ݹ���ú���
double solve(int l, int r)
{
    //����ݹ鵽ֻ��һ���㣬���������
    if (l == r)
        return INF;
    //λ����ȡ�е�
    int mid = (l + r) >> 1;
    double a = solve(l, mid);
    double b = solve(mid + 1, r);
    //���²�ݹ鷵�ص���С����
    double d = min(a, b);
    //�鲢���ߵĵݹ���
    for (int i = mid; i >= l; --i)
    {
        //�����ߵ�ǰ���Ѿ���mid��������²�ݹ�����break
        if (node[mid].x - node[i].x > d)
            break;
        for (int j = mid + 1; j <= r; ++j)
        {
             //����ұߵ�ǰ���Ѿ�����ߵ�ǰ���������²�ݹ�����break
            if (node[j].x - node[i].x > d)
                break;
            double tmp = dis(i, j);
            //�Ƚ�����ͬ�㼯����
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
        //��֪��Ϊɶһ��Ҫ����ȡ�Ŷ�
        printf("%.3lf\n", res);
        //cout<<fixed<<setprecision(2)<<res;
    }
}