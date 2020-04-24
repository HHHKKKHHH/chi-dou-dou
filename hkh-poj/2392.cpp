#include <iostream>
#include <algorithm>
#include <cstdio>
#include <cstring>
using namespace std;
#define MAXV 410    //ʯͷ��������ֵ
#define MAXM 40100 //�����ܵ�������߶ȣ�Ҳ��DP����Ĵ�С��

typedef struct
{
    int height, //����ʯͷ�ĸ߶�
    maxH, //��߸߶�
    count;//�ܹ��м���
} Blocks;

Blocks v[MAXV];

int cmp(Blocks x, Blocks y)
{
    return x.maxH < y.maxH;
}


int dp[MAXM]; //����DP��������
int user[MAXM];//��Ǵ˸߶�ĳ��ʯͷ�Ѿ��õ�����

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
        //��ʯͷ���ఴ�ɴ����߶��������򣬿ɴ�߶�С�ı����Ƚ�
        sort(v + 1, v + typeNum + 1, cmp);
        memset(dp, 0, sizeof(dp));
        dp[0] = 1;
        Max = 0; //��ֵΪ0���߶ȿ���Ϊ��
        //�ӿɴ�߶���С�Ŀ�ʼ����
        for (i = 1; i <= typeNum; i++)
        {
            //ÿ�����³�ʼ����ʹ������¼���飨��Ϊʯͷ����һ�֣�
            memset(user, 0, sizeof(user));
            for (j = v[i].height; j <= v[i].maxH; j++)
            {
                if (!dp[j] //����˸߶Ȼ�δ�ﵽ���Ž�
                && dp[j - v[i].height] //�Ҵ�ʯͷ�ܷ���ȥ������ȥ��ʯͷ�߶��Ѿ���ʯͷ�����ˣ�
                && user[j - v[i].height] + 1 <= v[i].count)//��ʯͷ��û����
                {
                    dp[j] = 1;//��Ǵ˸߶��Ѿ�ȷ�����Ž�
                    user[j] = user[j - v[i].height] + 1;//��ǵ�����߶����˶�������ʯͷ
                    if (j > Max)
                        Max = j;
                }
            }
        }
        cout<<Max<<endl;
    }
    return 0;
}