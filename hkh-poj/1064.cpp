#include <string>
#include <iomanip>
#include <iostream>
#include <cmath>
using namespace std;

const int maxN = 10000;//n最大值
const int maxK = 10000;//k最大值

int n = 0;
int k = 0;

double arr[10000] ; //储存所有网线


bool isSatisfied(double num){
    int total =0;
    for(int i = 0; i <n;i++){
        total+=arr[i]/num;
    }
    return total>=k;
}

int main(){
    cin>>n>>k;
    int i = 0;
    double maxLength = 0;
    while(n>i){
        cin>>arr[i];
         maxLength = arr[i]>maxLength?arr[i]:maxLength;
        i++;
       
    }
    double l = 0;double r = maxLength;
    for(int cut = 0;cut<100;cut++){
        double m = (l+r)/2.0;
        if(isSatisfied(m)){
            l = m;
        }
        else{
            r= m;
        }
    if(r-l<0.001){
        break;
    }
    }
    cout<<fixed<<setprecision(2)<<floor(r*100)/100;
    return 0;
}

