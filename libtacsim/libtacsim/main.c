
#include "tacsim.h"


int main(int argc, const char * argv[]) {
    
    int A[4][4] = {{-1,-1,0,-1}, {1,-1,2,-1}, {-1,-1,-1,3}, {-1,-1,-1,-1}};
    int Anw[4] = {1,1,5,1};
    int Aew[4] = {12,8,10,15};
    int Anode = 4;
    int Aedge = 4;
    
    int B[3][3] = {{-1,0,-1}, {-1,-1,1}, {-1,-1,-1}};
    int Bnw[3] = {1,3,1};
    int Bew[2] = {15,10};
    int Bnode = 3;
    int Bedge = 2;
    
    MatrixInt *graph = allocate_matrix_int(Anode, Aedge, -1);
    VectorReal *node_weights = allocate_vector_real(Anode, -1);
    VectorReal *edge_weights = allocate_vector_real(Aedge, -1);
    
    for (int i = 0; i<graph->h; i++) {
        for(int j = 0; j<graph->w; j++) {
            graph->m[i][j] = A[i][j];
        }
    }
    
    for (int i = 0; i < node_weights->l; i++)
        node_weights->v[i] = Anw[i];
    
    for (int i = 0; i < edge_weights->l; i++)
        edge_weights->v[i] = Aew[i];
    
    MatrixInt *graph2 = allocate_matrix_int(Bnode, Bnode, -1);
    VectorReal *node_weights2 = allocate_vector_real(Bnode, -1);
    VectorReal *edge_weights2 = allocate_vector_real(Bedge, -1);
    
    for (int i = 0; i<graph2->h; i++) {
        for(int j = 0; j<graph2->w; j++) {
            graph2->m[i][j] = B[i][j];
        }
    }
    
    for (int i = 0; i < node_weights2->l; i++)
        node_weights2->v[i] = Bnw[i];
    
    for (int i = 0; i < edge_weights2->l; i++)
        edge_weights2->v[i] = Bew[i];
    
    REAL **nsim;
    REAL **esim;
    
    // Similarity of two graphs
    calculate_tacsim(graph->m, node_weights->v, edge_weights->v, Anode, Aedge,
                     graph2->m, node_weights2->v, edge_weights2->v, Bnode, Bedge,
                     &nsim, &esim, 100, 1e-4, 1e-6);
    
    printf("NN Sim\n");
    for (int i = 0; i < Anode; i++) {
        for(int j = 0; j < Bnode; j++) {
            printf("%d,%d,%f\n", i, j, nsim[i][j]);
        }
    }
    printf("\n");
    
    printf("EE Sim\n");
    for (int i = 0; i < Aedge; i++) {
        for(int j = 0; j < Bedge; j++) {
            printf("%d,%d,%f\n", i, j, esim[i][j]);
        }
    }
    printf("\n");
    
    // Self-similarity of a graph
    calculate_tacsim_self(graph->m, node_weights->v, edge_weights->v, Anode, Aedge,
                          &nsim, &esim, 100, 1e-4, 1e-6);
    
    printf("NN SelfSim\n");
    for (int i = 0; i < Anode; i++) {
        for(int j = 0; j < Anode; j++) {
            printf("%d,%d,%f\n", i, j, nsim[i][j]);
        }
    }
    printf("\n");
    
    printf("EE SelfSim\n");
    for (int i = 0; i < Aedge; i++) {
        for(int j = 0; j < Aedge; j++) {
            printf("%d,%d,%f\n", i, j, esim[i][j]);
        }
    }
    printf("\n");
    
    return 0;
}