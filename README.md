# MPI_average_and_variance
A C-MPI parallel program that receives on an array X as input, returns its average, its variance 
and a new array based on: d[i]=((X[i]-min(X))/(max(X)-min(X))*100

Limitations:
1. only MPI_Send()/MPI_Recv() functions used
2. number of elements of X must be evenly divided by the number of the tasks

Guide to compile and run:
1. download the .tar file from here: ftp://ftp.mcs.anl.gov/pub/mpi/old/mpich-1.2.4.tar.gz
2. sudo apt-get install gcc
3. sudo apt-get install libmpich2-dev
4. mpicc -o ave_var ave_var.c
5. mpiexec -n "number_of_tasks" ./ave_var

Tested on Lubuntu 12.04 (Precise Pangolin) on a virtual machine.