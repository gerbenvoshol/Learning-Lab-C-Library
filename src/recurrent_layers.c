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

#include "llab.h"

/* This function returns a lstm structure, with the big difference with the commond long short term memory
 * standard structure, that each weight array is size*size dimensions (you can pad)
 * 
 * Inputs:
 * 
 *             @ int size:= the size of each hidden layer inside the lstm structure
 *             @ int dropout_flag1:= the dropout flag for the y output of the cell
 *             @ float dropout_threshold1:= the dropout_threshold for the y output
 *             @ int dropout_flag2:= the dropout flag for the hidden output to the next cell
 *             @ float dropout_threshold2:= the dropout threshold for the hidden state of the cell
 *             @ int layer:= the vertical layer of the lstm cell
 *                @ int window:= the number of unrolled cell in orizontal
 * 
 * */
lstm* recurrent_lstm(int size, int dropout_flag1, float dropout_threshold1, int dropout_flag2, float dropout_threshold2, int layer, int window, int residual_flag, int norm_flag, int n_grouped_cell){
    if(layer < 0 || size <= 0){
        fprintf(stderr,"Error: the layer flag must be >= 0 and size param should be > 0\n");
        exit(1);
    }
    
    if(window < 2){
        fprintf(stderr,"Error: your window must be >= 2\n");
        exit(1);
    }
    
    if(norm_flag == GROUP_NORMALIZATION){
        if(n_grouped_cell > window || window%n_grouped_cell){
            fprintf(stderr,"Error: you assumed the group normalization, but the number of grouped cells doesn't divide perfectly the number of unrolled cell during feed forward\n");
            exit(1);
        }
    }
    int i,j;
    lstm* lstml = (lstm*)malloc(sizeof(lstm));
    lstml->layer = layer;
    lstml->size = size;
    lstml->dropout_flag_up = dropout_flag1;
    lstml->dropout_flag_right = dropout_flag2;
    lstml->w = (float**)malloc(sizeof(float*)*4);
    lstml->u = (float**)malloc(sizeof(float*)*4);
    lstml->d_w = (float**)malloc(sizeof(float*)*4);
    lstml->ex_d_w_diff_grad = (float**)malloc(sizeof(float*)*4);
    lstml->d1_w = (float**)malloc(sizeof(float*)*4);
    lstml->d2_w = (float**)malloc(sizeof(float*)*4);
    lstml->d3_w = (float**)malloc(sizeof(float*)*4);
    lstml->d_u = (float**)malloc(sizeof(float*)*4);
    lstml->ex_d_u_diff_grad = (float**)malloc(sizeof(float*)*4);
    lstml->d1_u = (float**)malloc(sizeof(float*)*4);
    lstml->d2_u = (float**)malloc(sizeof(float*)*4);
    lstml->d3_u = (float**)malloc(sizeof(float*)*4);
    lstml->biases = (float**)malloc(sizeof(float*)*4);
    lstml->d_biases = (float**)malloc(sizeof(float*)*4);
    lstml->ex_d_biases_diff_grad = (float**)malloc(sizeof(float*)*4);
    lstml->d1_biases = (float**)malloc(sizeof(float*)*4);
    lstml->d2_biases = (float**)malloc(sizeof(float*)*4);
    lstml->d3_biases = (float**)malloc(sizeof(float*)*4);
    lstml->lstm_z = (float***)malloc(sizeof(float**)*window);
    lstml->lstm_hidden = (float**)malloc(sizeof(float*)*window);
    lstml->out_up = (float**)malloc(sizeof(float*)*window);
    lstml->lstm_cell = (float**)malloc(sizeof(float*)*window);
    lstml->dropout_mask_up = (float*)malloc(sizeof(float)*size);
    lstml->dropout_mask_right = (float*)malloc(sizeof(float)*size);
    lstml->dropout_threshold_up = dropout_threshold1;
    lstml->dropout_threshold_right = dropout_threshold2;
    lstml->residual_flag = residual_flag;
    lstml->norm_flag = norm_flag;
    lstml->n_grouped_cell = n_grouped_cell;
    if(norm_flag == GROUP_NORMALIZATION){
        lstml->bns = (bn**)malloc(sizeof(bn*)*window/n_grouped_cell);
        for(i = 0; i < window/n_grouped_cell; i++){
            lstml->bns[i] = batch_normalization(n_grouped_cell,size,layer,NO_ACTIVATION);
        }
    }
    
    else{
        lstml->bns = NULL;
    }
    
    if(lstml->dropout_flag_up == NO_DROPOUT)
        lstml->dropout_threshold_up = 0;
    if(lstml->dropout_flag_right == NO_DROPOUT)
        lstml->dropout_threshold_right = 0;
    
    for(i = 0; i < window; i++){
        lstml->lstm_z[i] = (float**)malloc(sizeof(float*)*4);
        lstml->lstm_hidden[i] = (float*)calloc(size,sizeof(float));
        lstml->lstm_cell[i] = (float*)calloc(size,sizeof(float));
        lstml->out_up[i] = (float*)calloc(size,sizeof(float));
        for(j = 0; j < 4; j++){
            lstml->lstm_z[i][j] = (float*)calloc(size,sizeof(float));
        }
    }
    
    for(i = 0; i < 4; i++){
        lstml->w[i] = (float*)calloc(size*size,sizeof(float));
        lstml->u[i] = (float*)calloc(size*size,sizeof(float));
        for(j = 0; j < size*size; j++){
            lstml->w[i][j] = random_general_gaussian(0,size);
            lstml->u[i][j] = random_general_gaussian(0,size);
        }
        lstml->d_w[i] = (float*)calloc(size*size,sizeof(float));
        lstml->ex_d_w_diff_grad[i] = (float*)calloc(size*size,sizeof(float));
        lstml->d1_w[i] = (float*)calloc(size*size,sizeof(float));
        lstml->d2_w[i] = (float*)calloc(size*size,sizeof(float));
        lstml->d3_w[i] = (float*)calloc(size*size,sizeof(float));
        lstml->d_u[i] = (float*)calloc(size*size,sizeof(float));
        lstml->ex_d_u_diff_grad[i] = (float*)calloc(size*size,sizeof(float));
        lstml->d1_u[i] = (float*)calloc(size*size,sizeof(float));
        lstml->d2_u[i] = (float*)calloc(size*size,sizeof(float));
        lstml->d3_u[i] = (float*)calloc(size*size,sizeof(float));
        lstml->biases[i] = (float*)calloc(size,sizeof(float));
        lstml->d_biases[i] = (float*)calloc(size,sizeof(float));
        lstml->ex_d_biases_diff_grad[i] = (float*)calloc(size,sizeof(float));
        lstml->d1_biases[i] = (float*)calloc(size,sizeof(float));
        lstml->d2_biases[i] = (float*)calloc(size,sizeof(float));
        lstml->d3_biases[i] = (float*)calloc(size,sizeof(float));
        
    }
    
    for(i = 0; i < size; i++){
        lstml->dropout_mask_up[i] = 1;
        lstml->dropout_mask_right[i] = 1;
    }
    
    lstml->window = window;
    
    return lstml;
    
    
}


