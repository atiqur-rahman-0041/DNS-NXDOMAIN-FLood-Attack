#include "./queryBuilder.hpp"

unordered_map<string, string> umap;

void initMap(){
    umap["0000"] = '0';
    umap["0001"] = '1';
    umap["0010"] = '2';
    umap["0011"] = '3';
    umap["0100"] = '4';
    umap["0101"] = '5';
    umap["0110"] = '6';
    umap["0111"] = '7';
    umap["1000"] = '8';
    umap["1001"] = '9';
    umap["1010"] = 'A';
    umap["1011"] = 'B';
    umap["1100"] = 'C';
    umap["1101"] = 'D';
    umap["1110"] = 'E';
    umap["1111"] = 'F';
}

int main(){
    srand(time(NULL));
    initMap();
    return 0;
}