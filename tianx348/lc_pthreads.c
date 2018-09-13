// CSCI5451 Spring 2018 Assignment1
// Author: Haonan Tian
// UMN ID: 5229178
// 02/03/2018

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
//#define _POSIX_C_SOURCE 199309L
//#define MAX_LINE_CHARACTERS = 1024

struct dataSet { //Struct for data set
	float *array;
	int totalItem;
	int dimension;
};

struct labelSet { //Struct for label set
	int totalItem;
	float *labelArray;
};

struct resultPool { //Struct to store computation results for XTY and XTX
	float *XTY;
	float *XTX;
} result_poor;

struct pass_struct {
		struct dataSet passInSet;
		struct labelSet passInLabel;
		float *outputXTY;
		float *outputXTX;
		int forEach;
		int total;
		int startPoint;
};

struct pass_struct2 {
	struct dataSet passInSet;
	struct labelSet passInLabel;
	float *outputXTX1;
	int start;
	int total;
	int forEach;
};

struct pass_struct3{
	struct dataSet passInSet;
	struct labelSet passInLabel;
	float *passInW;
	int total;
	int start;
	float result;
};

static void print_time(double const seconds){
	printf("Execution time: %0.04fs\n", seconds);
}

static inline double monotonic_seconds(){
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void readInFile1(char* fin, struct dataSet *input){ //Read in the data set to a 2-D integer array
	FILE *in = fopen(fin, "r");
	int info[2];
	char *temp;
	char p[1024];
	if (fgets(p,1024,in)!=NULL){
		temp = strtok(p," ");
		int info_cnt = 0;
		while (temp!=NULL){
			//printf("%s\n", temp);
			info[info_cnt] = atoi(temp);
			info_cnt += 1;
			temp = strtok(NULL," ");
		}
		input->totalItem = info[0];
		input->dimension = info[1];
		//printf("total item is %d and total dimension is %d\n", info[0], info[1]);
	}
	input->array = (float*)malloc(sizeof(float) * input->totalItem * input->dimension);
	//printf("%d\n", input->totalItem);
	int row = 0;
	char newTemp[sizeof(float)*input->dimension];
	//FILE *in2 = fopen(fin,"r");
	while(fgets(newTemp,sizeof(float)*input->dimension,in)!=NULL){
		int item_counter = 0;
		char modifiedTemp[strlen(newTemp)-1];
		strncpy(modifiedTemp,newTemp+1,strlen(newTemp)-1);
		char *token = strtok(modifiedTemp," ");
		while (token!=NULL){
			*(input->array + row * input->dimension + item_counter) = atof(token);
			item_counter += 1;
			token = strtok(NULL," ");
		}
		//printf("%d\n",item_counter );
		row += 1;
	}
	//printf("%d\n", row);
}

void readInFile2(char* fin2, struct labelSet *input){ //Read in the label file to an integer array
	FILE *in2 = fopen(fin2, "r");
	char temp[1024];
	//printf("Up to here 2\n");
	if (fgets(temp,1024,in2)!=NULL){
		//printf("Up to here 2\n");
		input->totalItem = atoi(temp);
	}
	input->labelArray = (float*)malloc(sizeof(float) * input->totalItem);
	int label_cnt = 0;
	while(fgets(temp,128,in2)){
		*(input->labelArray + label_cnt) = atof(temp);
		label_cnt += 1;
	}
	//printf("%d\n", label_cnt);
}

void initializeW(float w[], int dimension){ // vector w initializer
	for (int i = 0; i < dimension; i++){
		w[i] = 0;
	}
}

float vectorMultiplication(float vec1[], float vec2[], int length){ // function for vector multiplication
	float *localvec1 = vec1;
	float *localvec2 = vec2;
	float sum = 0;
	//printf("%d\n",length1);
	for (int i = 0; i < length; i++){
		sum = sum + localvec1[i]*localvec2[i];
	}
	return sum;
}

float* extractColumn(struct dataSet *inputSet, int col){ //get the transpose vector from data set NOTE: col starts from 0
	float *outputColumn;
	outputColumn = (float*) malloc(sizeof(float) * inputSet->totalItem);
	for (int i = 0; i < inputSet->totalItem; i++){
		outputColumn[i] = *(inputSet->array + i*inputSet->dimension + col);
	}
	return outputColumn;
}

void *computePool(void *arg){ // function to calculate result pool in multiple threads
	struct pass_struct *local = arg;
	struct dataSet *localSet = &local->passInSet;
	int start = local->startPoint;
	int end = local->total;
	int forEach = local->forEach;
	int counter = 0;
	while (counter<end){
		float *temp = extractColumn(localSet,counter+start*forEach);
		local->outputXTY[counter] = vectorMultiplication(temp,local->passInLabel.labelArray,localSet->totalItem);
		local->outputXTX[counter] = vectorMultiplication(temp,temp, localSet->totalItem);
		//printf("thread %d finished iteration %d\n", start, counter);
		free(temp);
		counter ++;
	}
	return NULL;
}

/*struct resultPool constructPool(struct dataSet *inputSet, struct labelSet *inputLabel, int totalThreads){ // use Multiple threads to construct XT multiple y[] vector set
	int each = round((float)inputSet->dimension/(float)totalThreads);
	//printf("each thread needs to handle %d cases\n", each);
	pthread_t threads[totalThreads];
	struct pass_struct passIn1;
	passIn1.passInSet = *inputSet;
	passIn1.passInLabel = *inputLabel;
	passIn1.dimension = inputSet->dimension;
	passIn1.outputXTY = malloc(sizeof(float)*inputSet->dimension);
	passIn1.outputXTX = malloc(sizeof(float)*inputSet->dimension);
	for (long p =0; p < totalThreads-1; p++){
		passIn1.startPoint = p * each;
		passIn1.total = each;
		pthread_create(&threads[p],NULL,computePool, &passIn1);
		usleep(2000);
		//printf("done 1\n");
	}
	passIn1.startPoint = (totalThreads-1) * each;
	passIn1.total = inputSet->dimension - passIn1.startPoint;
	pthread_create(&threads[totalThreads-1],NULL,computePool,&passIn1);
	//printf("finished creating threads\n");
	for(int p=0; p<totalThreads; p++){              // wait for each thread to finish
    pthread_join(threads[p], (void **) NULL);
	}
    struct resultPool output;
    output.XTY = passIn1.outputXTY;
    output.XTX = passIn1.outputXTX;
    return output;
}*/

struct resultPool constructPool(struct dataSet *inputSet, struct labelSet *inputLabel, int totalThreads){ // use Multiple threads to construct XT multiple y[] vector set
	int each = round((float)inputSet->dimension/(float)totalThreads);
	//printf("each thread needs to handle %d cases\n", each);
	pthread_t threads[totalThreads];
	struct pass_struct passIn1[totalThreads];
	for (int i =0; i < totalThreads; i++){
		if (i != totalThreads-1){
			struct pass_struct temp;
			temp.passInSet = *inputSet;
			temp.passInLabel = *inputLabel;
			temp.forEach = each;
			temp.outputXTX = malloc(sizeof(float)*each);
			temp.outputXTY = malloc(sizeof(float)*each);
			temp.startPoint = i;
			temp.total = each;
			passIn1[i] = temp;
		} else {
			struct pass_struct temp;
			temp.passInSet = *inputSet;
			temp.passInLabel = *inputLabel;
			temp.forEach = each;
			temp.total = inputSet->dimension - each*(totalThreads-1);
			temp.startPoint = i;
			temp.outputXTX = malloc(sizeof(float)*(inputSet->dimension - each*(totalThreads-1)));
			temp.outputXTY = malloc(sizeof(float)*(inputSet->dimension - each*(totalThreads-1)));
			passIn1[i] = temp;
		}
	}
	for (long p =0; p < totalThreads; p++){
		pthread_create(&threads[p],NULL,computePool, &passIn1[p]);
	}
	//printf("finished creating threads\n");
	for(int p=0; p<totalThreads; p++){              // wait for each thread to finish
    pthread_join(threads[p], (void **) NULL);
	}
    struct resultPool output;
    output.XTY = malloc(sizeof(float)*inputSet->dimension);
    output.XTX = malloc(sizeof(float)*inputSet->dimension);
		for (int i = 0; i < totalThreads; i++){
			int total = passIn1[i].total;
			for (int j = 0; j < total; j++){
				*(output.XTX+i*each+j) = *(passIn1[i].outputXTX+j);
				*(output.XTY+i*each+j) = *(passIn1[i].outputXTY+j);
			}
			free(passIn1[i].outputXTX);
			free(passIn1[i].outputXTY);
		}
    return output;
}

float extractXTY(struct resultPool *inputPool, int index){ // extract a certain element of XTY with index from result pool
	return *(inputPool->XTY + index);
}

float extractXTX(struct resultPool *inputPool, int index){ // extract a certain element of XTX with index from result pool
	return *(inputPool->XTX + index);
}

void *computeXTX1(void *arg){ // calculate each assigned XT multiple X^-1
	struct pass_struct2 *local = arg;
	int startPoint = local->start;
	struct dataSet *localSet = &local->passInSet;
	int forEach = local->forEach;
	int counter = 0;
	while (counter < local->total){
		float *temp = extractColumn(&local->passInSet, startPoint*forEach+counter);
		int temp_cnt = 0;
		for (int i = 0; i < localSet->dimension; i++){
			if (i != startPoint*forEach+counter){
				float *temp2 = extractColumn(&local->passInSet, i);
				*(local->outputXTX1+(counter)*(localSet->dimension-1)+temp_cnt) = vectorMultiplication(temp,temp2, localSet->totalItem);
				free(temp2);
				temp_cnt ++;
			}
		}
		//printf("thread %d finished iteration %d\n", startPoint, counter);
		counter ++;
	}
	return NULL;
}

/*float* constructXTX1(struct dataSet *inputSet, struct labelSet *inputLabel, int totalThreads){ // calculate the result of XT multiple X^-1
	int each = round((float)inputSet->dimension/(float)totalThreads);
	//printf("each thread needs to deal with %d cases\n", each);
	pthread_t threads[totalThreads];
	struct pass_struct2 passIn2;
	passIn2.passInSet = *inputSet;
	passIn2.passInLabel = *inputLabel;
	passIn2.dimension = inputSet->dimension;
	passIn2.outputXTX1 = malloc(sizeof(float)*inputSet->totalItem*(inputSet->dimension-1));
	//printf("start to create threads\n");
	for (long p = 0; p < totalThreads-1; p++){
		passIn2.start = p * each;
		passIn2.total = each;
		pthread_create(&threads[p],NULL,computeXTX1,&passIn2);
		usleep(2000);
	}
	passIn2.start = (totalThreads-1)*each;
	passIn2.total = inputSet->dimension - passIn2.start;
	pthread_create(&threads[totalThreads-1],NULL,computeXTX1,&passIn2);
	//printf("finish creating threads\n");
	for(int p=0; p<totalThreads; p++){              // wait for each thread to finish
    pthread_join(threads[p], (void **) NULL);
	}
	//printf("all threads joined\n");
	float *output = passIn2.outputXTX1;
	return output;
}*/

float* constructXTX1(struct dataSet *inputSet, struct labelSet *inputLabel, int totalThreads){ // calculate the result of XT multiple X^-1
	int each = round((float)inputSet->dimension/(float)totalThreads);
	//printf("each thread needs to deal with %d cases\n", each);
	pthread_t threads[totalThreads];
	struct pass_struct2 passIn2[totalThreads];
	for (int i =0; i < totalThreads; i++){
		if (i != totalThreads-1){
			struct pass_struct2 temp;
			temp.passInSet = *inputSet;
			temp.passInLabel = *inputLabel;
			temp.forEach = each;
			temp.start = i;
			temp.total = each;
			temp.outputXTX1 = malloc(sizeof(float)*(inputSet->dimension-1)*each);
			passIn2[i] = temp;
		} else {
			struct pass_struct2 temp;
			temp.passInSet = *inputSet;
			temp.passInLabel = *inputLabel;
			temp.forEach = each;
			temp.start = i;
			temp.total = inputSet->dimension - i * each;
			temp.outputXTX1 = malloc(sizeof(float)*(inputSet->dimension-1)*temp.total);
			passIn2[i] = temp;
		}
	}
	//printf("start to create threads\n");
	for (long p = 0; p < totalThreads; p++){
		pthread_create(&threads[p],NULL,computeXTX1,&passIn2[p]);
	}
	//printf("finish creating threads\n");
	for(int p=0; p<totalThreads; p++){              // wait for each thread to finish
    pthread_join(threads[p], (void **) NULL);
	}
	//printf("all threads joined\n");
	float *output;
	output = malloc(sizeof(float)*(inputSet->dimension-1)*inputSet->dimension);
	for (int i = 0; i < totalThreads; i++){
		int total = passIn2[i].total;
		for (int j = 0; j < total; j++){
			for (int k = 0; k < inputSet->dimension-1; k++){
				*(output+(i*each+j)*(inputSet->dimension-1) + k) = *(passIn2[i].outputXTX1+j*(inputSet->dimension-1)+k);
			}
		}
		free(passIn2[i].outputXTX1);
	}
	return output;
}

float* extractXTX2(float XTX2[], int index, int dimension){
	float *output;
	output = malloc(sizeof(float)*(dimension-1));
	for (int i = 0; i < dimension-1; i++){
		output[i] = *(XTX2 + index * (dimension-1) + i);
	}
	return output;
}

float* extractW(float w[], int index, int dimension){
	float *output;
	output = malloc(sizeof(float)*(dimension-1));
	int counter = 0;
	for (int i = 0; i < dimension-1; i++){
		if (i != index){
			output[counter] = *(w + i);
			counter ++;
		}
	}
	return output;
}


float* extractRow(struct dataSet *inputSet, int row){
	float *output;
	output = malloc(sizeof(float)*inputSet->dimension);
	for (int i = 0; i < inputSet->dimension; i++){
		output[i] = *(inputSet->array + row * inputSet->dimension + i);
	}
	return output;
}

void *computeLoss(void *arg){
	struct pass_struct3 *local = arg;
	struct dataSet *localSet = &local->passInSet;
	struct labelSet *localSet2 = &local->passInLabel;
	int start = local->start;
	float output = 0;
	int counter = 0;
	while (counter < local->total){
		float *temp = extractRow(&local->passInSet, start + counter);
		float tempResult = vectorMultiplication(temp,local->passInW, localSet->dimension);
		tempResult = tempResult - *(localSet2->labelArray + start + counter);
		tempResult = tempResult * tempResult;
		output = output + tempResult;
		counter ++;
		free(temp);
	}
	local->result = output;
	return NULL;
}

float calculateLoss(struct dataSet *inputSet, float w[], struct labelSet *inputLabel, int totalThreads){
	float result = 0;
	int each = round((float)inputSet->totalItem/(float)totalThreads);
	pthread_t threads[totalThreads];
	struct pass_struct3 passIn[totalThreads];
	for (int i = 0; i < totalThreads-1; i++){
		struct pass_struct3 pass1;
		pass1.passInSet = *inputSet;
		pass1.passInLabel = *inputLabel;
		pass1.total = each;
		pass1.start = i * each;
		pass1.passInW = w;
		pass1.result = 0;
		passIn[i] = pass1;
	}
	struct pass_struct3 pass1;
	pass1.passInSet = *inputSet;
	pass1.passInLabel = *inputLabel;
	pass1.passInW = w;
	pass1.result = 0;
	pass1.total = inputSet->totalItem - (totalThreads-1) * each;
	pass1.start = (totalThreads-1) * each;
	passIn[totalThreads-1] = pass1;
	for (long p = 0; p < totalThreads; p++){
		pthread_create(&threads[p],NULL,computeLoss,&passIn[p]);
	}
	for(int p=0; p<totalThreads; p++){              // wait for each thread to finish
    	pthread_join(threads[p], (void **) NULL);
	}
	for (int i = 0; i < totalThreads; i++){
		result = result + passIn[i].result;
		//printf("%f ",result);
	}
	//printf("\n");
	return result;
}

int main(int argc, char *argv[]){
	if (argc != 5){
		perror("Invalid input, program exit..");
		exit(1);
	}
	// initialization
	char *fin1 = argv[1];
	char *fin2 = argv[2];
	int outerIter = atoi(argv[3]);
	int num_threads = atoi(argv[4]);
	// read in data set file to a 2-D integer array
	if (fin1 == NULL){
		perror("File Open error");
		exit(1);
	}
	struct dataSet set1;
	readInFile1(fin1, &set1);
	// read in a label file to an integer array
	if (fin2 == NULL){
		perror("File Open Error");
		exit(1);
	}
	//float *temp = extractRow(&set1,0);
	struct labelSet label1;
	readInFile2(fin2, &label1);
	// start to record execution time
	double startTime = monotonic_seconds();
	// start to compute and stroe the computation results from XT multiple Y and XT multiple X.
	struct resultPool result1;
	printf("start to construct pool\n");
	result1 = constructPool(&set1,&label1,num_threads);
	printf("finished pool construction\n");
	// start to compute and store the result from multipling XT and X^-1
	float *resultXTX2;
	printf("start to construct XTX2\n");
	resultXTX2 = constructXTX1(&set1,&label1,num_threads);
	printf("finished XTX2 construction\n");
	printf("\n");
	// initialize vector w
	float *w;
	w = malloc(sizeof(float)*set1.dimension);
	initializeW(w,set1.dimension);
	// finish construct all result pools and start iterations
	int outer_cnt = 0;
	while (outer_cnt < outerIter){ // execute outer iterations
		for (int i = 0; i < set1.dimension; i++){ // execute inner iterations
			float temp3 = extractXTX(&result1,i);
			if (temp3 == 0){
				w[i] = 0;
			} else {
				float *tempVec1 = extractXTX2(resultXTX2,i,set1.dimension);
				float *tempVec2 = extractW(w,i,set1.dimension);
				w[i] = (extractXTY(&result1,i) - vectorMultiplication(tempVec1,tempVec2, set1.dimension-1))/temp3;
				free(tempVec1);
				free(tempVec2);
			}
		}
		printf("Finished outer iteration %d.\n", outer_cnt);
		float totalLoss = calculateLoss(&set1,w,&label1,num_threads);
		printf("The loss value for this round is %f\n\n", totalLoss);
		printf("\n\n");
		outer_cnt ++;
	}
	double endTime = monotonic_seconds();
  print_time(endTime-startTime);
   	free(w);
   	free(set1.array);
   	free(label1.labelArray);
   	free(result1.XTX);
   	free(result1.XTY);
   	free(resultXTX2);
    printf("Program finished normally\n");
    exit(0);
}
