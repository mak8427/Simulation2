//p
// Created by dadoi on 21/06/2022.
//
#include<iostream>
#include<vector>
#include<map>
#include "../Agents//Pop.h"

class Pop;

using namespace std;

int Partition(vector<float> &v, int start, int end){

    int pivot = end;
    int j = start;
    for(int i=start;i<end;++i){
        if(v[i]>v[pivot]){
            swap(v[i],v[j]);
            ++j;
        }
    }
    swap(v[j],v[pivot]);
    return j;

}

void Quicksort(vector<float> &v, int start, int end ){

    if(start<end){
        int p = Partition(v,start,end);
        Quicksort(v,start,p-1);
        Quicksort(v,p+1,end);
    }

}
void PrintVector(vector<float> v){
    for(int i=0;i<v.size();++i)
        cout<<v[i]<<" ";
    cout<<"\n\n";
}

bool cmp(pair<Pop, float>& a,
         pair<Pop, float>& b)
{
    return a.second < b.second;
}



void Sort(map<Pop, float>& M)
{

    // Declare vector of pairs
    vector<pair<Pop, float> > A;

    // Copy key-value pair from Map
    // to vector of pairs
    for (auto& it : M) {
        A.push_back(it);
    }

    // Sort using comparator function
    sort(A.begin(), A.end(), cmp);

}
