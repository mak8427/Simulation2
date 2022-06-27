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

auto Array1(float logic[][3], int q, float j[3]){
    for (int i = 0; i < 3; i++) {
        j[i] = logic[q][i];
    }
    //cout<<j[0]<<" "<<j[1]<<" "<<j[2]<<"\n";
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
void SortMatrix(float m[][3] , int columnm){
    float f[3];
    float x[3];
    int index=0;
    for (int l=0; l<sizeof(m);l++) {
        Array1(m, l, f);
        for (int i = l; i < sizeof(m); i++) {
            Array1(m, i + 1, x);
            if (x[columnm] > f[columnm]) {
                index= i+1;
            }
        }
        for (int j = 0; j < 3; j++) {
            swap(m[l][j], m[index][j]);
        }
        index=l;
    }
    for( int x=0; x<sizeof(m)+1; x++) {
        std::cout << m[x][0] << "  " << m[x][1] << "  " << m[x][2]<<'\n';
    }


}