/* This function frees the space allocated by a rlstm structure
 * 
 * Inputs:
 * 
 *             @ lstm* rlstm:= the lstm structure that must be deallocated
 * 
 * */
void free_recurrent_lstm(lstm* rlstm){
    
    if(rlstm == NULL)
        return;
        
    
    int i,j;
    
    for(i = 0; i < rlstm->window; i++){
        for(j = 0; j < 4; j++){
            free(rlstm->lstm_z[i][j]);
        }
        free(rlstm->lstm_z[i]);
        free(rlstm->lstm_hidden[i]);
        free(rlstm->lstm_cell[i]);
        free(rlstm->out_up[i]);
    }
    
    for(i = 0; i < 4; i++){
        free(rlstm->w[i]);
        free(rlstm->u[i]);
        free(rlstm->d_w[i]);
        free(rlstm->ex_d_w_diff_grad[i]);
        free(rlstm->d1_w[i]);
        free(rlstm->d2_w[i]);
        free(rlstm->d3_w[i]);
        free(rlstm->d_u[i]);
        free(rlstm->ex_d_u_diff_grad[i]);
        free(rlstm->d1_u[i]);
        free(rlstm->d2_u[i]);
        free(rlstm->d3_u[i]);
        free(rlstm->biases[i]);
        free(rlstm->d_biases[i]);
        free(rlstm->d1_biases[i]);
        free(rlstm->d2_biases[i]);
        free(rlstm->d3_biases[i]);
        free(rlstm->ex_d_biases_diff_grad[i]);
        
    }
    
    if(rlstm->norm_flag == GROUP_NORMALIZATION){
        for(i = 0; i < rlstm->window/rlstm->n_grouped_cell; i++){
            free_batch_normalization(rlstm->bns[i]);
        }
        free(rlstm->bns);
    }
    free(rlstm->w);
    free(rlstm->u);
    free(rlstm->d_w);
    free(rlstm->ex_d_w_diff_grad);
    free(rlstm->d1_w);
    free(rlstm->d2_w);
    free(rlstm->d3_w);
    free(rlstm->d_u);
    free(rlstm->ex_d_u_diff_grad);
    free(rlstm->d1_u);
    free(rlstm->d2_u);
    free(rlstm->d3_u);
    free(rlstm->biases);
    free(rlstm->d_biases);
    free(rlstm->ex_d_biases_diff_grad);
    free(rlstm->d1_biases);
    free(rlstm->d3_biases);
    free(rlstm->lstm_z);
    free(rlstm->lstm_hidden);
    free(rlstm->lstm_cell);
    free(rlstm->out_up);
    free(rlstm->dropout_mask_right);
    free(rlstm->dropout_mask_up);
    free(rlstm);

}

/* This function saves a lstm layer on a .bin file with name n.bin
 * 
 * Input:
 * 
 *             @ lstm* rlstm:= the actual layer that must be saved
 *             @ int n:= the name of the bin file where the layer is saved
 * 
 * 
 * */
void save_lstm(lstm* rlstm, int n){
    if(rlstm == NULL)
        return;
    int i,j;
    FILE* fw;
    char* s = (char*)malloc(sizeof(char)*256);
    char* t = ".bin";
    s = itoa(n,s);
    s = strcat(s,t);
    
    fw = fopen(s,"a+");
    
    if(fw == NULL){
        fprintf(stderr,"Error: error during the opening of the file %s\n",s);
        exit(1);
    }
    
    i = fwrite(&rlstm->residual_flag,sizeof(int),1,fw);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
        exit(1);
    }
    
    i = fwrite(&rlstm->norm_flag,sizeof(int),1,fw);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
        exit(1);
    }
    
    i = fwrite(&rlstm->n_grouped_cell,sizeof(int),1,fw);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
        exit(1);
    }
    
    i = fwrite(&rlstm->size,sizeof(int),1,fw);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
        exit(1);
    }
    
    i = fwrite(&rlstm->layer,sizeof(int),1,fw);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
        exit(1);
    }
    
    i = fwrite(&rlstm->dropout_flag_up,sizeof(int),1,fw);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
        exit(1);
    }
    
    i = fwrite(&rlstm->dropout_flag_right,sizeof(int),1,fw);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
        exit(1);
    }
    
    i = fwrite(&rlstm->window,sizeof(int),1,fw);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
        exit(1);
    }
    
    i = fwrite(&rlstm->dropout_threshold_up,sizeof(float),1,fw);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
        exit(1);
    }
    
    i = fwrite(&rlstm->dropout_threshold_right,sizeof(float),1,fw);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
        exit(1);
    }
    
    
    for(j = 0; j < 4; j++){
        i = fwrite(rlstm->w[j],sizeof(float)*(rlstm->size)*(rlstm->size),1,fw);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
            exit(1);
        }
        
        i = fwrite(rlstm->u[j],sizeof(float)*(rlstm->size)*(rlstm->size),1,fw);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
            exit(1);
        }
        
        i = fwrite(rlstm->biases[j],sizeof(float)*(rlstm->size),1,fw);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
            exit(1);
        }
    }
    
    i = fclose(fw);
    
    if(i != 0){
        fprintf(stderr,"Error: an error occurred closing the file %s\n",s);
        exit(1);
    }
    
    if(rlstm->norm_flag == GROUP_NORMALIZATION){
        for(j = 0; j < rlstm->window/rlstm->n_grouped_cell; j++){
            save_bn(rlstm->bns[j],n);
        }
    }
    free(s);
    
}

