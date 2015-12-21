//
//  tacsim.c
//  libtacsim
//
//  Created by Xiaming Chen on 12/18/15.
//  Copyright © 2015 Xiaming Chen. All rights reserved.
//
#include "tacsim.h"

/**
 * Allocate a 2D int array with h rows and w columns
 */
MatrixInt* allocate_matrix_int(int h, int w, int defv){
    MatrixInt *mat = malloc(sizeof(MatrixInt));
    mat->h = h;
    mat->w = w;
    
    int **arr;
    arr = (int**) malloc(h * sizeof(int*));
    for (int i = 0; i < h; i++) {
        arr[i] = (int*) malloc(w * sizeof(int));
        for (int j = 0; j < w; j++) {
            arr[i][j] = defv;
        }
    }
    
    mat->m = arr;
    return mat;
}

/**
 * Allocate a 2D REAL array with h rows and w columns
 */
MatrixReal* allocate_matrix_real(int h, int w, REAL defv){
    MatrixReal *mat = malloc(sizeof(MatrixReal));
    mat->h = h;
    mat->w = w;
    
    REAL **arr;
    arr = (REAL**) malloc(h * sizeof(REAL*));
    for (int i = 0; i < h; i++) {
        arr[i] = (REAL*) malloc(w * sizeof(REAL));
        for (int j = 0; j < w; j++) {
            arr[i][j] = defv;
        }
    }
    
    mat->m = arr;
    return mat;
}

/**
 * Allocate a 1D REAL array with l elements.
 */
VectorReal* allocate_vector_real(int l, REAL defv){
    VectorReal *vec = malloc(sizeof(VectorReal));
    vec->l = l;
    vec->v = malloc(vec->l * sizeof(REAL));
    for (int i=0; i < vec->l; i++) {
        vec->v[i] = defv;
    }
    return vec;
}

/**
 * Free struct MatrixInt
 */
void free_matrix_int(MatrixInt *mat) {
    int **arr = mat->m;
    for (int i = 0; i < mat->h; i++) {
        free(arr[i]);
    }
    free(arr);
    free(mat);
}

/**
 * Free struct MatrixReal
 */
void free_matrix_real(MatrixReal *mat) {
    REAL **arr = mat->m;
    for (int i = 0; i < mat->h; i++) {
        free(arr[i]);
        arr[i] = 0;
    }
    free(arr);
    arr = 0;
    free(mat);
}

/**
 * Free struct VectorReal
 */
void free_vector_real(VectorReal *vec) {
    free(vec->v);
    free(vec);
}

/**
 * Calculat the strength of neighboring nodes.
 */
REAL strength_node(REAL nw1, REAL nw2, REAL ew) {
    return 1.0 * nw1 * nw2 / pow(ew, 2);
}

/**
 * Calculat the strength of neighboring edges.
 */
REAL strength_edge(REAL ew1, REAL ew2, REAL nw) {
    return 1.0 * pow(nw, 2) / (ew1 * ew2);
}

/**
 * Calculat the strength coherence of two neighbor pairs.
 */
REAL strength_coherence(REAL s1, REAL s2) {
    if (s1 + s2 == 0) {
        printf("Invalid strength values: s1=%f, s2=%f\n", s1, s2);
        exit(-1);
    }
    
    return 2.0 * sqrt(s1 * s2) / (s1 + s2);
}

/**
 * Normalize a 1D array.
 */
int normalize_vector(REAL **vec, int len){
    REAL sum = 0;
    for (int i = 0; i < len; i++) {
        sum += pow((*vec)[i], 2);
    }
    REAL norm = sqrt(sum);
    for (int i = 0; i < len; i++) {
        (*vec)[i] /= norm;
    }
    return 0;
}

/**
 * Normalize a 2D array.
 */
int normalize_matrix(REAL ***mat, int m, int n){
    REAL sum = 0;
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            sum += pow((*mat)[i][j], 2);
        }
    }
    REAL norm = sqrt(sum);
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            (*mat)[i][j] /= norm;
        }
    }
    return 0;
}

