#include <iostream>
#define max(a, b) (((a) > (b)) ? (a) : (b))
using namespace std;
int a[1010];
int dp[1010] = {};
int main()
{

    int N = 0; // ������
    cin >> N;
    int res = 0; //�������
    for (int i = 0; i < N; i++)
    {
        cin >> a[i];
    }
    for (int i = 0; i < N; i++)
    {
        //��ʼ����ǰ����Ϊ1 
        dp[i] = 1;
        //������ǰ����ǰ����������ÿ���ж��Ƿ���Խ���ǰֵ������ͷ��
        for (int j = 0; j < i; j++)
        {
            if (a[i] > a[j])//������Բ���
            {
                //��ǰֵȨ���Ƿ�Ҫ���룬����Ļ����� j ��������������ټ�һ�� i ��
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
