#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

int main( int argc, char *argv[])
{
  int myrank, size,i,j,count=0;
  int data = atoi(argv[1]);
  int no_of_double = (data * 1024) /sizeof(double);
  double buf[no_of_double],maxbuf[no_of_double];

  int no_of_groups = atoi(argv[2]);
  int nodes_per_group = atoi(argv[3]);
  int cores = atoi(argv[4]);
  int flag = atoi(argv[5]);

  MPI_Init(&argc, &argv);
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &size );

  MPI_Comm node_comm;
  MPI_Comm group_comm;
  MPI_Comm multi_group_comm;
  int new_node_rank,new_group_rank,new_multi_group_rank,node_size,group_size,multi_group_size;
  double sum_time = 0, sTime, eTime, time,maxTime,avg_time;

  MPI_Comm_split(MPI_COMM_WORLD, myrank/cores, myrank, &node_comm);
  MPI_Comm_rank (node_comm, &new_node_rank);
  MPI_Comm_size(node_comm, &node_size);
  
  if(new_node_rank == 0){
     MPI_Comm_split(MPI_COMM_WORLD, myrank/(cores*nodes_per_group), myrank, &group_comm);
  }
  else{
     MPI_Comm_split(MPI_COMM_WORLD, -1, myrank, &group_comm);
  }

  MPI_Comm_rank (group_comm, &new_group_rank);
  MPI_Comm_size(group_comm, &group_size);
  
  if(new_node_rank == 0 && new_group_rank == 0){
     MPI_Comm_split(MPI_COMM_WORLD, 0, myrank, &multi_group_comm);
  }else{
     MPI_Comm_split(MPI_COMM_WORLD, -1, myrank, &multi_group_comm);
  }

  MPI_Comm_rank (multi_group_comm, &new_multi_group_rank);
  MPI_Comm_size(multi_group_comm, &multi_group_size);   

  /************************************************************************/
  // Default MPI Bcast