/**
 * Get the number of edges from node adjacency matrix.
 */
int get_edge_count(MatrixInt **node_adjacency){
    MatrixInt *nnadj = *node_adjacency;
    int elen = 0;
    for (int i = 0; i < nnadj->h; i++)
        for (int j = 0; j < nnadj->h; j++)
            if (nnadj->m[i][j] >= 0)
                elen += 1;
    return elen;
}


/**
 * Derive the source-terminal edge neighbors for each node.
 */
MatrixInt* get_edge_adjacency(MatrixInt **node_adjacency, int elen){
    MatrixInt *nnadj = *node_adjacency;
    MatrixInt *eeadj = allocate_matrix_int(elen, elen, -1);
    
    for (int i=0; i < nnadj->h; i++) {
        for (int j = 0; j < nnadj->h; j++) {
            if (nnadj->m[j][i] >= 0) {
                for (int k = 0; k < nnadj->h; k++) {
                    if (nnadj->m[i][k] >= 0) {
                        int src = nnadj->m[j][i];
                        int dst = nnadj->m[i][k];
                        eeadj->m[src][dst] = i;
                    }
                }
            }
        }
    }
    
    return eeadj;
}

/**
 * Check if two iterations are converged.
 */
int is_converged(MatrixReal **simmat, MatrixReal **simmat_prev, REAL eps) {
    MatrixReal *sim = *simmat;
    MatrixReal *sim_prev = *simmat_prev;
    
    for (int i = 0; i < sim->h; i++) {
        for (int j = 0; j < sim->w; j++) {
            if (fabs(sim->m[i][j] - sim_prev->m[i][j]) > eps)
                return 1;
        }
    }
    return 0;
}

/**
 * Copy the similarity from one matrix to another.
 */
int copyTo(MatrixReal **simmat, MatrixReal **simmat_prev) {
    MatrixReal *sim = *simmat;
    MatrixReal *sim_prev = *simmat_prev;
    
    for (int i = 0; i < sim->h; i++) {
        for (int j = 0; j < sim->w; j++) {
            sim_prev->m[i][j] = sim->m[i][j];
        }
    }
    
    return 0;
}

/**
 * Calculate the graph elements employed by the algorithm.
 */
int graph_elements(MatrixInt *nnadj, VectorReal *node_weights, MatrixReal **nn_strength_mat,
                   MatrixInt *eeadj, VectorReal *edge_weights, MatrixReal **ee_strength_mat) {
    
    printf("Entering graph_elements\n");

    if (nnadj->h != nnadj->w || eeadj->h != eeadj->w) {
        printf("The adjacent matrix should be square.\n");
        exit(-1);
    }
    
    if (node_weights->l != nnadj->h) {
        printf("The node weight vector should be the same length as nnadj.\n");
        exit(-1);
    }
    
    if (edge_weights->l != eeadj->h) {
        printf("The edge weight vector should be the same length as eeadj.\n");
        exit(-1);
    }
    
    MatrixReal *nnsm = *nn_strength_mat;
    MatrixReal *eesm = *ee_strength_mat;
    
    for (int i = 0; i < nnadj->h; i++) {
        for (int j = 0; j < nnadj->h; j++) {
            int edge_index = nnadj->m[i][j];
            if(edge_index >= 0) {
                nnsm->m[i][j] = strength_node(node_weights->v[i], node_weights->v[j], edge_weights->v[edge_index]);
            }
        }
    }
    
    for (int i = 0; i < eeadj->h; i++) {
        for (int j = 0; j < eeadj->h; j++) {
            int node_index = eeadj->m[i][j];
            if(node_index >= 0) {
                eesm->m[i][j] = strength_edge(edge_weights->v[i], edge_weights->v[j], node_weights->v[node_index]);
            }
        }
    }
    
    printf("Exiting graph_elements\n");
    
    return 0;
}


/**
 * Set the value smaller than tolerance to zero.
 */
