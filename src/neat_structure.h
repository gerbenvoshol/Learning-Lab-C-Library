/*
MIT License

Copyright (c) 2018 Viviano Riccardo

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files ((the "LICENSE")), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __NEAT_STRUCTURE_H__
#define __NEAT_STRUCTURE_H__

#include "genome.h"
#include "species.h"

    
typedef struct neat{
    int i,j,z,k,w,flag,min,max,total_species,count;
    int global_inn_numb_connections,global_inn_numb_nodes, actual_genomes, n_survivors,temp_gg2_counter,temp_gg3_counter, n_species;
    int* dict_connections;
    int** matrix_nodes;
    int** matrix_connections;
    species* s;
    genome* g;
    genome** gg;
    genome** temp_gg1;
    genome** temp_gg2;
    genome** temp_gg3;
    float a,b,n,sum;
}neat;

#endif