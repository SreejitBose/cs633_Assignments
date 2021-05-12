#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "mpi.h"

int main(int argc, char *argv[])
{
    int myrank,size;
    char *file_name;
    if(argc<2){
	    printf("provide inputfile");
    	return 0;   
    }
else
    file_name=argv[1];
    MPI_Init(&argc, &argv);

    MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    
    float *arrs;
    int row = 0;
    int column = 0;
    double sTime,eTime,time,maxTime;
    if(myrank == 0){
        FILE* fp = fopen(file_name, "r");
        if (!fp){
            printf("Can't open file\n");

            MPI_Finalize();
            return 0;
        }
        char buffer[500];
        while (fgets(buffer,500, fp)) {
            row++;
            if (row == 2){
                char* value = strtok(buffer, ",");
                while (value){
                    value = strtok(NULL, ", ");
                    column++;
                }
            }
        }
        column = column - 2;
        row--;
        arrs=(float*)malloc(row*column*sizeof(float));
        rewind(fp);
        int t_row=0;
        int k=0;
        while (fgets(buffer,500, fp)) {
            int t_column = 0;
            t_row++;
            if (t_row == 1)
                continue;
            char* value = strtok(buffer, ",");
            while (value) {
                if(t_column==0 || t_column==1){
                    t_column++;
                    value = strtok(NULL, ", ");
                    continue;                
                }
                arrs[(t_column-2)*row + k]=atof(value);
                value = strtok(NULL, ", ");
                t_column++;
            }
            k++;
        }
        fclose(fp);
    }
    sTime=MPI_Wtime();
    
    int send_col[size*3];
    if (myrank == 0){
        int cpp = column/size;  //column per process
        int bais = column -( cpp * size ); // column left after evenly dividing processes
        int start[size],end[size];
        for(int i = 0;i<size;i++){
            if(i==0)
                start[i] = 0;
            else
                start[i] = end[i-1] + 1;
            if (i < bais)
                end[i] = start[i] + cpp + 1;
            else
                end[i] = start[i] + cpp;
            int t = i*3; 
            send_col[t] = row;
            send_col[t+1] = column;
            send_col[t+2] = end[i] - start[i]; // Columns/years a node is processing 
        }
    }
    int meta[3];
    MPI_Scatter( send_col , 3 , MPI_INT , &meta , 3 , MPI_INT , 0 , MPI_COMM_WORLD);

    // meta[0] is Rows
    // meta[1] is Column
    // meta[2] is Columns/years a node is processing 

    int sendcounts[size],displs[size];
    displs[0] = 0;
    if(myrank == 0){
        for(int i = 0;i<size;i++){
            sendcounts[i] = send_col[i*3 + 2]*row;
            if(i != 0)
                displs[i] = displs[i-1] + sendcounts[i-1];
        }
    }

    float *recvbuf;
    recvbuf = (float*)malloc(meta[0]*meta[2]*sizeof(float));

    MPI_Scatterv( arrs , sendcounts ,displs, MPI_FLOAT , recvbuf , meta[0]*meta[2] , MPI_FLOAT , 0 , MPI_COMM_WORLD);
    
    float min[meta[2]];
    for(int i =0; i< meta[2];i++){
        min[i] = recvbuf[i*meta[0]];
        for(int j = 1; j< meta[0];j++){
            if(recvbuf[i*meta[0]+j] < min[i]){
                min[i] = recvbuf[i*meta[0]+j];
            }
        }
    }

    float result[meta[1]];
    displs[0] = 0;
    for(int i = 0;i<size;i++){
        sendcounts[i] = send_col[i*3 + 2];
        if(i != 0)
            displs[i] = displs[i-1] + sendcounts[i-1];
    }

    MPI_Gatherv( min , meta[2] , MPI_FLOAT , result , sendcounts , displs , MPI_FLOAT , 0 , MPI_COMM_WORLD);

    float overallmin;
    if(myrank == 0){
        overallmin = result[0];
        for(int i = 0; i< meta[1];i++){
	if (i==meta[1]-1)
	printf("%f",result[i]);	
	else
          printf("%f,",result[i]);
            if(result[i] < overallmin)
                overallmin = result[i];
        }
       printf("\n%f\n",overallmin);
    }
    eTime=MPI_Wtime();
    time=eTime-sTime;
    MPI_Reduce (&time, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if(myrank == 0)
	    printf("%f\n",maxTime);


    MPI_Finalize();
    return 0;
}
