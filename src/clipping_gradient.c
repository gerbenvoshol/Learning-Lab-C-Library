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

/* This function, given a threshold, clips the gradient of the weights of the whole model given by all the models m and rmodels r if the ||DL/Dw|| > threshold,
 * in that case DL/Dw_i *= threshold/||DL/Dw||
 * 
 * Input:
 * 
 *             @ model** m:= the models
 *             @ model** r:= the rmodels
 *             @ int n_m:= the number of models
 *             @ int n_r:= the number of rmodels
 *             @ float threshold:= the threshold
 * 
 * */
void general_clipping_gradient(model** m, rmodel** r, int n_m, int n_r, float threshold){
    double sum = 0;
    int i,j;
    for(i = 0; i < n_m; i++){
        sum += (double)sum_all_quadratic_derivative_weights_fcls(m[i]->fcls,m[i]->n_fcl);
        sum += (double)sum_all_quadratic_derivative_weights_cls(m[i]->cls,m[i]->n_cl);
        sum += (double)sum_all_quadratic_derivative_weights_rls(m[i]->rls,m[i]->n_rl);
    }
    
    for(i = 0; i < n_r; i++){
        sum += (double)sum_all_quadratic_derivative_weights_lstms(r[i]->lstms,r[i]->layers);
    }
    
    sum = sqrtf(sum);
    if(sum >= threshold){
        for(i = 0; i < n_m; i++){
            clip_fcls(m[i]->fcls,m[i]->n_fcl,threshold,sum);
            clip_cls(m[i]->cls,m[i]->n_cl,threshold,sum);
            clip_rls(m[i]->rls,m[i]->n_rl,threshold,sum);
        }
    }
    for(i = 0; i < n_r; i++){
        clip_lstms(r[i]->lstms,r[i]->layers,threshold,sum);
    }
}
/* This function, given a threshold, clips the gradient of the weights of the model if the ||DL/Dw|| > threshold,
 * in that case DL/Dw_i *= threshold/||DL/Dw||
 * 
 * Input:
 * 
 *             @ model* m:= the model
 *             @ float threshold:= the threshold
 * 
 * */
 
void clipping_gradient(model* m, float threshold) {
     float sum = 0;
     sum += sum_all_quadratic_derivative_weights_fcls(m->fcls, m->n_fcl);
     sum += sum_all_quadratic_derivative_weights_cls(m->cls, m->n_cl);
     sum += sum_all_quadratic_derivative_weights_rls(m->rls, m->n_rl);
     
     sum = sqrtf(sum);
     if(sum >= threshold){
         clip_fcls(m->fcls, m->n_fcl, threshold, sum);
         clip_cls(m->cls, m->n_cl, threshold, sum);
         clip_rls(m->rls, m->n_rl, threshold, sum);
     }
}

/* This function, given a threshold, clips the gradient of the weights of the rmodel if the ||DL/Dw|| > threshold
 * in that case DL/Dw_i *= threshold/||DL/Dw||
 * 
 * Input:
 * 
 *             @ rmodel* m:= the recurrent model
 *             @ float threshold:= the threshold
 * 
 * */
 
void clipping_gradient_rmodel(rmodel* m, float threshold) {
     float sum = 0;
     int i,j;
     sum += sum_all_quadratic_derivative_weights_lstms(m->lstms,m->layers);
     sum = sqrtf(sum);
     if(sum >= threshold)
         clip_lstms(m->lstms,m->layers,threshold,sum);   
}


/* This function, given a threshold, clips the gradient of the weights of the vaemodel if the ||DL/Dw|| > threshold
 * in that case DL/Dw_i *= threshold/||DL/Dw||
 * 
 * Input:
 * 
 *             @ vaemodel* vm:= the variational autoencoder model
 *             @ float threshold:= the threshold
 * 
 * */
void clipping_gradient_vae_model(vaemodel* vm, float threshold) {
     float sum = 0;
     sum += sum_all_quadratic_derivative_weights_fcls(vm->encoder->fcls, vm->encoder->n_fcl);
     sum += sum_all_quadratic_derivative_weights_cls(vm->encoder->cls, vm->encoder->n_cl);
     sum += sum_all_quadratic_derivative_weights_rls(vm->encoder->rls, vm->encoder->n_rl);
     sum += sum_all_quadratic_derivative_weights_fcls(vm->decoder->fcls, vm->decoder->n_fcl);
     sum += sum_all_quadratic_derivative_weights_cls(vm->decoder->cls, vm->decoder->n_cl);
     sum += sum_all_quadratic_derivative_weights_rls(vm->decoder->rls, vm->decoder->n_rl);
     
     sum = sqrtf(sum);
     if(sum >= threshold){
         clip_fcls(vm->encoder->fcls, vm->encoder->n_fcl, threshold, sum);
         clip_cls(vm->encoder->cls, vm->encoder->n_cl, threshold, sum);
         clip_rls(vm->encoder->rls, vm->encoder->n_rl, threshold, sum);
         clip_fcls(vm->decoder->fcls, vm->decoder->n_fcl, threshold, sum);
         clip_cls(vm->decoder->cls, vm->decoder->n_cl, threshold, sum);
         clip_rls(vm->decoder->rls, vm->decoder->n_rl, threshold, sum);
     }
}
 