/* This function saves a lstm layer on a .bin file with name n.bin
 * 
 * Input:
 * 
 *             @ lstm* rlstm:= the actual layer that must be saved
 *             @ int n:= the name of the bin file where the layer is saved
 * 
 * 
 * */
void heavy_save_lstm(lstm* rlstm, int n){
    if(rlstm == NULL)
        return;
    int i,j;
    FILE* fw;
    char* s = (char*)malloc(sizeof(char)*256);
    char* t = ".bin";
    s = itoa(n,s);
    s = strcat(s,t);
    
    fw = fopen(s,"a+");
    
    if(fw == NULL){
        fprintf(stderr,"Error: error during the opening of the file %s\n",s);
        exit(1);
    }
    
    i = fwrite(&rlstm->residual_flag,sizeof(int),1,fw);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
        exit(1);
    }
    
    i = fwrite(&rlstm->norm_flag,sizeof(int),1,fw);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
        exit(1);
    }
    
    i = fwrite(&rlstm->n_grouped_cell,sizeof(int),1,fw);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
        exit(1);
    }
    
    i = fwrite(&rlstm->size,sizeof(int),1,fw);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
        exit(1);
    }
    
    i = fwrite(&rlstm->layer,sizeof(int),1,fw);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
        exit(1);
    }
    
    i = fwrite(&rlstm->dropout_flag_up,sizeof(int),1,fw);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
        exit(1);
    }
    
    i = fwrite(&rlstm->dropout_flag_right,sizeof(int),1,fw);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
        exit(1);
    }
    
    i = fwrite(&rlstm->window,sizeof(int),1,fw);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
        exit(1);
    }
    
    i = fwrite(&rlstm->dropout_threshold_up,sizeof(float),1,fw);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
        exit(1);
    }
    
    i = fwrite(&rlstm->dropout_threshold_right,sizeof(float),1,fw);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
        exit(1);
    }
    
    
    for(j = 0; j < 4; j++){
        i = fwrite(rlstm->w[j],sizeof(float)*(rlstm->size)*(rlstm->size),1,fw);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
            exit(1);
        }
        
        i = fwrite(rlstm->d1_w[j],sizeof(float)*(rlstm->size)*(rlstm->size),1,fw);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
            exit(1);
        }
        
        i = fwrite(rlstm->d2_w[j],sizeof(float)*(rlstm->size)*(rlstm->size),1,fw);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
            exit(1);
        }
        
        i = fwrite(rlstm->d3_w[j],sizeof(float)*(rlstm->size)*(rlstm->size),1,fw);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
            exit(1);
        }
        
        i = fwrite(rlstm->ex_d_w_diff_grad[j],sizeof(float)*(rlstm->size)*(rlstm->size),1,fw);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
            exit(1);
        }
        i = fwrite(rlstm->u[j],sizeof(float)*(rlstm->size)*(rlstm->size),1,fw);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
            exit(1);
        }
        
        i = fwrite(rlstm->d1_u[j],sizeof(float)*(rlstm->size)*(rlstm->size),1,fw);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
            exit(1);
        }
        
        i = fwrite(rlstm->d2_u[j],sizeof(float)*(rlstm->size)*(rlstm->size),1,fw);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
            exit(1);
        }
        
        i = fwrite(rlstm->d3_u[j],sizeof(float)*(rlstm->size)*(rlstm->size),1,fw);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
            exit(1);
        }
        
        i = fwrite(rlstm->ex_d_u_diff_grad[j],sizeof(float)*(rlstm->size)*(rlstm->size),1,fw);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
            exit(1);
        }
        i = fwrite(rlstm->biases[j],sizeof(float)*(rlstm->size),1,fw);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
            exit(1);
        }
        
        i = fwrite(rlstm->d1_biases[j],sizeof(float)*(rlstm->size),1,fw);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
            exit(1);
        }
        
        i = fwrite(rlstm->d2_biases[j],sizeof(float)*(rlstm->size),1,fw);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
            exit(1);
        }
        
        i = fwrite(rlstm->d3_biases[j],sizeof(float)*(rlstm->size),1,fw);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
            exit(1);
        }
        
        i = fwrite(rlstm->ex_d_biases_diff_grad[j],sizeof(float)*(rlstm->size),1,fw);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred saving a lstm layer\n");
            exit(1);
        }
    }
    
    i = fclose(fw);
    
    if(i != 0){
        fprintf(stderr,"Error: an error occurred closing the file %s\n",s);
        exit(1);
    }
    
    if(rlstm->norm_flag == GROUP_NORMALIZATION){
        for(j = 0; j < rlstm->window/rlstm->n_grouped_cell; j++){
            heavy_save_bn(rlstm->bns[j],n);
        }
    }
    free(s);
    
}

/* This function loads a lstm layer from a .bin file from fr
 * 
 * Input:
 * 
 *             @ FILE* fr:= a pointer to a file already opened
 * 
 * */
