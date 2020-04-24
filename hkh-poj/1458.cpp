#include <iostream>
#include <cstring>

using namespace std;

char string1[5005];
char string2[5005];
int maxLen[5005][5005];
int main()
{
    while (cin >> string1 >> string2)
    {
        int length1 = strlen(string1);
        int length2 = strlen(string2);
        int nTmp;
        int i, j;
        for (i = 0; i <= length1; i++)
            maxLen[i][0] = 0;
        for (j = 0; j <= length2; j++)
            maxLen[0][j] = 0;
        for (i = 1; i <= length1; i++)
        {
            for (j = 1; j <= length2; j++)
            {
                if (string1[i - 1] == string2[j - 1])
                    maxLen[i][j] = maxLen[i - 1][j - 1] + 1;
                else
                    maxLen[i][j] = max(maxLen[i][j - 1], maxLen[i - 1][j]);
            }
        }
        cout << maxLen[length1][length2] << endl;
    }
    return 0;
}