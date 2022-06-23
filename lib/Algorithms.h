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

bool cmp(pair<int, float>& a,
         pair<int, float>& b)
{
    return a.second < b.second;
}



void Sort(map<int, float>& M)
{

    // Declare vector of pairs
    vector<pair<int, float> > A;

    // Copy key-value pair from Map
    // to vector of pairs
    for (auto& it : M) {
        A.push_back(it);
    }

    // Sort using comparator function
    sort(A.begin(), A.end(), cmp);

    for (auto& it : A) {

        cout << it.first << ' '
             << it.second << endl;
    }

}

//function that sorts a coulmn of a matrix in descending order
void SortMatrix(float m[][2] , int columnm, int i){
    std::cout << m[i][0]<<" "<< m[i][1]<<" "<<m[i][2]<<'\n';


}