lstm* load_lstm(FILE* fr){
    if(fr == NULL)
        return NULL;
    int i,j;
    
    int size = 0,layer = 0,dropout_flag_up = 0,dropout_flag_right = 0, window = 0, residual_flag = 0, norm_flag = 0, n_grouped_cell = 0;
    float dropout_threshold_right = 0,dropout_threshold_up = 0;
    float** w = (float**)malloc(sizeof(float*)*4);
    float** u = (float**)malloc(sizeof(float*)*4);
    float** biases = (float**)malloc(sizeof(float*)*4);
    
    i = fread(&residual_flag,sizeof(int),1,fr);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
        exit(1);
    }
    
    i = fread(&norm_flag,sizeof(int),1,fr);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
        exit(1);
    }
    
    i = fread(&n_grouped_cell,sizeof(int),1,fr);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
        exit(1);
    }
    
    i = fread(&size,sizeof(int),1,fr);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
        exit(1);
    }
    
    i = fread(&layer,sizeof(int),1,fr);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
        exit(1);
    }
    
    i = fread(&dropout_flag_up,sizeof(int),1,fr);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
        exit(1);
    }
    
    i = fread(&dropout_flag_right,sizeof(int),1,fr);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
        exit(1);
    }
    
    i = fread(&window,sizeof(int),1,fr);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
        exit(1);
    }
    
    i = fread(&dropout_threshold_up,sizeof(float),1,fr);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
        exit(1);
    }
    
    i = fread(&dropout_threshold_right,sizeof(float),1,fr);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
        exit(1);
    }
    
    for(j = 0; j < 4; j++){
        w[j] = (float*)malloc(sizeof(float)*size*size);
        u[j] = (float*)malloc(sizeof(float)*size*size);
        biases[j] = (float*)malloc(sizeof(float)*size);
        
        i = fread(w[j],sizeof(float)*(size)*(size),1,fr);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
            exit(1);
        }
        
        i = fread(u[j],sizeof(float)*(size)*(size),1,fr);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
            exit(1);
        }
        
        i = fread(biases[j],sizeof(float)*(size),1,fr);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
            exit(1);
        }
    }
    
    bn** bns = NULL;
    if(norm_flag == GROUP_NORMALIZATION){
        bns = (bn**)malloc(sizeof(bn*)*window/n_grouped_cell);
        for(i = 0; i < window/n_grouped_cell; i++){
            bns[i] = load_bn(fr);
        }
    }
    
    lstm* l = recurrent_lstm(size,dropout_flag_up,dropout_threshold_up,dropout_flag_right,dropout_threshold_right,layer, window, residual_flag,norm_flag,n_grouped_cell);
    for(i = 0; i < 4; i++){
        copy_array(w[i],l->w[i],size*size);
        copy_array(u[i],l->u[i],size*size);
        copy_array(biases[i],l->biases[i],size);
        free(w[i]);
        free(u[i]);
        free(biases[i]);
    }
    
    if(norm_flag == GROUP_NORMALIZATION){
        for(i = 0; i < window/n_grouped_cell; i++){
            free(l->bns[i]);
        }
        free(l->bns);
        l->bns = bns;
    }
    
    
    
    free(w);
    free(u);
    free(biases);
    return l;
}

/* This function loads a lstm layer from a .bin file from fr
 * 
 * Input:
 * 
 *             @ FILE* fr:= a pointer to a file already opened
 * 
 * */
