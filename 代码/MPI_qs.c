#include <mpi.h>
#include<stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>



int pow2(int num)
{
	int result=1<<num;
	return result;
}

/*
    将给定的数组分成两个分区
    -低于枢轴
    -高于枢轴
    并返回数组中的Pivot索引
*/
int partition(int *arr, int low, int high){
    int pivot = arr[high];
    int i = (low - 1);
    int j,temp;
    for (j=low;j<=high-1;j++){
	if(arr[j] < pivot){
	     i++;
             temp=arr[i];  
             arr[i]=arr[j];
             arr[j]=temp;	
	}
    }
    temp=arr[i+1];  
    arr[i+1]=arr[high];
    arr[high]=temp; 
    return (i+1);
}

/*
    霍尔分划——起始枢轴是中点
*/
int hoare_partition(int *arr, int low, int high){
    int middle = floor((low+high)/2);
    int pivot = arr[middle];
    int j,temp;
    // 将枢轴移到最后
    temp=arr[middle];  
    arr[middle]=arr[high];
    arr[high]=temp;

    int i = (low - 1);
    for (j=low;j<=high-1;j++){
        if(arr[j] < pivot){
            i++;
            temp=arr[i];  
            arr[i]=arr[j];
            arr[j]=temp;	
        }
    }
    temp=arr[i+1];  
    arr[i+1]=arr[high];
    arr[high]=temp; 

    return (i+1);
}

/*
    顺序快速排序算法
*/
void quicksort(int *number,int first,int last){
    if(first<last){
        int pivot_index = partition(number, first, last);
        quicksort(number,first,pivot_index-1);
        quicksort(number,pivot_index+1,last);
    }
}

/*
    将子数组分发给正确的集群
*/
int quicksort_recursive(int* arr, int arrSize, int currProcRank, int maxRank, int rankIndex) {
    MPI_Status status;

    // 计算将发送的线程的ID
    int shareProc = currProcRank + pow2(rankIndex);
    // rankIndex+1
    rankIndex++;

    // 如果没有可用的线程了，那就开始顺序排序并返回
    if (shareProc > maxRank) {
        MPI_Barrier(MPI_COMM_WORLD);
	    quicksort(arr, 0, arrSize-1 );
        return 0;
    }
    // 将数组按枢轴分成两个子序列
    int j = 0;
    int pivotIndex;
    pivotIndex = hoare_partition(arr, j, arrSize-1 );

    // 根据大小发送数组(总是发送较小的部分)
    // 对剩余的数组进行排序，
    // 从子进程中接受排好的子序列
    if (pivotIndex <= arrSize - pivotIndex) {
        MPI_Send(arr, pivotIndex , MPI_INT, shareProc, pivotIndex, MPI_COMM_WORLD);
	    quicksort_recursive((arr + pivotIndex+1), (arrSize - pivotIndex-1 ), currProcRank, maxRank, rankIndex); 
        MPI_Recv(arr, pivotIndex , MPI_INT, shareProc, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    }
    else {
        MPI_Send((arr + pivotIndex+1), arrSize - pivotIndex-1, MPI_INT, shareProc, pivotIndex + 1, MPI_COMM_WORLD);
        quicksort_recursive(arr, (pivotIndex), currProcRank, maxRank, rankIndex);
        MPI_Recv((arr + pivotIndex+1), arrSize - pivotIndex-1, MPI_INT, shareProc, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    }
}


int main(int argc, char *argv[]) {
    int SIZE = atoi(argv[1]);
    int *unsorted_array;
    int array_size = SIZE;
    int size, rank;

    
    // MPI初始化
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    char processor_name[MPI_MAX_PROCESSOR_NAME];
	int name_len;
	//看一下线程运行的节点名字
    MPI_Get_processor_name(processor_name, &name_len);
	printf("node is %s\n", processor_name);

    if(rank==0){
        unsorted_array = (int*)malloc(SIZE * sizeof(int));
        srand(time(NULL) + rand());   //随机数种子
        for (int i = 0; i < SIZE; i++)
            unsorted_array[i] = (int)rand()%SIZE;   //获取n个随机整数
	}

    // 计算每个进程是哪一层的
    int rankPower = 0;
    while (pow2(rankPower) <= rank){
        rankPower++;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double start_timer, finish_timer;
    if (rank == 0) {
	    start_timer = MPI_Wtime();
        quicksort_recursive(unsorted_array, array_size, rank, size - 1, rankPower);    
    }else{ 
        MPI_Status status;
        int subarray_size;
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &subarray_size);
	    int source_process = status.MPI_SOURCE;     
        int subarray[subarray_size];
        MPI_Recv(subarray, subarray_size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        quicksort_recursive(subarray, subarray_size, rank, size - 1, rankPower);
        MPI_Send(subarray, subarray_size, MPI_INT, source_process, 0, MPI_COMM_WORLD);
    };
    
    if(rank==0){
        finish_timer = MPI_Wtime();
	    printf("Total time for %d Clusters : %lf ms \n",size, (finish_timer-start_timer)*1000);
        //正确性检查
        printf("Checking.. \n");
        bool error = false;
        int i=0;
        for(i=0;i<SIZE-1;i++) { 
            if (unsorted_array[i] > unsorted_array[i+1]){
		        error = true;
                printf("error in i=%d \n", i);
            }
        }
        if(error)
            printf("Error..Not sorted correctly\n");
        else
            printf("Correct!\n");        
    }
    MPI_Finalize();
    return 0;
}
