#include<iostream>
#include<cmath>
#include<cstdio>
using namespace std;
int n,k;
double f[10005];
bool p(double t)
{
    int sum=0;
    for(int i=1;i<=n;i++)
        sum+=(int)(f[i]/t);
    return sum>=k;
}
int main()
{
    cin>>n>>k;
    for(int i=1;i<=n;i++) scanf("%lf",&f[i]);
    double l=0,r=100000,mid=0;
    while(r-l>1e-5)
    {
        mid=(l+r)/2;
        if(p(mid)) l=mid;
        else r=mid;
    }
    printf("%.2f\n",floor(r*100)/100);
    return 0;
}