lstm* heavy_load_lstm(FILE* fr){
    if(fr == NULL)
        return NULL;
    int i,j;
    
    int size = 0,layer = 0,dropout_flag_up = 0,dropout_flag_right = 0, window = 0, residual_flag = 0, norm_flag = 0, n_grouped_cell = 0;
    float dropout_threshold_right = 0,dropout_threshold_up = 0;
    float** w = (float**)malloc(sizeof(float*)*4);
    float** d1_w = (float**)malloc(sizeof(float*)*4);
    float** d2_w = (float**)malloc(sizeof(float*)*4);
    float** d3_w = (float**)malloc(sizeof(float*)*4);
    float** ex_d_w_diff_grad = (float**)malloc(sizeof(float*)*4);
    float** u = (float**)malloc(sizeof(float*)*4);
    float** d1_u = (float**)malloc(sizeof(float*)*4);
    float** d2_u = (float**)malloc(sizeof(float*)*4);
    float** d3_u = (float**)malloc(sizeof(float*)*4);
    float** ex_d_u_diff_grad = (float**)malloc(sizeof(float*)*4);
    float** biases = (float**)malloc(sizeof(float*)*4);
    float** d1_biases = (float**)malloc(sizeof(float*)*4);
    float** d2_biases = (float**)malloc(sizeof(float*)*4);
    float** d3_biases = (float**)malloc(sizeof(float*)*4);
    float** ex_d_biases_diff_grad = (float**)malloc(sizeof(float*)*4);
    
    i = fread(&residual_flag,sizeof(int),1,fr);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
        exit(1);
    }
    
    i = fread(&norm_flag,sizeof(int),1,fr);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
        exit(1);
    }
    
    i = fread(&n_grouped_cell,sizeof(int),1,fr);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
        exit(1);
    }
    
    i = fread(&size,sizeof(int),1,fr);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
        exit(1);
    }
    
    i = fread(&layer,sizeof(int),1,fr);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
        exit(1);
    }
    
    i = fread(&dropout_flag_up,sizeof(int),1,fr);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
        exit(1);
    }
    
    i = fread(&dropout_flag_right,sizeof(int),1,fr);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
        exit(1);
    }



    i = fread(&window,sizeof(int),1,fr);
    if(i != 1){
        fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
        exit(1);
    }
    
    i = fread(&dropout_threshold_up,sizeof(float),1,fr);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
        exit(1);
    }
    
    i = fread(&dropout_threshold_right,sizeof(float),1,fr);
    
    if(i != 1){
        fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
        exit(1);
    }
    
    for(j = 0; j < 4; j++){
        w[j] = (float*)malloc(sizeof(float)*size*size);
        d1_w[j] = (float*)malloc(sizeof(float)*size*size);
        d2_w[j] = (float*)malloc(sizeof(float)*size*size);
        d3_w[j] = (float*)malloc(sizeof(float)*size*size);
        ex_d_w_diff_grad[j] = (float*)malloc(sizeof(float)*size*size);
        u[j] = (float*)malloc(sizeof(float)*size*size);
        d1_u[j] = (float*)malloc(sizeof(float)*size*size);
        d2_u[j] = (float*)malloc(sizeof(float)*size*size);
        d3_u[j] = (float*)malloc(sizeof(float)*size*size);
        ex_d_u_diff_grad[j] = (float*)malloc(sizeof(float)*size*size);
        biases[j] = (float*)malloc(sizeof(float)*size);
        d1_biases[j] = (float*)malloc(sizeof(float)*size);
        d2_biases[j] = (float*)malloc(sizeof(float)*size);
        d3_biases[j] = (float*)malloc(sizeof(float)*size);
        ex_d_biases_diff_grad[j] = (float*)malloc(sizeof(float)*size);
        
        i = fread(w[j],sizeof(float)*(size)*(size),1,fr);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
            exit(1);
        }
        
        i = fread(d1_w[j],sizeof(float)*(size)*(size),1,fr);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
            exit(1);
        }
        
        i = fread(d2_w[j],sizeof(float)*(size)*(size),1,fr);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
            exit(1);
        }
        
        i = fread(d3_w[j],sizeof(float)*(size)*(size),1,fr);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
            exit(1);
        }
        
        i = fread(ex_d_w_diff_grad[j],sizeof(float)*(size)*(size),1,fr);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
            exit(1);
        }
        i = fread(u[j],sizeof(float)*(size)*(size),1,fr);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
            exit(1);
        }
        
        i = fread(d1_u[j],sizeof(float)*(size)*(size),1,fr);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
            exit(1);
        }
        i = fread(d2_u[j],sizeof(float)*(size)*(size),1,fr);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
            exit(1);
        }
        i = fread(d3_u[j],sizeof(float)*(size)*(size),1,fr);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
            exit(1);
        }
        i = fread(ex_d_u_diff_grad[j],sizeof(float)*(size)*(size),1,fr);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
            exit(1);
        }
        i = fread(biases[j],sizeof(float)*(size),1,fr);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
            exit(1);
        }
        i = fread(d1_biases[j],sizeof(float)*(size),1,fr);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
            exit(1);
        }
        
        i = fread(d2_biases[j],sizeof(float)*(size),1,fr);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
            exit(1);
        }
        
        i = fread(d3_biases[j],sizeof(float)*(size),1,fr);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
            exit(1);
        }
        
        i = fread(ex_d_biases_diff_grad[j],sizeof(float)*(size),1,fr);
    
        if(i != 1){
            fprintf(stderr,"Error: an error occurred loading a lstm layer\n");
            exit(1);
        }
    }
    
    
    bn** bns = NULL;
    if(norm_flag == GROUP_NORMALIZATION){
        bns = (bn**)malloc(sizeof(bn*)*window/n_grouped_cell);
        for(i = 0; i < window/n_grouped_cell; i++){
            bns[i] = heavy_load_bn(fr);
        }
    }
    
    lstm* l = recurrent_lstm(size,dropout_flag_up,dropout_threshold_up,dropout_flag_right,dropout_threshold_right,layer, window, residual_flag,norm_flag,n_grouped_cell);
    for(i = 0; i < 4; i++){
        copy_array(w[i],l->w[i],size*size);
        copy_array(d1_w[i],l->d1_w[i],size*size);
        copy_array(d2_w[i],l->d2_w[i],size*size);
        copy_array(d3_w[i],l->d3_w[i],size*size);
        copy_array(ex_d_w_diff_grad[i],l->ex_d_w_diff_grad[i],size*size);
        copy_array(u[i],l->u[i],size*size);
        copy_array(d1_u[i],l->d1_u[i],size*size);
        copy_array(d2_u[i],l->d2_u[i],size*size);
        copy_array(d3_u[i],l->d3_u[i],size*size);
        copy_array(ex_d_u_diff_grad[i],l->ex_d_u_diff_grad[i],size*size);
        copy_array(biases[i],l->biases[i],size);
        copy_array(d1_biases[i],l->d1_biases[i],size);
        copy_array(d2_biases[i],l->d2_biases[i],size);
        copy_array(d3_biases[i],l->d3_biases[i],size);
        copy_array(ex_d_biases_diff_grad[i],l->ex_d_biases_diff_grad[i],size);
        free(w[i]);
        free(d1_w[i]);
        free(d2_w[i]);
        free(d3_w[i]);
        free(ex_d_w_diff_grad[i]);
        free(u[i]);
        free(d1_u[i]);
        free(d2_u[i]);
        free(d3_u[i]);
        free(ex_d_u_diff_grad[i]);
        free(biases[i]);
        free(d1_biases[i]);
        free(d2_biases[i]);
        free(d3_biases[i]);
        free(ex_d_biases_diff_grad[i]);
    }
    
    if(norm_flag == GROUP_NORMALIZATION){
        for(i = 0; i < window/n_grouped_cell; i++){
            free(l->bns[i]);
        }
        free(l->bns);
        l->bns = bns;
    }
    
    
    
    free(w);
    free(d1_w);
    free(d2_w);
    free(d3_w);
    free(ex_d_w_diff_grad);
    free(u);
    free(d1_u);
    free(d2_u);
    free(d3_u);
    free(ex_d_u_diff_grad);
    free(biases);
    free(d1_biases);
    free(d2_biases);
    free(d3_biases);
    free(ex_d_biases_diff_grad);
    return l;
}


/* This function returns a lstm* layer that is the same copy of the input l
 * except for the temporary arrays used during the feed forward and backprop.
 * You have a lstm* l structure, this function creates an identical structure
 * with all the arrays used for the feed forward and back propagation
 * with all the initial states. and the same weights and derivatives in l are copied
 * into the new structure. d1 and d2 weights are used by nesterov and adam algorithms
 * 
 * Input:
 * 
 *             @ lstm* l:= the lstm layer that must be copied
 * 
 * */
