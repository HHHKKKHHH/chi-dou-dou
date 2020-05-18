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
    int res = -inf; //�������ʼ������֤����ֵ������=��
    //i��ָ�У�j������ʼ�У�k���ҽ����У���ǰֵΪ��ijk��Χ�ڵ�Ԫ�غ����ֵ
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            int sum = 0;
            for (int k = j; k < n; k++)
            {
                sum += array[i][k];
                //�Ƚ��ǽ�����һ�н�����չ��������
                if (i > 0)
                {
                    DP[i][j][k] = max(DP[i - 1][j][k] + sum, sum);
                }
                else
                {
                    DP[i][j][k] = sum;
                }
                //ȡ���ֹ������ֵ
                res = max(res, DP[i][j][k]);
            }
        }
    }
    cout << res << endl;
    return 0;
}
