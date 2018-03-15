#include <stdio.h>
#include "mpi.h"
#include <math.h>

int main(int argc, char** argv)  
{
	int cpu_rank;			//name of the cpu
	int n;					//number of X numbers
	int p;					//number of cpu's
	int i;		
	int num;	

	int source;				//indexes for mass communication between cpus
	int target;
	
   	int tag1=50;			//tag for sending n
   	int tag2=60;			//tag for sending numbers of X
   	int tag3=70;			//tag for sending local_sum's
   	int tag4=80;			//tag for sending average m
	int tag5=90;			//tag for sending local_var's
	int tag6=100;			//tag for sending local_max's
	int tag7=110;			//tag for sending local_min's
   	
	int X[100];				//buffer for the all of the X records
   	int X_local[100];		//local buffer for the X records of each cpu
   	
   	MPI_Status status;

   	MPI_Init(&argc, &argv);
   		MPI_Comm_rank(MPI_COMM_WORLD, &cpu_rank);
   		MPI_Comm_size(MPI_COMM_WORLD, &p);
   		
		////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////AVERAGE///////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////
		
		int local_sum=0;		
		int sum=0;
		float m=0;			//average of all X numbers
		
		
		if (cpu_rank == 0)  													//if you are the cpu coordinator
	   	{
			printf("Give me the size of input:\n");
			scanf("%d", &n);
		       
			printf("Type the elements of X array:\n", n); 
			for (i=0; i<n; i++)
				scanf("%d", &X[i]);
		            
			for (target = 1; target < p; target++) 											//send to all the other cpu's
				MPI_Send(&n, 1, MPI_INT, target, tag1, MPI_COMM_WORLD);						//the total number of X numbers
		           
			num = n/p;  
			i=num;
		      
			for (target = 1; target < p; target++)  										//send to all the other cpu's
			{																				//the X numbers that they will work with			
				MPI_Send(&X[i], num, MPI_INT, target, tag2, MPI_COMM_WORLD);	
				i+=num;  
			}
		        
			for (i=0; i<num; i++)
				X_local[i]=X[i]; 	
	    }
		else  																		//if you are a cpu worker
	   	{
			MPI_Recv(&n, 1, MPI_INT, 0, tag1, MPI_COMM_WORLD, &status);				//receive the total number of X numbers
			num = n/p;
			MPI_Recv(&X_local[0], num, MPI_INT, 0, tag2, MPI_COMM_WORLD, &status);	//receive the X numbers that you will work with
		}

		/////////////////////////////////////////////////////////
		///////ALL CPU'S WILL EXECUTE THIS PART//////////////////
		local_sum=0;
		for(i=0; i<num; i++)
			local_sum+=X_local[i];
		/////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////

   		if (cpu_rank != 0)  														//if you are a cpu worker
   		{
			MPI_Send(&local_sum, 1, MPI_INT, 0, tag3, MPI_COMM_WORLD);				//send back the sum of the numbers you worked with	
		}
   		else  																		//if you are the cpu coordinator
   		{
		   sum = local_sum;
		       
		   for (source = 1; source < p; source++)  									//receive each sum from all the workers and add them together
		   {
				MPI_Recv(&local_sum, 1, MPI_INT, source, tag3, MPI_COMM_WORLD, &status);
				sum = sum + local_sum;
			}
			
			m=(float)sum/n;
			
    		printf("\na.\n Average: %.3f\n", m);		
   		}

		////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////VARIANCE//////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////
		
		float each_var=0;				//(x[i]-m)^2
		float local_var=0;				//(x[i]-m)^2+(x[i+1]-m)^2+.....
		float var=0;					
		float total_var=0;
		
		if(cpu_rank == 0)
		{
			for (target = 1; target < p; target++)  	
    				MPI_Send(&m, 1, MPI_FLOAT, target, tag4, MPI_COMM_WORLD);
		}
		else
		{
			MPI_Recv(&m, 1, MPI_FLOAT, 0, tag4, MPI_COMM_WORLD, &status);
		}


		/////////////////////////////////////////////////////////
		///////ALL CPU'S WILL EXECUTE THIS PART///////
		for(i=0; i<num; i++)
		{
			each_var=pow((X_local[i]-m),2);
			local_var=local_var+each_var;	
		}
		/////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////

   		if (cpu_rank != 0)  														//if you are a cpu worker
   		{
       			MPI_Send(&local_var, 1, MPI_INT, 0, tag5, MPI_COMM_WORLD);			//send back the var of the numbers you worked with
    	}
   		else  																		//if you are the cpu coordinator
   		{
		   var = local_var;
		       
		   for (source = 1; source < p; source++)  									//receive each var from all the workers and add them together
		   {
				MPI_Recv(&local_var, 1, MPI_INT, source, tag5, MPI_COMM_WORLD, &status);
				var=var+local_var;
		   }
			
			total_var=(float)var/n;

    			printf("\nb.\n Variance: %.3f\n", total_var);
   		}
   		
   		/////////////////////////////////////////////////////////////////////////////////////////////
   		//////////////////////////////////////////d[i] ARRAY/////////////////////////////////////////
   		/////////////////////////////////////////////////////////////////////////////////////////////
   		
   		int max_list[p];			//array which each record is the max record from all local records of all cpu's
   		int min_list[p];			//array which each record is the min record from all local records of all cpu's
   		
   		int local_max;
   		int local_min;
   		
   		int total_max;
   		int total_min;
   		
   		int d[n];					//d[i]=((X[i]-min(X))/(max(X)-min(X))*100

		/////////////////////////////////////////////////////////
		///////ALL CPU'S WILL EXECUTE THIS PART//////////////////
		local_max=X_local[0];
		for(i=0; i<num; i++)
		{
			if(X_local[i]>local_max)
				local_max=X_local[i];
		}
		
		local_min=X_local[0];
		for(i=0; i<num; i++)
		{
			if(X_local[i]<local_min)
				local_min=X_local[i];
		}
		/////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////

		if(cpu_rank != 0)														//if you are a cpu worker
		{
			MPI_Send(&local_max, 1, MPI_INT, 0, tag6, MPI_COMM_WORLD);			//send back your local_max and local_min
			MPI_Send(&local_min, 1, MPI_INT, 0, tag7, MPI_COMM_WORLD);				
		}
		else																	//if you are the cpu coordinator
		{
			for (source = 1; source < p; source++)  							//receive each local_max from all the workers and put them to their list
			{
		       		MPI_Recv(&local_max, 1, MPI_INT, source, tag6, MPI_COMM_WORLD, &status);
		       		max_list[source-1]=local_max;
		       		
		       		MPI_Recv(&local_min, 1, MPI_INT, source, tag7, MPI_COMM_WORLD, &status);
				min_list[source-1]=local_min;
			}
			
			max_list[p-1]=local_max;											//add the coordinator's local_max and local_min at the end of each list
			min_list[p-1]=local_min;
			
			
			total_max=max_list[0];												//find the total max of the entire array
			for(i=0; i<p; i++)
			{
				if(max_list[i]>total_max)
					total_max=max_list[i];
			}
			
			total_min=min_list[0];												//find the total min of the entire array
			for(i=0; i<p; i++)
			{
				if(min_list[i]<total_min)
					total_min=min_list[i];
					
			}
			
			for(i=0; i<n; i++)
				d[i]=((X[i]-total_min)/(total_max-total_min))*100;

			printf("\nc.\n New Vector d[]: \n");
			
			for(i=0; i<n; i++)
				printf("\td[%d]=%d\n",i, d[i]);
		}

   MPI_Finalize();
   
   return 0;
}