lstm* copy_lstm(lstm* l){
    if(l == NULL)
        return NULL;
    int i;
    
    lstm* copy = recurrent_lstm(l->size,l->dropout_flag_up,l->dropout_threshold_up,l->dropout_flag_right,l->dropout_threshold_right,l->layer, l->window,l->residual_flag,l->norm_flag,l->n_grouped_cell);
    for(i = 0; i < 4; i++){
        copy_array(l->w[i],copy->w[i],l->size*l->size);
        copy_array(l->d_w[i],copy->d_w[i],l->size*l->size);
        copy_array(l->ex_d_w_diff_grad[i],copy->ex_d_w_diff_grad[i],l->size*l->size);
        copy_array(l->d1_w[i],copy->d1_w[i],l->size*l->size);
        copy_array(l->d2_w[i],copy->d2_w[i],l->size*l->size);
        copy_array(l->d3_w[i],copy->d3_w[i],l->size*l->size);
        copy_array(l->u[i],copy->u[i],l->size*l->size);
        copy_array(l->d_u[i],copy->d_u[i],l->size*l->size);
        copy_array(l->ex_d_u_diff_grad[i],copy->ex_d_u_diff_grad[i],l->size*l->size);
        copy_array(l->d1_u[i],copy->d1_u[i],l->size*l->size);
        copy_array(l->d2_u[i],copy->d2_u[i],l->size*l->size);
        copy_array(l->d3_u[i],copy->d3_u[i],l->size*l->size);
        copy_array(l->biases[i],copy->biases[i],l->size);
        copy_array(l->d_biases[i],copy->d_biases[i],l->size);
        copy_array(l->ex_d_biases_diff_grad[i],copy->ex_d_biases_diff_grad[i],l->size);
        copy_array(l->d1_biases[i],copy->d1_biases[i],l->size);
        copy_array(l->d2_biases[i],copy->d2_biases[i],l->size);
        copy_array(l->d3_biases[i],copy->d3_biases[i],l->size);
    }
    
    if(l->norm_flag == GROUP_NORMALIZATION){
        for(i = 0; i < l->window/l->n_grouped_cell; i++){
            copy_array(l->bns[i]->gamma,copy->bns[i]->gamma,l->bns[i]->vector_dim);
            copy_array(l->bns[i]->d_gamma,copy->bns[i]->d_gamma,l->bns[i]->vector_dim);
            copy_array(l->bns[i]->d1_gamma,copy->bns[i]->d1_gamma,l->bns[i]->vector_dim);
            copy_array(l->bns[i]->d2_gamma,copy->bns[i]->d2_gamma,l->bns[i]->vector_dim);
            copy_array(l->bns[i]->d3_gamma,copy->bns[i]->d3_gamma,l->bns[i]->vector_dim);
            copy_array(l->bns[i]->ex_d_gamma_diff_grad,copy->bns[i]->ex_d_gamma_diff_grad,l->bns[i]->vector_dim);
            copy_array(l->bns[i]->beta,copy->bns[i]->beta,l->bns[i]->vector_dim);
            copy_array(l->bns[i]->d_beta,copy->bns[i]->d_beta,l->bns[i]->vector_dim);
            copy_array(l->bns[i]->d2_beta,copy->bns[i]->d2_beta,l->bns[i]->vector_dim);
            copy_array(l->bns[i]->d3_beta,copy->bns[i]->d3_beta,l->bns[i]->vector_dim);
            copy_array(l->bns[i]->ex_d_beta_diff_grad,copy->bns[i]->ex_d_beta_diff_grad,l->bns[i]->vector_dim);
        }
    }
    
    
    
    return copy;
}


/* This function returns a lstm* layer that is the same copy of the input l
 * except for the arrays used during the ff and bp
 * This functions copies the weights and D and D1 and D2 into a another structure
 * 
 * Input:
 * 
 *             @ lstm* l:= the lstm layer that must be copied
 *             @ lstm* copy:= the lstm layer where l is copied
 * 
 * */
void paste_lstm(lstm* l,lstm* copy){
    if(l == NULL)
        return;
        
    int i;
    for(i = 0; i < 4; i++){
        copy_array(l->w[i],copy->w[i],l->size*l->size);
        copy_array(l->d_w[i],copy->d_w[i],l->size*l->size);
        copy_array(l->ex_d_w_diff_grad[i],copy->ex_d_w_diff_grad[i],l->size*l->size);
        copy_array(l->d1_w[i],copy->d1_w[i],l->size*l->size);
        copy_array(l->d2_w[i],copy->d2_w[i],l->size*l->size);
        copy_array(l->d3_w[i],copy->d3_w[i],l->size*l->size);
        copy_array(l->u[i],copy->u[i],l->size*l->size);
        copy_array(l->d_u[i],copy->d_u[i],l->size*l->size);
        copy_array(l->ex_d_u_diff_grad[i],copy->ex_d_u_diff_grad[i],l->size*l->size);
        copy_array(l->d1_u[i],copy->d1_u[i],l->size*l->size);
        copy_array(l->d2_u[i],copy->d2_u[i],l->size*l->size);
        copy_array(l->d3_u[i],copy->d3_u[i],l->size*l->size);
        copy_array(l->biases[i],copy->biases[i],l->size);
        copy_array(l->d_biases[i],copy->d_biases[i],l->size);
        copy_array(l->ex_d_biases_diff_grad[i],copy->ex_d_biases_diff_grad[i],l->size);
        copy_array(l->d1_biases[i],copy->d1_biases[i],l->size);
        copy_array(l->d2_biases[i],copy->d2_biases[i],l->size);
        copy_array(l->d3_biases[i],copy->d3_biases[i],l->size);
    }
    
    if(l->norm_flag == GROUP_NORMALIZATION){
        for(i = 0; i < l->window/l->n_grouped_cell; i++){
            paste_bn(l->bns[i],copy->bns[i]);
        }
    }
    return;
}