void mask_lower_values(MatrixReal **simmat, REAL tolerance) {
    MatrixReal *sim = *simmat;
    for (int i = 0; i < sim->h; i++){
        for (int j = 0; j < sim->w; j++){
            if ( fabs(sim->m[i][j]) < tolerance)
                sim->m[i][j] = 0;
        }
    }
}

/**
 * The algorithm to calculate the topology-attribute coupling similarity for two graphs.
 */
int tacsim(MatrixInt *g1_nnadj, MatrixInt *g1_eeadj, MatrixReal *g1_nn_strength_mat, MatrixReal *g1_ee_strength_mat,
           MatrixInt *g2_nnadj, MatrixInt *g2_eeadj, MatrixReal *g2_nn_strength_mat, MatrixReal *g2_ee_strength_mat,
           MatrixReal **nn_simmat, MatrixReal **ee_simmat, int max_iter, REAL eps, REAL tolerance) {
    
    printf("Entering tacsim\n");
    
    MatrixReal *nn_sim = *nn_simmat;
    MatrixReal *ee_sim = *ee_simmat;
    MatrixReal *nn_sim_prev = allocate_matrix_real(nn_sim->h, nn_sim->w, 0);
    MatrixReal *ee_sim_prev = allocate_matrix_real(ee_sim->h, ee_sim->w, 0);
    
    int iter = 0;
    for (; iter < max_iter; iter++) {
        
        printf("Iteration %d\n", iter);
        
        if (is_converged(&nn_sim, &nn_sim_prev, eps) == 0 &&
            is_converged(&ee_sim, &ee_sim_prev, eps) == 0)
            break;
        
        copyTo(&nn_sim, &nn_sim_prev);
        copyTo(&ee_sim, &ee_sim_prev);
        
        // Update node similarity, in and out node neighbors
        int N = nn_sim->h;
        int M = nn_sim->w;
        
        for (int i = 0; i < N; i++){
            for (int j = 0; j < M; j++) {
                // In neighbors
                for (int u = 0; u < N; u++){
                    REAL sui = g1_nn_strength_mat->m[u][i];
                    if (sui >= 0) {
                        for (int v = 0; v < M; v++){
                            REAL svj = g2_nn_strength_mat->m[v][j];
                            if (svj >= 0) {
                                int u_edge = g1_nnadj->m[u][i];
                                int v_edge = g2_nnadj->m[v][j];
                                
                                nn_sim->m[i][j] = nn_sim->m[i][j] + 0.5 \
                                * strength_coherence(sui, svj) \
                                * (nn_sim_prev->m[u][v] + ee_sim_prev->m[u_edge][v_edge]);
                            }
                        }
                    }
                }
                
                // Out neighbors
                for (int u = 0; u < N; u++){
                    REAL siu = g1_nn_strength_mat->m[i][u];
                    if (siu >= 0) {
                        for (int v = 0; v < M; v++){
                            REAL sjv = g2_nn_strength_mat->m[j][v];
                            if (sjv >= 0) {
                                int u_edge = g1_nnadj->m[i][u];
                                int v_edge = g2_nnadj->m[j][v];
                                
                                nn_sim->m[i][j] += 0.5 \
                                * strength_coherence(siu, sjv) \
                                * (nn_sim_prev->m[u][v] + ee_sim_prev->m[u_edge][v_edge]);
                            }
                        }
                    }
                }
            }
        }
        
        // Update edge similarity, in and out edge neighbors
        int P = ee_sim->h;
        int Q = ee_sim->w;
        
        for (int i = 0; i < P; i++){
            for (int j = 0; j < Q; j++) {
                // In neighbors
                for (int u = 0; u < P; u++){
                    REAL sui = g1_ee_strength_mat->m[u][i];
                    if (sui >= 0) {
                        for (int v = 0; v < Q; v++){
                            REAL svj = g2_ee_strength_mat->m[v][j];
                            if (svj >= 0) {
                                int u_node = g1_eeadj->m[u][i];
                                int v_node = g2_eeadj->m[v][j];
                                
                                ee_sim->m[i][j] += 0.5 \
                                * strength_coherence(sui, svj) \
                                * (ee_sim_prev->m[u][v] + nn_sim_prev->m[u_node][v_node]);
                            }
                        }
                    }
                }
                
                // Out neighbors
                for (int u = 0; u < P; u++){
                    REAL siu = g1_ee_strength_mat->m[i][u];
                    if (siu >= 0) {
                        for (int v = 0; v < Q; v++){
                            REAL sjv = g2_ee_strength_mat->m[j][v];
                            if (sjv >= 0) {
                                int u_node = g1_eeadj->m[i][u];
                                int v_node = g2_eeadj->m[j][v];
                                
                                ee_sim->m[i][j] += 0.5 \
                                * strength_coherence(siu, sjv) \
                                * (ee_sim_prev->m[u][v] + nn_sim_prev->m[u_node][v_node]);
                            }
                        }
                    }
                }
            }
        }
        
        // Normalize matrices before entering next iteration
        normalize_matrix(&(nn_sim->m), nn_sim->h, nn_sim->w);
        normalize_matrix(&(ee_sim->m), ee_sim->h, ee_sim->w);
        
    }
    
    mask_lower_values(&nn_sim, tolerance);
    mask_lower_values(&ee_sim, tolerance);
    
    free_matrix_real(nn_sim_prev);
    free_matrix_real(ee_sim_prev);
    
    printf("Converge after %d iterations (eps=%f).\n", iter, eps);
    
    return 0;
}

