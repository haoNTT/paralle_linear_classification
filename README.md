# paralle_linear_classification

General Description
The purpose of this project is to present a demo of speedup linear classification with squared loss by using Pthreads and OpenMP.
The source files of this project include two c source file named lc_pthreads.c and lc_openmp.c, two zip file within which two sets
of datasets are contained. Unzip the zip file named data.zip will get two files named data.csc and data.label which is the small
test data set for testing the correctness of the program. Unzip the zip file named data_large.zip will get two files data_lar.csv
and data_lar.label. These is the dataset used to test the performance of the program and generated reports.

About Dataset
The dataset used in this project is derived from the MNIST dataset. Details about MNIST can be found at: 
http://yann.lecun.com/exdb/mnist/
In this project, binary classification is performed to classify the input set. the dataset here corresponds to pixel values of 28X28 images of handwritten
digits corresponding to digits '1' and '7'. In the label file provided, the label corresponding to digit '1' is 1, while for '7', it is -1.

Input/Output Format
Each program will take as input two files (the list of data points, and the list of labels).
The input file with data points contains N+1 lines. The first line contains two space-separated integers: the
number of data points (N), and the dimensionality of each data point (D). The following N lines each contain
D space-separated integerss which represent the coordinates of the current data point. For example, an
input with four two-dimensional data points would be stored in a file as: 
4 2
1 1
1 2
3 4
3 2

The input file with class labels contains N+1 lines. The first line contains the number of data points (N). The
following N lines each contains one integer each, the class label of each data-point. For example, an input
with four data points would be stored in a file as: 
4
1
-1
1
-1

The project utilizes a  high-precision,monotonic, wall-clock timer to record and present the execution time. The time spent on reading and loading file will be omitted.
The following shows the details of time recording techniques:
/* Gives us high-resolution timers. */
#define _POSIX_C_SOURCE 199309L
#include <time.h>
/**
* @brief Return the number of seconds since an unspecified time (e.g., Unix
* epoch). This is accomplished with a high-resolution monotonic timer,
* suitable for performance timing.
*
* @return The number of seconds.
*/
static inline double monotonic_seconds()
{
 struct timespec ts;
 clock_gettime(CLOCK_MONOTONIC, &ts);
 return ts.tv_sec + ts.tv_nsec * 1e-9;
 }
 
 /**
* @brief Output the seconds elapsed while execution.
*
* @param seconds Seconds spent on execution, excluding IO.
*/
static void print_time(double const seconds)
{
 printf("Execution time: %0.04fs\n", seconds);
}

Function Call
The valid use of the the above parallel programs is to properly compile and then follows a valid input format. 
The valid input format is: executable + path to data file + path to label file + number of outer iterations + number of threads. 
For example to run the program on data.csv and data.label with 10 outer iterations and 2 threads, the following command should be entered into terminal:
./lc_pthreads /path/to/data.csv /path/to/data.label 10 2
./lc_openmp /path/to/data.csv /path/to/data.label 10 2