/* This functions clips the derivative weights according to the clipping_gradient formula
  * of residual layers
  * 
  * Input:
  * 
  *             @ rl* rls:= residual layers
  *             @ int n:= the number of residual layers
  *             @ float threshold:= the threshold of the clipping gradient formula
  *             @ float norm:= the ||DL/Dw|| of the entire network
  * 
  * */
void clip_rls(rl** rls, int n, float threshold,float norm){
    int i;
    for(i = 0; i < n; i++){
        clip_cls(rls[i]->cls, rls[i]->n_cl, threshold, norm);
    }
}
 
 
/* This functions clips the derivative weights according to the clipping_gradient formula
  * of convolutional layers
  * 
  * Input:
  * 
  *             @ cl* cls:= convolutional layers
  *             @ int n:= the number of convolutional layers
  *             @ float threshold:= the threshold of the clipping gradient formula
  *             @ float norm:= the ||DL/Dw|| of the entire network
  * 
  * */
void clip_cls(cl** cls, int n, float threshold, float norm){
    int j,k,z;
    for(j = 0; j < n; j++){
        if(cls[j]->convolutional_flag == CONVOLUTION){
            for(k = 0; k < cls[j]->n_kernels; k++){
                for(z = 0; z < cls[j]->channels*cls[j]->kernel_rows*cls[j]->kernel_cols; z++){
                    cls[j]->d_kernels[k][z]*=(threshold)/(norm);
                }
            }
            
            if(cls[j]->normalization_flag == GROUP_NORMALIZATION){
                for(k = 0; k < cls[j]->n_kernels/cls[j]->group_norm_channels; k++){
                    for(z = 0; z < cls[j]->group_norm[k]->vector_dim; z++){
                        cls[j]->group_norm[k]->d_gamma[z]*=(threshold)/(norm);
                    }
                }
            }
        }
    }
    
}


/* This functions clips the derivative weights according to the clipping_gradient formula
  * of fully-connected layers
  * 
  * Input:
  * 
  *             @ fcl* fcls:= fully-connected layers
  *             @ int n:= the number of fully-connected layers
  *             @ float threshold:= the threshold of the clipping gradient formula
  *             @ float norm:= the ||DL/Dw|| of the entire network
  * 
  * */
void clip_fcls(fcl** fcls, int n, float threshold, float norm){
    int i,j;
    for(i = 0; i < n; i++){
        for(j = 0; j < fcls[i]->output*fcls[i]->input; j++){
            fcls[i]->d_weights[j]*=(threshold)/(norm);
        }
        if(fcls[i]->normalization_flag == LAYER_NORMALIZATION)
            clip_bns(&fcls[i]->layer_norm,fcls[i]->output/fcls[i]->n_groups,threshold,norm);
        
    }
    
}

/* This functions clips the derivative weights according to the clipping_gradient formula
  * of lstm layers
  * 
  * Input:
  * 
  *             @ lstm* lstms:= lstm layers
  *             @ int n:= the number of lstm layers
  *             @ float threshold:= the threshold of the clipping gradient formula
  *             @ float norm:= the ||DL/Dw|| of the entire network
  * 
  * */
void clip_lstms(lstm** lstms, int n, float threshold, float norm){
    int i,j;
    for(i = 0; i < n; i++){
        if(lstms[i]->norm_flag == GROUP_NORMALIZATION)
            clip_bns(lstms[i]->bns,lstms[i]->window/lstms[i]->n_grouped_cell,threshold,norm);
        for(j = 0; j < lstms[i]->size*lstms[i]->size; j++){
            lstms[i]->d_w[0][j]*=(threshold)/(norm);
            lstms[i]->d_u[0][j]*=(threshold)/(norm);
            lstms[i]->d_w[1][j]*=(threshold)/(norm);
            lstms[i]->d_u[1][j]*=(threshold)/(norm);
            lstms[i]->d_w[2][j]*=(threshold)/(norm);
            lstms[i]->d_u[2][j]*=(threshold)/(norm);
            lstms[i]->d_w[3][j]*=(threshold)/(norm);
            lstms[i]->d_u[3][j]*=(threshold)/(norm);
        }
    }
    
}

/* This functions clips the derivative weights according to the clipping_gradient formula
  * of batch-normalized layers
  * 
  * Input:
  * 
  *             @ bn** bns:= batch normalized layers
  *             @ int n:= the number of batch normalized layers
  *             @ float threshold:= the threshold of the clipping gradient formula
  *             @ float norm:= the ||DL/Dw|| of the entire network
  * 
  * */
void clip_bns(bn** bns, int n, float threshold, float norm){
    int i,j;
    for(i = 0; i < n; i++){
        for(j = 0; j < bns[i]->vector_dim; j++){
            bns[i]->d_gamma[j]*=(threshold)/(norm);
        }
    }
    
}

