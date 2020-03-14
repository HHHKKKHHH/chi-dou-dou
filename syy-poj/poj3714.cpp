#include<iostream>
#include<cstdio>
#include<cmath>
#include<algorithm>
using namespace std;
const double MD=1e100;
const int MAXN=200005;
int T,n,ys[MAXN];
struct P{
    double x, y;
    bool flag;
}a[MAXN];
double dist(const P&a,const P&b) {
    if(a.flag!=b.flag) return sqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y));
    return MD;
}
bool cmp(const P &a,const P&b){
    return a.x<b.x;
}
bool y_cmp(const int&u,const int&v) {
    return a[u].y<a[v].y;
}
double calc(int l,int r){
    if(r-l==1)return dist(a[l], a[r]);
    else if(r-l==2) return min(min(dist(a[l], a[l+1]), dist(a[l], a[l+2])),dist(a[l+1], a[l+2]));
    int mid=(l+r)/2,yn=0;
    double ans=min(calc(l,mid),calc(mid+1, r));
    if(ans==0) return 0;
    for(int i=mid;a[mid].x-a[i].x<ans&&i>=l;i--) ys[yn++]=i;
    int y_mid=yn;
    for(int i=mid+1;a[i].x-a[mid].x<ans&&i<=r;i++) ys[yn++]=i;
    for(int i=0;i<y_mid;i++)
        for(int j=y_mid;j<yn;++j)
            ans=min(ans,dist(a[ys[i]],a[ys[j]]));
    return ans;
}
int main()
{
    cin>>T;
    while(T--)
    {
        scanf("%d",&n);
        for(int i=0;i<n;i++) {
            scanf("%lf%lf",&a[i].x,&a[i].y);
            a[i].flag=false;
        }
        for(int i=n;i<2*n;i++) {
            scanf("%lf%lf",&a[i].x, &a[i].y);
            a[i].flag=true;
        }
        sort(a,a+2*n,cmp);
        printf("%.3f\n", calc(0,2*n-1));
    }
    return 0;
}
