// Generated CUDA kernel by BulkCompiler
// Loop bound: 5000000
#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>

__global__ void bulkKernel(int *a, int *b, int *c, int N) {
    int i_L1 = blockIdx.x * blockDim.x + threadIdx.x;
    if (i_L1 < N) {
        // TODO: replace with actual loop body
        c[i_L1] = a[i_L1] + b[i_L1];
    }
}

int main() {
    const int N = 5000000;

    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));

    for (int i = 0; i < N; i++) { a[i] = i; b[i] = i * 2; }

    int *d_a;
    int *d_b;
    int *d_c;

    cudaMalloc(&d_a, N * sizeof(int));
    cudaMalloc(&d_b, N * sizeof(int));
    cudaMalloc(&d_c, N * sizeof(int));

    cudaMemcpy(d_a, a, N * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_b, b, N * sizeof(int), cudaMemcpyHostToDevice);

    int threads = 256;
    int blocks  = (N + threads - 1) / threads;
    bulkKernel<<<blocks, threads>>>(d_a, d_b, d_c, N);
    cudaDeviceSynchronize();

    cudaMemcpy(c, d_c, N * sizeof(int), cudaMemcpyDeviceToHost);

    printf("c[10] = %d\n", c[10]);

    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_c);
    free(a); free(b); free(c);
    return 0;
}