/**
 * Export interface of tacsim algorithm.
 */
int calculate_tacsim(int **A, REAL *Anw, REAL *Aew, int Anode, int Aedge,
                     int **B, REAL *Bnw, REAL *Bew, int Bnode, int Bedge,
                     REAL ***nsim, REAL ***esim,
                     int max_iter, REAL eps, REAL tol) {

    // create a new graph
    MatrixInt *graph = allocate_matrix_int(Anode, Anode, -1);
    MatrixInt *graph_eeadj;
    VectorReal *node_weights = allocate_vector_real(Anode, -1);
    VectorReal *edge_weights = allocate_vector_real(Aedge, -1);
    MatrixReal *nn_strength_mat = allocate_matrix_real(Anode, Anode, -1);
    MatrixReal *ee_strength_mat = allocate_matrix_real(Aedge, Aedge, -1);
    
    for (int i = 0; i<graph->h; i++) {
        for(int j = 0; j<graph->w; j++) {
            graph->m[i][j] = A[i][j];
        }
    }
    
    for (int i = 0; i < node_weights->l; i++)
        node_weights->v[i] = Anw[i];
    
    for (int i = 0; i < edge_weights->l; i++)
        edge_weights->v[i] = Aew[i];
    
    normalize_vector(&(node_weights->v), node_weights->l);
    normalize_vector(&(edge_weights->v), edge_weights->l);
    
    graph_eeadj = get_edge_adjacency(&graph, Aedge);
    
    graph_elements(graph, node_weights, &nn_strength_mat, graph_eeadj, edge_weights, &ee_strength_mat);
    
    free_vector_real(node_weights);
    free_vector_real(edge_weights);
    
    // create another graph
    MatrixInt *graph2 = allocate_matrix_int(Bnode, Bnode, -1);
    MatrixInt *graph_eeadj2;
    VectorReal *node_weights2 = allocate_vector_real(Bnode, -1);
    VectorReal *edge_weights2 = allocate_vector_real(Bedge, -1);
    MatrixReal *nn_strength_mat2 = allocate_matrix_real(Bnode, Bnode, -1);
    MatrixReal *ee_strength_mat2 = allocate_matrix_real(Bedge, Bedge, -1);
    
    for (int i = 0; i<graph2->h; i++) {
        for(int j = 0; j<graph2->w; j++) {
            graph2->m[i][j] = B[i][j];
        }
    }
    
    for (int i = 0; i < node_weights2->l; i++)
        node_weights2->v[i] = Bnw[i];
    
    for (int i = 0; i < edge_weights2->l; i++)
        edge_weights2->v[i] = Bew[i];
    
    normalize_vector(&(node_weights2->v), node_weights2->l);
    normalize_vector(&(edge_weights2->v), edge_weights2->l);

    graph_eeadj2 = get_edge_adjacency(&graph2, Bedge);

    graph_elements(graph2, node_weights2, &nn_strength_mat2, graph_eeadj2, edge_weights2, &ee_strength_mat2);
    
    free_vector_real(node_weights2);
    free_vector_real(edge_weights2);
    
    MatrixReal *nn_sim = allocate_matrix_real(Anode, Bnode, 1);
    MatrixReal *ee_sim = allocate_matrix_real(Aedge, Bedge, 1);
    
    tacsim(graph, graph_eeadj, nn_strength_mat, ee_strength_mat,
           graph2, graph_eeadj2, nn_strength_mat2, ee_strength_mat2,
           &nn_sim, &ee_sim, max_iter, eps, tol);
    
    free_matrix_int(graph);
    free_matrix_int(graph_eeadj);
    free_matrix_real(nn_strength_mat);
    free_matrix_real(ee_strength_mat);
    
    free_matrix_int(graph2);
    free_matrix_int(graph_eeadj2);
    free_matrix_real(nn_strength_mat2);
    free_matrix_real(ee_strength_mat2);
    
    *nsim = nn_sim->m;
    *esim = ee_sim->m;
    
    return 0;
}

