#include <string>
#include <iostream>
#include <cmath>
using namespace std;

const int maxN = 10000;//n最大值
const int maxK = 10000;//k最大值

int n = 0;
int k = 0;

double* arr ; //储存所有棒子


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
    while(cin>>arr[i]){
        i++;
    }
    
}