/* This functions returns the derivative of the weights of the residual layers in quadratic form
 * returns Sum for all i (DL/Dw_i^2)    
  * 
  * Input:
  * 
  *             @ rl** rls:= residual layers
  *             @ int n:= the number of residual layers
  * 
  * */
float sum_all_quadratic_derivative_weights_rls(rl** rls, int n){
    int i;
    float sum = 0;
    for(i = 0; i < n; i++){
        sum+= sum_all_quadratic_derivative_weights_cls(rls[i]->cls, rls[i]->n_cl);
    }
    return sum;
}

/* This functions returns the derivative of the weights of the convolutional layers in quadratic form
 * returns Sum for all i (DL/Dw_i^2)    
  * 
  * Input:
  * 
  *             @ cl** cls:= convolutional layers
  *             @ int n:= the number of convolutional layers
  * 
  * */
float sum_all_quadratic_derivative_weights_cls(cl** cls, int n){
    int j,k,z;
    float sum = 0,temp;
    for(j = 0; j < n; j++){
        if(cls[j]->convolutional_flag == CONVOLUTION || cls[j]->convolutional_flag == TRANSPOSED_CONVOLUTION){
            for(k = 0; k < cls[j]->n_kernels; k++){
                for(z = 0; z < cls[j]->channels*cls[j]->kernel_rows*cls[j]->kernel_cols; z++){
                    temp = cls[j]->d_kernels[k][z];
                    sum += temp*temp;
                }
            }
            
            if(cls[j]->normalization_flag == GROUP_NORMALIZATION){
                for(k = 0; k < cls[j]->n_kernels/cls[j]->group_norm_channels; k++){
                    for(z = 0; z < cls[j]->group_norm[k]->vector_dim; z++){
                        temp = cls[j]->group_norm[k]->d_gamma[z];
                        sum += temp*temp;
                    }
                }
            }
        }
    }
    return sum;
}


/* This functions returns the derivative of the weights of the fully-connected layers in quadratic form
 * returns Sum for all i (DL/Dw_i^2)    
  * 
  * Input:
  * 
  *             @ fcl** fcls:= fully-connected layers
  *             @ int n:= the number of fully-connected layers
  * 
  * */
float sum_all_quadratic_derivative_weights_fcls(fcl** fcls, int n){
    int i,j;
    float sum = 0,temp;
    for(i = 0; i < n; i++){
        if(fcls[i]->feed_forward_flag != ONLY_DROPOUT){
            for(j = 0; j < fcls[i]->output*fcls[i]->input; j++){
                temp = fcls[i]->d_weights[j];
                sum += temp*temp;
            }
        }
        if(fcls[i]->normalization_flag == LAYER_NORMALIZATION)
            sum += sum_all_quadratic_derivative_weights_bns(&fcls[i]->layer_norm,1);
    }
    
    return sum;
}

/* This functions returns the derivative of the weights of the lstm layers in quadratic form
 * returns Sum for all i (DL/Dw_i^2)    
  * 
  * Input:
  * 
  *             @ lstms** lstms:= lstm layers
  *             @ int n:= the number of fully-connected layers
  * 
  * */
float sum_all_quadratic_derivative_weights_lstms(lstm** lstms, int n){
    int i,j;
    float sum = 0,temp;
    for(i = 0; i < n; i++){
        if(lstms[i]->norm_flag == GROUP_NORMALIZATION)
            sum += sum_all_quadratic_derivative_weights_bns(lstms[i]->bns,lstms[i]->window/lstms[i]->n_grouped_cell);
        
        for(j = 0; j < lstms[i]->size*lstms[i]->size; j++){
            temp = lstms[i]->d_w[0][j];
            sum += temp*temp;
            temp = lstms[i]->d_u[0][j];
            sum += temp*temp;
            temp = lstms[i]->d_w[1][j];
            sum += temp*temp;
            temp = lstms[i]->d_u[1][j];
            sum += temp*temp;
            temp = lstms[i]->d_w[2][j];
            sum += temp*temp;
            temp = lstms[i]->d_u[2][j];
            sum += temp*temp;
            temp = lstms[i]->d_w[3][j];
            sum += temp*temp;
            temp = lstms[i]->d_u[3][j];
            sum += temp*temp;
        }
    }
    
    return sum;
}

/* This functions returns the derivative of the weights of the batch_normalized layers in quadratic form
 * returns Sum for all i (DL/Dw_i^2)    
  * 
  * Input:
  * 
  *             @ bn** bns:= batch_normalized layers
  *             @ int n:= the number of batch normalized layers
  * 
  * */
float sum_all_quadratic_derivative_weights_bns(bn** bns, int n){
    int i,j;
    float sum = 0,temp;
    for(i = 0; i < n; i++){
        for(j = 0; j < bns[i]->vector_dim; j++){
            temp = bns[i]->d_gamma[j];
            sum += temp*temp;
        }
    }
    
    return sum;
}