/* This function returns a lstm* layer that is the same copy of the input l
 * except for the arrays used during the ff and bp
 * This functions copies the weights and D and D1 and D2 into a another structure
 * 
 * Input:
 * 
 *             @ lstm* l:= the lstm layer that must be copied
 *             @ lstm* copy:= the lstm layer where l is copied
 * 
 * */
void paste_w_lstm(lstm* l,lstm* copy){
    if(l == NULL)
        return;
        
    int i;
    for(i = 0; i < 4; i++){
        copy_array(l->w[i],copy->w[i],l->size*l->size);
        copy_array(l->u[i],copy->u[i],l->size*l->size);
        copy_array(l->biases[i],copy->biases[i],l->size);
    }
    
    if(l->norm_flag == GROUP_NORMALIZATION){
        for(i = 0; i < l->window/l->n_grouped_cell; i++){
            paste_w_bn(l->bns[i],copy->bns[i]);
        }
    }
    return;
}
/* This function returns a lstm* layer that is the same copy for the weights and biases
 * of the layer l with the rule teta_i = tau*teta_j + (1-tau)*teta_i
 * 
 * Input:
 * 
 *             @ lstm* l:= the lstm layer that must be copied
 *             @ lstm* copy:= the lstm layer where l is copied
 *             @ float tau:= the tau param
 * */
void slow_paste_lstm(lstm* l,lstm* copy, float tau){
    if(l == NULL)
        return;
    int i,j;
    for(i = 0; i < 4; i++){
        for(j = 0; j < l->size*l->size; j++){
            copy->w[i][j] = tau*l->w[i][j] + (1-tau)*copy->w[i][j];
            copy->d1_w[i][j] = tau*l->d1_w[i][j] + (1-tau)*copy->d1_w[i][j];
            copy->d2_w[i][j] = tau*l->d2_w[i][j] + (1-tau)*copy->d2_w[i][j];
            copy->d3_w[i][j] = tau*l->d3_w[i][j] + (1-tau)*copy->d3_w[i][j];
            copy->ex_d_w_diff_grad[i][j] = tau*l->ex_d_w_diff_grad[i][j] + (1-tau)*copy->ex_d_w_diff_grad[i][j];
            copy->u[i][j] = tau*l->u[i][j] + (1-tau)*copy->u[i][j];
            copy->d1_u[i][j] = tau*l->d1_u[i][j] + (1-tau)*copy->d1_u[i][j];
            copy->d2_u[i][j] = tau*l->d2_u[i][j] + (1-tau)*copy->d2_u[i][j];
            copy->d3_u[i][j] = tau*l->d3_u[i][j] + (1-tau)*copy->d3_u[i][j];
            copy->ex_d_u_diff_grad[i][j] = tau*l->ex_d_u_diff_grad[i][j] + (1-tau)*copy->ex_d_u_diff_grad[i][j];
            if(j < l->size){
                copy->biases[i][j] = tau*l->biases[i][j] + (1-tau)*copy->biases[i][j];
                copy->d1_biases[i][j] = tau*l->d1_biases[i][j] + (1-tau)*copy->d1_biases[i][j];
                copy->d2_biases[i][j] = tau*l->d2_biases[i][j] + (1-tau)*copy->d2_biases[i][j];
                copy->d3_biases[i][j] = tau*l->d3_biases[i][j] + (1-tau)*copy->d3_biases[i][j];
                copy->ex_d_biases_diff_grad[i][j] = tau*l->ex_d_biases_diff_grad[i][j] + (1-tau)*copy->ex_d_biases_diff_grad[i][j];
            }
        }
    }
    
    if(l->norm_flag == GROUP_NORMALIZATION){
        for(i = 0; i < l->window/l->n_grouped_cell; i++){
            slow_paste_bn(l->bns[i],copy->bns[i],tau);
        }
    }
    return;
}

/* this function reset all the arrays of a lstm layer
 * used during the feed forward and backpropagation
 * You have a lstm* f structure, this function resets all the arrays used
 * for the feed forward and back propagation with partial derivatives D inculded
 * but the weights and D1 and D2 don't change
 * 
 * 
 * Input:
 * 
 *             @ lstm* f:= a lstm* f layer
 * 
 * */
lstm* reset_lstm(lstm* f){
    if(f == NULL)
        return NULL;
    int i,j,k;
    for(i = 0; i < 4; i++){
        for(j = 0; j < f->size*f->size; j++){
            f->d_w[i][j] = 0;
            f->d_u[i][j] = 0;
            if(j < f->size){
                f->d_biases[i][j] = 0;
                if(!i){
                    f->dropout_mask_up[j] = 1;
                    f->dropout_mask_right[j] = 1;
                }
            }
        }
    }
    for(i = 0; i < f->window; i++){
        for(j = 0; j < 4; j++){
            for(k = 0; k < f->size; k++){
                f->lstm_z[i][j][k] = 0;                        
                f->lstm_hidden[i][k] = 0;
                f->lstm_cell[i][k] = 0;
                f->out_up[i][k] = 0;
            }
        }
    }
    
    if(f->norm_flag == GROUP_NORMALIZATION){
        for(i = 0; i < f->window/f->n_grouped_cell; i++){
            reset_bn(f->bns[i]);
        }
    }
    return f;
}


/* this function gives the number of float params for biases and weights in a lstm
 * 
 * Input:
 * 
 * 
 *                 @ lstm* f:= the lstm layer
 * */
int get_array_size_params_lstm(lstm* f){
    
    int sum = 0;
    int i;
    if(f->norm_flag == GROUP_NORMALIZATION){
        for(i = 0; i < f->window/f->n_grouped_cell; i++){
            sum+=f->bns[i]->vector_dim*2;
        }
    }
    return sum+12*f->size*f->size+6*f->size;
}

