## This is a makefile for CSCI5451 Porject1 ##
## Author: Haonan Tian ##
## 02/11/2018 ##
CC = gcc
CFLAGS = -g -Wall

default: lc_pthreads lc_openmp

lc_pthreads: lc_pthreads.o
		$(CC) $(CFLAGS) -O3 -march=native -pthread -o lc_pthreads lc_pthreads.o -lm

lc_pthreads.o: lc_pthreads.c
		$(CC) $(CFLAGS) -O3 -march=native -pthread -c lc_pthreads.c -lm

lc_openmp: lc_openmp.o
		$(CC) $(CFLAGS) -fopenmp -O3 -march=native -o lc_openmp lc_openmp.o -lm

lc_openmp.o: lc_openmp.c
		$(CC) $(CFLAGS) -fopenmp -O3 -march=native -c lc_openmp.c -lm

clean:
		rm lc_openmp lc_pthreads *.o
