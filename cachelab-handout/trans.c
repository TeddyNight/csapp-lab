/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc1[] = "Transpose submission1";
void transpose_submit1(int M, int N, int A[N][M], int B[M][N])
{
	if(M==61&&N==67){
		//B是3行不冲突
		//A是4行不冲突
		for(int i=0;i<64;i+=4){
			for(int j=0;j<60;j+=4){
				for(int ii=i;ii<i+4;ii++){
					for(int jj=j;jj<j+4;jj++){
						B[jj][ii]=A[ii][jj];
					}
				}
			}
		}
		for(int i=0;i<64;i++) B[60][i]=A[i][60];
		for(int i=64;i<N;i++){
			for(int j=0;j<M;j++){
				B[j][i]=A[i][j];
			}
		}
	}
}

char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
	// s=5,E=1,b=5 32*32 bytes
	if(M==61&&N==67){
		//B是3.82行不冲突
		//A是4.19行不冲突
		//2019
		for(int i=0;i<64;i+=8){
			for(int j=0;j<60/*64*/;j+=8){
				int a,b,c,d,e,f,g,h,t;
				for(int ii=i;ii<i+4;ii++){
					a=A[ii][j];
					b=A[ii][j+1];
					c=A[ii][j+2];
					d=A[ii][j+3];
					//A断掉的块接上来
					if(j==56){
						e=A[ii][j+4];
						f=A[ii+4][j+4];
						B[j+4][ii]=e;
						B[j+4][ii+4]=f;

					}
					e=A[ii+4][j];
					f=A[ii+4][j+1];
					g=A[ii+4][j+2];
					h=A[ii+4][j+3];
					if(j!=0&&ii==0){
						t=A[64][j-1];
						B[j-1][64]=t;
						B[j-1][65]=A[65][j-1];
						B[j-1][66]=A[66][j-1];
					}
					B[j][ii]=a;
					B[j][ii+4]=e;
					//B断掉的块接上来
					if(ii==0){
						t=A[64][j];
						a=A[65][j];
						e=A[66][j];
						B[j][64]=t;
						B[j][65]=a;
						B[j][66]=e;
					}
					B[j+1][ii]=b;
					B[j+1][ii+4]=f;
					//B断掉的块接上来
					if(ii==0){
						t=A[64][j+1];
						a=A[65][j+1];
						e=A[66][j+1];
						B[j+1][64]=t;
						B[j+1][65]=a;
						B[j+1][66]=e;
					}
					B[j+2][ii]=c;
					B[j+2][ii+4]=g;
					//B断掉的块接上来
					if(ii==0){
						t=A[64][j+2];
						a=A[65][j+2];
						e=A[66][j+2];
						B[j+2][64]=t;
						B[j+2][65]=a;
						B[j+2][66]=e;
					}
					B[j+3][ii]=d;
					B[j+3][ii+4]=h;
					//B断掉的块接上来
					if(ii==0&&j==56){
						t=A[64][j+3];
						a=A[65][j+3];
						e=A[66][j+3];
						B[j+3][64]=t;
						B[j+3][65]=a;
						B[j+3][66]=e;
					}

				}
				if(j==56) break;
				for(int ii=i+7;ii>i+3;ii--){
					a=A[ii][j+4];
					b=A[ii][j+5];
					c=A[ii][j+6];
					d=A[ii][j+7];
					e=A[ii-4][j+4];
					f=A[ii-4][j+5];
					g=A[ii-4][j+6];
					h=A[ii-4][j+7];
					B[j+4][ii]=a;
					//B断掉的块接上来
					if(ii-4==0){
						t=A[64][j+3];
						a=A[65][j+3];
						B[j+3][64]=t;
						B[j+3][65]=a;
						B[j+3][66]=A[66][j+3];
					}
					B[j+4][ii-4]=e;
					B[j+5][ii]=b;
					//B断掉的块接上来
					if(ii-4==0){
						t=A[64][j+4];
						a=A[65][j+4];
						e=A[66][j+4];
						B[j+4][64]=t;
						B[j+4][65]=a;
						B[j+4][66]=e;
					}
					B[j+5][ii-4]=f;
					B[j+6][ii]=c;
					//B断掉的块接上来
					if(ii-4==0){
						t=A[64][j+5];
						a=A[65][j+5];
						e=A[66][j+5];
						B[j+5][64]=t;
						B[j+5][65]=a;
						B[j+5][66]=e;
					}
					B[j+6][ii-4]=g;
					B[j+7][ii]=d;
					if(ii-4==0){
						t=A[64][j+6];
						a=A[65][j+6];
						e=A[66][j+6];
						B[j+6][64]=t;
						B[j+6][65]=a;
						B[j+6][66]=e;
					}
					B[j+7][ii-4]=h;
				}
			}
		}
		for(int i=64;i<N;i++) B[60][i]=A[i][60];
	}
	// 64*64: 1258
	else if(M==64&&N==64){
		for(int i=0;i<N;i+=8){
			for(int j=0;j<M;j+=8){
				int a,b,c,d,e,f,g,h;
				for(int ii=i;ii<i+4;ii++){
					a=A[ii][j];
					b=A[ii][j+1];
					c=A[ii][j+2];
					d=A[ii][j+3];
					//ii=i+3，往下跳，A不与B冲突，为下一块保存好一块A，因为两块A冲突，这时候能减少conflict miss
					if(ii!=i+3||i==j){
						//针对i=j对角线，除了最右下角那块，A与B冲突的情况，把A保存到B不冲突的块，这样下面那行开始的时候不会冲突
						if(i==j&&i<N-8){
							//ii-i get the index
							for(int k=0;k<4;k++) B[j+ii-i][i+8+k]=A[ii][j+4+k];
							for(int k=0;k<4;k++) B[j+ii-i][i+12+k]=A[ii+4][j+4+k];
						}
						e=A[ii+4][j];
						f=A[ii+4][j+1];
						g=A[ii+4][j+2];
						h=A[ii+4][j+3];
					}
					else if(ii==i+3){
						e=A[ii][j+4];
						f=A[ii][j+5];
						g=A[ii][j+6];
						h=A[ii][j+7];
					}
					B[j][ii]=a;
					B[j+1][ii]=b;
					B[j+2][ii]=c;
					B[j+3][ii]=d;
					if(ii!=i+3||i==j){
						B[j][ii+4]=e;
						B[j+1][ii+4]=f;
						B[j+2][ii+4]=g;
						B[j+3][ii+4]=h;
					}
					else{
						B[j][ii+4]=A[ii+4][j];
						B[j+1][ii+4]=A[ii+4][j+1];
						B[j+2][ii+4]=A[ii+4][j+2];
						B[j+3][ii+4]=A[ii+4][j+3];
					}
				}
				// 接着算下一行，当不是对角线的情况的时候，能利用上加载上的缓存
				// 并且先选择右下角那块
				// 因为这时候还能利用上最后一次i+3，i+3的4个存到变量里？
				// 虽然只减少一次不命中，但一共减少8*8=64次，在1360的时候这个优化还是很可观
				for(int ii=i+7;ii>i+3;ii--){
					//for i=j 对角线
					if(i==j&&i<N-8){
						//ii-i-4 get the index
						a=B[j+ii-i-4][i+12];
						b=B[j+ii-i-4][i+13];
						c=B[j+ii-i-4][i+14];
						d=B[j+ii-i-4][i+15];
					}
					else{
						//ii start from i+7
						a=A[ii][j+4];
						b=A[ii][j+5];
						c=A[ii][j+6];
						d=A[ii][j+7];
					}
					//for i=j 对角线
					if(i==j&&i<N-8){
						e=B[j+ii-i-4][i+8];
						f=B[j+ii-i-4][i+9];
						g=B[j+ii-i-4][i+10];
						h=B[j+ii-i-4][i+11];
					}
					//针对不在对角线以及最右小角那块
					else if(ii!=i+7||i==j){
						e=A[ii-4][j+4];
						f=A[ii-4][j+5];
						g=A[ii-4][j+6];
						h=A[ii-4][j+7];
					}
					B[j+4][ii]=a;
					B[j+5][ii]=b;
					B[j+6][ii]=c;
					B[j+7][ii]=d;
					B[j+4][ii-4]=e;
					B[j+5][ii-4]=f;
					B[j+6][ii-4]=g;
					B[j+7][ii-4]=h;
				}
			}
		}
	}
	// 32*32
	else {
		for(int i=0;i<N;i+=8){
			for(int j=0;j<M;j+=8){
				// bsize*bsize small matrix
				for(int ii=i;ii<i+8;ii++){
					// conflict miss happens when blocks of B may replace the blocks of A
					// codes below aim to reduce conflict miss
					int a=A[ii][j];
					int b=A[ii][j+1];
					int c=A[ii][j+2];
					int d=A[ii][j+3];
					int e=A[ii][j+4];
					int f=A[ii][j+5];
					int g=A[ii][j+6];
					int h=A[ii][j+7];
					B[j][ii]=a;
					B[j+1][ii]=b;
					B[j+2][ii]=c;
					B[j+3][ii]=d;
					B[j+4][ii]=e;
					B[j+5][ii]=f;
					B[j+6][ii]=g;
					B[j+7][ii]=h;
				}
			}
		}
	}
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 
    //registerTransFunction(transpose_submit1, transpose_submit_desc1); 

    /* Register any additional transpose functions */
    //registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}