/* this function paste the weights and biases in a single vector
 * 
 * Inputs:
 * 
 * 
 *                 @ lstm* f:= the lstm layer
 *                 @ float* vector:= the vector where is copyed everything
 * */
void memcopy_vector_to_params_lstm(lstm* f, float* vector){
    int i;
    for(i = 0; i < 4; i++){
        memcpy(f->w[i],&vector[(i*2)*f->size*f->size],f->size*f->size*sizeof(float));    
        memcpy(f->u[i],&vector[(i*2+1)*f->size*f->size],f->size*f->size*sizeof(float));    
    }
    
    for(i = 0; i < 4; i++){
        memcpy(f->biases[i],&vector[8*f->size*f->size+(i)*f->size],f->size*sizeof(float));        
    }
    
    if(f->norm_flag == GROUP_NORMALIZATION){
        for(i = 0; i < f->window/f->n_grouped_cell; i++){
            memcpy(f->bns[i]->gamma,&vector[8*f->size*f->size+4*f->size+i*f->bns[i]->vector_dim],f->bns[i]->vector_dim*sizeof(float));
        }
        
        for(i = 0; i < f->window/f->n_grouped_cell; i++){
            memcpy(f->bns[i]->beta,&vector[8*f->size*f->size+4*f->size+(f->window/f->n_grouped_cell+i)*f->bns[i]->vector_dim],f->bns[i]->vector_dim*sizeof(float));
        }
    }
}


/* this function paste the vector in the weights and biases of the lstm
 * 
 * Inputs:
 * 
 * 
 *                 @ lstm* f:= the lstm layer
 *                 @ float* vector:= the vector where is copyed everything
 * */
void memcopy_params_to_vector_lstm(lstm* f, float* vector){
    int i;
    for(i = 0; i < 4; i++){
        memcpy(&vector[(i*2)*f->size*f->size],f->w[i],f->size*f->size*sizeof(float));    
        memcpy(&vector[(i*2+1)*f->size*f->size],f->u[i],f->size*f->size*sizeof(float));    
    }
    
    for(i = 0; i < 4; i++){
        memcpy(&vector[8*f->size*f->size+(i)*f->size],f->biases[i],f->size*sizeof(float));        
    }
    
    if(f->norm_flag == GROUP_NORMALIZATION){
        for(i = 0; i < f->window/f->n_grouped_cell; i++){
            memcpy(&vector[8*f->size*f->size+4*f->size+i*f->bns[i]->vector_dim],f->bns[i]->gamma,f->bns[i]->vector_dim*sizeof(float));
        }
        
        for(i = 0; i < f->window/f->n_grouped_cell; i++){
            memcpy(&vector[8*f->size*f->size+4*f->size+(f->window/f->n_grouped_cell+i)*f->bns[i]->vector_dim],f->bns[i]->beta,f->bns[i]->vector_dim*sizeof(float));
        }
    }
}

/* this function paste the dweights and dbiases in a single vector
 * 
 * Inputs:
 * 
 * 
 *                 @ lstm* f:= the lstm layer
 *                 @ float* vector:= the vector where is copyed everything
 * */
void memcopy_vector_to_derivative_params_lstm(lstm* f, float* vector){
    int i;
    for(i = 0; i < 4; i++){
        memcpy(f->d_w[i],&vector[(i*2)*f->size*f->size],f->size*f->size*sizeof(float));    
        memcpy(f->d_u[i],&vector[(i*2+1)*f->size*f->size],f->size*f->size*sizeof(float));    
    }
    
    for(i = 0; i < 4; i++){
        memcpy(f->d_biases[i],&vector[8*f->size*f->size+(i)*f->size],f->size*sizeof(float));        
    }
    
    if(f->norm_flag == GROUP_NORMALIZATION){
        for(i = 0; i < f->window/f->n_grouped_cell; i++){
            memcpy(f->bns[i]->d_gamma,&vector[8*f->size*f->size+4*f->size+i*f->bns[i]->vector_dim],f->bns[i]->vector_dim*sizeof(float));
        }
        
        for(i = 0; i < f->window/f->n_grouped_cell; i++){
            memcpy(f->bns[i]->d_beta,&vector[8*f->size*f->size+4*f->size+(f->window/f->n_grouped_cell+i)*f->bns[i]->vector_dim],f->bns[i]->vector_dim*sizeof(float));
        }
    }
}


/* this function paste the vector in the dweights and dbiases of the lstm
 * 
 * Inputs:
 * 
 * 
 *                 @ lstm* f:= the lstm layer
 *                 @ float* vector:= the vector where is copyed everything
 * */
void memcopy_derivative_params_to_vector_lstm(lstm* f, float* vector){
    int i;
    for(i = 0; i < 4; i++){
        memcpy(&vector[(i*2)*f->size*f->size],f->d_w[i],f->size*f->size*sizeof(float));    
        memcpy(&vector[(i*2+1)*f->size*f->size],f->d_u[i],f->size*f->size*sizeof(float));    
    }
    
    for(i = 0; i < 4; i++){
        memcpy(&vector[8*f->size*f->size+(i)*f->size],f->d_biases[i],f->size*sizeof(float));        
    }
    
    if(f->norm_flag == GROUP_NORMALIZATION){
        for(i = 0; i < f->window/f->n_grouped_cell; i++){
            memcpy(&vector[8*f->size*f->size+4*f->size+i*f->bns[i]->vector_dim],f->bns[i]->d_gamma,f->bns[i]->vector_dim*sizeof(float));
        }
        
        for(i = 0; i < f->window/f->n_grouped_cell; i++){
            memcpy(&vector[8*f->size*f->size+4*f->size+(f->window/f->n_grouped_cell+i)*f->bns[i]->vector_dim],f->bns[i]->d_beta,f->bns[i]->vector_dim*sizeof(float));
        }
    }
}
