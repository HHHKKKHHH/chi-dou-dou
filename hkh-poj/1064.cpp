#include <string>
#include <iostream>
#include <cmath>
using namespace std;

const int maxN = 10000;//n���ֵ
const int maxK = 10000;//k���ֵ

int n = 0;
int k = 0;

double* arr ; //�������а���


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

