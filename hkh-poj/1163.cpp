#include <iostream>

using namespace std;
#define max(a, b) (((a) > (b)) ? (a) : (b))
int row = 0;        //����
int num[50000] = {}; //��������
int res[50000] = {}; //DP�������

int main()
{
    cin >> row;
    int max = row * (row + 1) / 2 - 1; //����±�;
    for (int i = 0; i <= max; i++)
    {
        cin >> num[i];
    }
    for (int i = row; i > 0; i--)
    {
        for (int index = 0; index < i; index++)
        {
            int loc = i * (i - 1) / 2 + index ; //��ǰ�±�
            if (loc + row > max)                   //���û����һ�У����Ž�Ϊ��ǰֵ
            {
                res[loc] = num[loc];
            }
            else
            {
                //���ƹ�ʽ����ǰ���Ž� = �²�����ѡ���Ž����ֵ + ��ǰֵ
                res[loc] = max(res[loc + i], res[loc + i + 1]) + num[loc];
            }
        }
    }
    cout << res[0];
    return 0;
}