if(flag==1){
	for (int i=0; i<no_of_double; i++)
      buf[i] = myrank + i*i;


  // has to be called by all processes
  for(int k = 0;k<5;k++){
    sTime = MPI_Wtime();

    MPI_Bcast(buf, no_of_double, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    eTime = MPI_Wtime();
    time=eTime-sTime;
    maxTime;
    MPI_Reduce (&time, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if(myrank == 0)
      sum_time = sum_time + maxTime;
  }
  if(myrank==0){
    avg_time = sum_time /5;
    //printf("%lf\n",avg_time);
    FILE* f = fopen("output_Bcast.txt","a");
    if(f==NULL){
      printf("failed to open file\n");
      exit(1);
    }else{
      printf("1,%d,%d,%d,0,%lf\n",data,no_of_groups*nodes_per_group,cores,avg_time);
      fprintf(f,"1,%d,%d,%d,0,%lf\n",data,no_of_groups*nodes_per_group,cores,avg_time);
    }
    fclose(f);
  }

  // simple check for correctness
  //printf ("%d %lf %lf %lf\n", myrank, eTime - sTime, buf[0], buf[1]);

  // Optimized MPI Bcast

  for (int i=0; i<no_of_double; i++)
    buf[i] = myrank + i*i;
  if(myrank == 0)
    sum_time = 0;
  for(int k = 0;k<5;k++){
  sTime = MPI_Wtime();
  if(new_node_rank == 0 && new_group_rank == 0)
    MPI_Bcast(buf, no_of_double, MPI_DOUBLE, 0,multi_group_comm);
  if(new_node_rank == 0)
    MPI_Bcast(buf, no_of_double, MPI_DOUBLE, 0,group_comm);
  MPI_Bcast(buf, no_of_double, MPI_DOUBLE, 0,node_comm);

  eTime = MPI_Wtime();
    time=eTime-sTime;
    MPI_Reduce (&time, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if(myrank == 0)
      sum_time = sum_time + maxTime;
  }
  
  if(myrank==0){
    avg_time = sum_time/5;
    //printf("%lf\n",avg_time);
    FILE* f = fopen("output_Bcast.txt","a");
    if(f==NULL){
      printf("failed to open file\n");
      exit(1);
    }else{
      printf("1,%d,%d,%d,1,%lf\n",data,no_of_groups*nodes_per_group,cores,avg_time);
      fprintf(f,"1,%d,%d,%d,1,%lf\n",data,no_of_groups*nodes_per_group,cores,avg_time);
    }
    fclose(f);
  }
  // simple check for correctness
  //printf ("Optimized %d %lf %lf %lf\n", myrank, eTime - sTime, buf[0], buf[1]);

  MPI_Finalize();
  return 0;
}

MPI_Barrier (MPI_COMM_WORLD);

/************************************************************************/
  // Default MPI Reduce
if(flag==2){
  for (int i=0; i<no_of_double; i++)
    buf[i] = myrank + i*i;

  if(myrank == 0)
    sum_time = 0;
  for(int k = 0;k<5;k++){
    sTime = MPI_Wtime();
    MPI_Reduce(&buf, &maxbuf, no_of_double, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    eTime=MPI_Wtime();
    time=eTime-sTime;
    MPI_Reduce (&time, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  if(myrank == 0)
      sum_time = sum_time + maxTime;
  }
  if(myrank==0){
    avg_time = sum_time/5;
    //printf("%lf\n",avg_time);
    FILE* f = fopen("output_Reduce.txt","a");
    if(f==NULL){
      printf("failed to open file\n");
      exit(1);
    }else{
      printf("2,%d,%d,%d,0,%lf\n",data,no_of_groups*nodes_per_group,cores,avg_time);
      fprintf(f,"2,%d,%d,%d,0,%lf\n",data,no_of_groups*nodes_per_group,cores,avg_time);
    }
    fclose(f);
  }
  //Optimized MPI Reduce

  for (int i=0; i<no_of_double; i++)
        buf[i] = myrank + i*i;

  if(myrank == 0)
    sum_time = 0;
  for(int k = 0;k<5;k++){
    sTime = MPI_Wtime();

    MPI_Reduce(&buf, &maxbuf, no_of_double, MPI_DOUBLE, MPI_SUM, 0, node_comm);
    if(new_node_rank == 0)
      MPI_Reduce(&buf, &maxbuf, no_of_double, MPI_DOUBLE, MPI_SUM, 0, group_comm);
    if(new_node_rank == 0 && new_group_rank == 0)
      MPI_Reduce(&buf, &maxbuf, no_of_double, MPI_DOUBLE, MPI_SUM, 0, multi_group_comm);

    eTime=MPI_Wtime();
    time=eTime-sTime;
    MPI_Reduce (&time, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  if(myrank == 0)
    sum_time = sum_time + maxTime;
  }
  if(myrank==0){
    avg_time = sum_time/5;
    //printf("%lf\n",avg_time);
    FILE* f = fopen("output_Reduce.txt","a");
    if(f==NULL){
      printf("failed to open file\n");
      exit(1);
    }else{
      printf("2,%d,%d,%d,1,%lf\n",data,no_of_groups*nodes_per_group,cores,avg_time);
      fprintf(f,"2,%d,%d,%d,1,%lf\n",data,no_of_groups*nodes_per_group,cores,avg_time);
    }
    fclose(f);
  }
  MPI_Finalize();
  return 0;
}
MPI_Barrier (MPI_COMM_WORLD);
/************************************************************************/
  // Default MPI Gather
if (flag==3){
  for (int i=0; i<no_of_double; i++)
        buf[i] = myrank + i*i;

  double *recvarr = (double*) malloc(size*no_of_double* sizeof(double));
  if(myrank == 0)
    sum_time = 0;
  for(int k = 0;k<5;k++){
    sTime = MPI_Wtime();
    MPI_Gather(buf,no_of_double, MPI_DOUBLE, recvarr, no_of_double, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    eTime=MPI_Wtime();
    time=eTime-sTime;
    MPI_Reduce (&time, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if(myrank == 0)
      sum_time = sum_time + maxTime;
  }
  if(myrank==0){
    avg_time = sum_time/5;
    //printf("%lf\n",avg_time);
    FILE* f = fopen("output_Gather.txt","a");
    if(f==NULL){
      printf("failed to open file\n");
      exit(1);
    }else{
      printf("3,%d,%d,%d,0,%lf\n",data,no_of_groups*nodes_per_group,cores,avg_time);
      fprintf(f,"3,%d,%d,%d,0,%lf\n",data,no_of_groups*nodes_per_group,cores,avg_time);
    }
    fclose(f);
  }

  MPI_Barrier (MPI_COMM_WORLD);
  // Optimized MPI Gather

  for (int i=0; i<no_of_double; i++)
        buf[i] = myrank + i*i;

  double *node_recvarr=(double*) malloc(node_size*no_of_double* sizeof(double));
  double *group_recvarr=(double*) malloc(group_size*node_size*no_of_double* sizeof(double));
  double *multi_group_recvarr=(double*) malloc(multi_group_size*group_size*node_size*no_of_double* sizeof(double));
  if(myrank == 0)
    sum_time = 0;
  for(int k = 0;k<5;k++){
    sTime = MPI_Wtime();
    MPI_Gather(buf,no_of_double, MPI_DOUBLE, node_recvarr, no_of_double, MPI_DOUBLE, 0, node_comm);
    if(new_node_rank == 0)
      MPI_Gather(node_recvarr,node_size*no_of_double, MPI_DOUBLE,group_recvarr, node_size*no_of_double, MPI_DOUBLE, 0, group_comm);
    if(new_node_rank == 0 && new_group_rank == 0)
      MPI_Gather(group_recvarr,node_size*no_of_double*group_size, MPI_DOUBLE,multi_group_recvarr, node_size*no_of_double*group_size, MPI_DOUBLE, 0, multi_group_comm);
    eTime=MPI_Wtime();
    time=eTime-sTime;
    MPI_Reduce (&time, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  if(myrank == 0)
      sum_time = sum_time + maxTime;
  }
  if(myrank==0){
    avg_time = sum_time/5;
    //printf("%lf\n",avg_time);
    FILE* f = fopen("output_Gather.txt","a");
    if(f==NULL){
      printf("failed to open file\n");
      exit(1);
    }else{
      printf("3,%d,%d,%d,1,%lf\n",data,no_of_groups*nodes_per_group,cores,avg_time);
      fprintf(f,"3,%d,%d,%d,1,%lf\n",data,no_of_groups*nodes_per_group,cores,avg_time);
    }
    fclose(f);
  }

  MPI_Finalize();
  return 0;
}
MPI_Barrier (MPI_COMM_WORLD);
/************************************************************************/
//MPI alltoallv
no_of_double = no_of_double/size;

int sendcount = no_of_double*size;
     double *send_all;
     send_all = (double *)malloc(sendcount * sizeof(double));
     for (i = 0;i<sendcount;i++){
          send_all[i] = myrank + i*i;
     }
     double *receive_all;
     receive_all = (double *)malloc(sendcount * sizeof(double));
     double *receive_all_opt;
     receive_all_opt = (double *)malloc(sendcount * sizeof(double));
  if(myrank == 0)
    sum_time = 0;
  int *sendcounts;
  int *displs;
  sendcounts = (int *)malloc(size * sizeof(int));
  displs = (int *)malloc(size * sizeof(int));
  for(int k = 0;k<5;k++){
    // default
     sTime = MPI_Wtime();
     for(i = 0;i<size;i++){
        sendcounts[i] = no_of_double;
        displs[i] = no_of_double * i;
     }

     MPI_Alltoallv(send_all,sendcounts,displs,MPI_DOUBLE,receive_all,sendcounts,displs,MPI_DOUBLE,MPI_COMM_WORLD);

     eTime = MPI_Wtime();    
     time = eTime - sTime;
     MPI_Reduce (&time, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if(myrank == 0)
      sum_time = sum_time + maxTime;
  }
  if(myrank==0){
    avg_time = sum_time/5;
    //printf("%lf\n",avg_time);
    FILE* f = fopen("output_AlltoAllv.txt","a");
    if(f==NULL){
      printf("failed to open file\n");
      exit(1);
    }else{
      printf("4,%d,%d,%d,0,%lf\n",data,no_of_groups*nodes_per_group,cores,avg_time);
      fprintf(f,"4,%d,%d,%d,0,%lf\n",data,no_of_groups*nodes_per_group,cores,avg_time);
    }
    fclose(f);
  }

  MPI_Barrier (MPI_COMM_WORLD);

  // optimized 

  if(myrank == 0)
    sum_time = 0;
  for(int k = 0;k<5;k++){
    sTime = MPI_Wtime();
      for(i = 0;i<size;i++){
        sendcounts[i] = no_of_double;
        displs[i] = i * no_of_double;
      }
      for(i = 0;i<size;i++){
        MPI_Scatterv(send_all, sendcounts,displs,MPI_DOUBLE,receive_all_opt+ (no_of_double * i),no_of_double,MPI_DOUBLE,i,MPI_COMM_WORLD);
      }
    eTime = MPI_Wtime();    
    time = eTime - sTime;
    MPI_Reduce (&time, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if(myrank == 0)
      sum_time = sum_time + maxTime;
  }
  if(myrank==0){
    avg_time = sum_time/5;
    //printf("%lf\n",avg_time);
    FILE* f = fopen("output_AlltoAllv.txt","a");
    if(f==NULL){
      printf("failed to open file\n");
      exit(1);
    }else{
      printf("4,%d,%d,%d,1,%lf\n",data,no_of_groups*nodes_per_group,cores,avg_time);
      fprintf(f,"4,%d,%d,%d,1,%lf\n",data,no_of_groups*nodes_per_group,cores,avg_time);
    }
    fclose(f);
  }

  MPI_Finalize();
  return 0;    
}