/**
 * Calculate the self-similarity via TACSim algrithm.
 */
int calculate_tacsim_self(int **A, REAL *Anw, REAL *Aew, int Anode, int Aedge,
                          REAL ***nsim, REAL ***esim,
                          int max_iter, REAL eps, REAL tol) {
    
    // create a new graph
    MatrixInt *graph = allocate_matrix_int(Anode, Anode, -1);
    MatrixInt *graph_eeadj;
    VectorReal *node_weights = allocate_vector_real(Anode, -1);
    VectorReal *edge_weights = allocate_vector_real(Aedge, -1);
    MatrixReal *nn_strength_mat = allocate_matrix_real(Anode, Anode, -1);
    MatrixReal *ee_strength_mat = allocate_matrix_real(Aedge, Aedge, -1);
    
    for (int i = 0; i<graph->h; i++) {
        for(int j = 0; j<graph->w; j++) {
            graph->m[i][j] = A[i][j];
        }
    }
    
    for (int i = 0; i < node_weights->l; i++)
        node_weights->v[i] = Anw[i];
    
    for (int i = 0; i < edge_weights->l; i++)
        edge_weights->v[i] = Aew[i];
    
    normalize_vector(&(node_weights->v), node_weights->l);
    normalize_vector(&(edge_weights->v), edge_weights->l);
    
    graph_eeadj = get_edge_adjacency(&graph, Aedge);
    
    graph_elements(graph, node_weights, &nn_strength_mat, graph_eeadj, edge_weights, &ee_strength_mat);
    
    free_vector_real(node_weights);
    free_vector_real(edge_weights);
    
    MatrixReal *nn_sim = allocate_matrix_real(Anode, Anode, 1);
    MatrixReal *ee_sim = allocate_matrix_real(Aedge, Aedge, 1);
    
    tacsim(graph, graph_eeadj, nn_strength_mat, ee_strength_mat,
           graph, graph_eeadj, nn_strength_mat, ee_strength_mat,
           &nn_sim, &ee_sim, max_iter, eps, tol);
    
    free_matrix_int(graph);
    free_matrix_int(graph_eeadj);
    free_matrix_real(nn_strength_mat);
    free_matrix_real(ee_strength_mat);
    
    *nsim = nn_sim->m;
    *esim = ee_sim->m;
    
    return 0;
}
