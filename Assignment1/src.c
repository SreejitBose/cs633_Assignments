#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>


int main( int argc, char *argv[])
{
  /*
   * datapoints are total matrix size and so no of rows or column is sqrt of datapoints
   * Second argument is no_of_steps.
   */
  int datapoints = atoi(argv[1]);
  int time_step = atoi(argv[2]);
  /*****************************************************/
  int myrank, size, len,proc_x,proc_y,i,j,k;
  double sTime, eTime, time,maxTime;
  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &size );

  if (myrank == 0) {
    FILE* f = fopen("output.txt","a");
    if(f==NULL){
      printf("failed to open file\n");
      exit(1);
    }else{
      fprintf(f,"\n%d,", datapoints);
      fprintf(f,"%d,", size);
    }
    fclose(f);
  }

  /*
  * size here is no of process in commuication group
  * I assumed proc_dim is no of row for the process matrix
  */
  int proc_dim = (int) sqrt((double)size);


  double **data,**temp;
    data = (double**)malloc(datapoints * sizeof(double*));
    temp = (double**)malloc(datapoints * sizeof(double*));
    for (i=0; i<datapoints; i++) {
         data[i] = (double *)malloc(datapoints * sizeof(double));
         temp[i] = (double *)malloc(datapoints * sizeof(double));
    }


  for(i=0;i<datapoints;i++){
    for(j=0;j<datapoints;j++){
      data[i][j]=rand();
    }
  }

  proc_x=myrank%proc_dim;
  proc_y=myrank/proc_dim;
  double left_arr[datapoints],right_arr[datapoints],up_arr[datapoints],down_arr[datapoints];
  int left=proc_x-1,right=proc_x+1,up=proc_y-1,down=proc_y+1;
  for(i=0;i<datapoints;i++){
    left_arr[i]=0;
    right_arr[i]=0;
    up_arr[i]=0;
    down_arr[i]=0;
  }

  int left_proc = myrank - 1;
  int right_proc = myrank + 1;
  int up_proc = myrank - proc_dim;
  int down_proc = myrank + proc_dim;
    
  /********************************************************************************************************
   * ***********************************************Blocking***********************************************
   * *****************************************************************************************************/
  sTime = MPI_Wtime();
  for(k=0;k<time_step;k++){                    //for each time step
    
    //Computation
    for(i=0;i<datapoints;i++){
      for(j=0;j<datapoints;j++){
        int l,r,u,d;
        if(j==0)
          l=left_arr[i];
        else
          l=data[i][j-1];
        
        if(j==datapoints-1)
          r=right_arr[i];
        else
          r=data[i][j+1];
        
        if(i==0)
          u=up_arr[j];
        else
          u=data[i-1][j];
        
        if(i==datapoints-1)
          d=down_arr[j];
        else
          d=data[i+1][j];
        
        int result=(l+r+u+d)/4;
        temp[i][j]=result;
      }
    }
    for(i=0;i<datapoints;i++){
      for(j=0;j<datapoints;j++){
        data[i][j]=temp[i][j];
      }
    }

    //Communication
    /*********************************************SEND***********************************/
    if(left>=0){
      //communicate to left
      for(i=0;i<datapoints;i++)
        MPI_Send(&data[i][0], 1, MPI_DOUBLE, left_proc, 1, MPI_COMM_WORLD);
    }
    if (right<proc_dim){
      //communicate to right
      for(i=0;i<datapoints;i++)
        MPI_Send(&data[i][datapoints-1], 1, MPI_DOUBLE, right_proc, 2, MPI_COMM_WORLD);
    }
    if (up>=0){
      //communicate to up
      for(i=0;i<datapoints;i++)
        MPI_Send(&data[0][i], 1, MPI_DOUBLE, up_proc, 3, MPI_COMM_WORLD);
    }
    if (down<proc_dim){
      //communicate to down
      for(i=0;i<datapoints;i++)
        MPI_Send(&data[datapoints-1][i], 1, MPI_DOUBLE, down_proc, 4, MPI_COMM_WORLD);
    }
    /******************************** Receive ******************************************/
    if(left>=0){
      //communicate to left
      for(i=0;i<datapoints;i++)
        MPI_Recv(&left_arr[i], 1, MPI_DOUBLE, left_proc, 2, MPI_COMM_WORLD,&status);
    }
    if (right<proc_dim){
      //communicate to right
      for(i=0;i<datapoints;i++)
        MPI_Recv(&right_arr[i], 1, MPI_DOUBLE, right_proc, 1, MPI_COMM_WORLD,&status);
    }
    if (up>=0){
      //communicate to up
      for(i=0;i<datapoints;i++)
        MPI_Recv(&up_arr[i], 1, MPI_DOUBLE, up_proc, 4, MPI_COMM_WORLD,&status);
    }
    if (down<proc_dim){
      //communicate to down
      for(i=0;i<datapoints;i++)
        MPI_Recv(&down_arr[i], 1, MPI_DOUBLE, down_proc, 3, MPI_COMM_WORLD,&status);
    }
    
  }
 
  eTime = MPI_Wtime();    
  time = eTime - sTime;
  MPI_Reduce (&time, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  //if (!myrank) printf ("Direct:%lf\n", maxTime);
  if (myrank == 0) {
    FILE* f = fopen("output.txt","a");
    if(f==NULL){
      printf("failed to open file: permission issue ?\n");
      exit(1);
    }else{
      printf ("%lf\n ", maxTime);
      fprintf(f,"%lf,", maxTime);
    }
    fclose(f);
  }
      
  /**********************************************************************************
  ******************************* Part 2: Using Packed ******************************
  **********************************************************************************/

 double buffer_p_un[datapoints];

 sTime = MPI_Wtime();
  for(k=0;k<time_step;k++){                    //for each time step
    //Computation
    for(i=0;i<datapoints;i++){
      for(j=0;j<datapoints;j++){
        int l,r,u,d;
        if(j==0)
          l=left_arr[i];
        else
          l=data[i][j-1];
        
        if(j==datapoints-1)
          r=right_arr[i];
        else
          r=data[i][j+1];
        
        if(i==0)
          u=up_arr[j];
        else
          u=data[i-1][j];
        
        if(i==datapoints-1)
          d=down_arr[j];
        else
          d=data[i+1][j];
        
        int result=(l+r+u+d)/4;
        temp[i][j]=result;
      }
    }
    for(i=0;i<datapoints;i++){
      for(j=0;j<datapoints;j++){
        data[i][j]=temp[i][j];
      }
    }

    //Communication
    /***********************************SEND***********************************/
    int position = 0;
    if(left>=0){
      //communicate to left
      for(i=0;i<datapoints;i++)
        MPI_Pack(&data[i][0], 1, MPI_DOUBLE, buffer_p_un, datapoints*8, &position, MPI_COMM_WORLD);
      MPI_Send(&buffer_p_un, position, MPI_PACKED, left_proc, 1, MPI_COMM_WORLD);
    }
    position = 0;
    if (right<proc_dim){
      //communicate to right
      for(i=0;i<datapoints;i++)
        MPI_Pack(&data[i][datapoints-1], 1, MPI_DOUBLE, buffer_p_un, datapoints*8, &position, MPI_COMM_WORLD);
      MPI_Send(&buffer_p_un, position, MPI_PACKED, right_proc, 2, MPI_COMM_WORLD);
    }
    position = 0;
    if (up>=0){
      //communicate to up
      for(i=0;i<datapoints;i++)
        MPI_Pack (&data[0][i], 1, MPI_DOUBLE, buffer_p_un, datapoints*8, &position, MPI_COMM_WORLD);
      MPI_Send(&buffer_p_un, position, MPI_PACKED, up_proc, 3, MPI_COMM_WORLD);
    }
    position = 0;
    if (down<proc_dim){
      //communicate to down
      for(i=0;i<datapoints;i++)
        MPI_Pack (&data[datapoints-1][i], 1, MPI_DOUBLE, buffer_p_un, datapoints*8, &position, MPI_COMM_WORLD);
      MPI_Send(&buffer_p_un, position, MPI_PACKED, down_proc, 4, MPI_COMM_WORLD);
    }
    /******************************** Receive ******************************************/
    position = 0;
    if(left>=0){
      //communicate to left
      MPI_Recv(&buffer_p_un, datapoints*8, MPI_PACKED, left_proc, 2, MPI_COMM_WORLD,&status);
      for(i=0;i<datapoints;i++)
        MPI_Unpack(&buffer_p_un,datapoints*8,&position,&left_arr[i],1,MPI_DOUBLE,MPI_COMM_WORLD);
    }
    position = 0;
    if (right<proc_dim){
      //communicate to right
      MPI_Recv(&buffer_p_un, datapoints*8, MPI_PACKED, right_proc, 1, MPI_COMM_WORLD,&status);
      for(i=0;i<datapoints;i++)
        MPI_Unpack(&buffer_p_un,datapoints*8,&position,&right_arr[i],1,MPI_DOUBLE,MPI_COMM_WORLD);
    }
    position = 0;
    if (up>=0){
      //communicate to up
      MPI_Recv(&buffer_p_un, datapoints*8, MPI_PACKED, up_proc, 4, MPI_COMM_WORLD,&status);
      for(i=0;i<datapoints;i++)
        MPI_Unpack(&buffer_p_un,datapoints*8,&position,&up_arr[i],1,MPI_DOUBLE,MPI_COMM_WORLD);
    }
    position = 0;
    if (down<proc_dim){
      //communicate to down
      MPI_Recv(&buffer_p_un, datapoints*8, MPI_PACKED, down_proc, 3, MPI_COMM_WORLD,&status);
      for(i=0;i<datapoints;i++)
        MPI_Unpack(&buffer_p_un,datapoints*8,&position,&down_arr[i],1,MPI_DOUBLE,MPI_COMM_WORLD);
    }
  }
 
  eTime = MPI_Wtime();    
  time = eTime - sTime;
  MPI_Reduce (&time, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  //if (!myrank) printf ("Packed %lf\n", maxTime);
  if (!myrank) {
    FILE* f = fopen("output.txt","a");
    if(f==NULL){
      printf("failed to open file: permission issue ?\n");
      exit(1);
    }else{
      printf ("%lf\n", maxTime);
      fprintf(f,"%lf,", maxTime);
    }
    fclose(f);
  }
  
  /**********************************************************************************
  ********************************* Part 3: Vector **********************************
  **********************************************************************************/

 	MPI_Datatype newtype,rowtype;   
  MPI_Type_vector(datapoints,1,datapoints,MPI_DOUBLE,&newtype);
  MPI_Type_commit(&newtype);
  MPI_Type_vector(1,datapoints,1,MPI_DOUBLE,&rowtype);
  MPI_Type_commit(&rowtype);

  sTime = MPI_Wtime();
  for(k=0;k<time_step;k++){                    //for each time step
                        //Computation
  for(i=0;i<datapoints;i++){
    for(j=0;j<datapoints;j++){
      int l,r,u,d;
      if(j==0)
        l=left_arr[i];
      else
        l=data[i][j-1];
      
      if(j==datapoints-1)
        r=right_arr[i];
      else
        r=data[i][j+1];
      
      if(i==0)
        u=up_arr[j];
      else
        u=data[i-1][j];
      
      if(i==datapoints-1)
        d=down_arr[j];
      else
        d=data[i+1][j];
      
      int result=(l+r+u+d)/4;
      temp[i][j]=result;
    }
  }
    for(i=0;i<datapoints;i++){
      for(j=0;j<datapoints;j++){
        data[i][j]=temp[i][j];
      }
    }

    //Communication
    /***********************************SEND***********************************/
    if(left>=0){
      //communicate to left
      MPI_Send(&data[0][0], 1, newtype, left_proc, 1, MPI_COMM_WORLD);
    }
    if (right<proc_dim){
      //communicate to right
      MPI_Send(&data[0][datapoints-1], 1, newtype, right_proc, 2, MPI_COMM_WORLD);
    }
    if (up>=0){
      //communicate to up
      MPI_Send(&data[0][0], 1, rowtype, up_proc, 3, MPI_COMM_WORLD);
    }
    if (down<proc_dim){
      //communicate to down
      MPI_Send(&data[datapoints-1][0], 1, rowtype, down_proc, 4, MPI_COMM_WORLD);
    }

    /******************************** Receive ******************************************/
    if(left>=0){
      //communicate to left
      MPI_Recv(left_arr,datapoints, MPI_DOUBLE, left_proc, 2, MPI_COMM_WORLD,&status);
    }
    if (right<proc_dim){
      //communicate to right
      MPI_Recv(right_arr, datapoints, MPI_DOUBLE, right_proc, 1, MPI_COMM_WORLD,&status);
    }
    if (up>=0){
      //communicate to up
      MPI_Recv(up_arr, datapoints, MPI_DOUBLE, up_proc, 4, MPI_COMM_WORLD,&status);
    }
    if (down<proc_dim){
      //communicate to down
      MPI_Recv(down_arr, datapoints, MPI_DOUBLE, down_proc, 3, MPI_COMM_WORLD,&status);
    }
  }

  eTime = MPI_Wtime();    
  time = eTime - sTime;
  MPI_Reduce (&time, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  //if (!myrank) printf ("Vector %lf\n", maxTime);
  if (!myrank) {
    FILE* f = fopen("output.txt","a");
    if(f==NULL){
      printf("failed to open file: permission issue ?\n");
      exit(1);
    }else{
      printf ("%lf\n", maxTime);
      fprintf(f,"%lf", maxTime);
    }
    fclose(f);
  }

  MPI_Finalize();
  return 0;
}
