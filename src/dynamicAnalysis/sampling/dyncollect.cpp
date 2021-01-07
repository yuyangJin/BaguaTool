#define __USE_GNU
//#define _GNU_SOURCE


#include "IRStruct.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cassert>
#include <chrono>
#include <string>
#include <unordered_map>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
//#include <unwind.h>
//#include <asm-generic/sections.h>
//#include <pthread.h>
//#include <dlfcn.h>
#include <papi.h>
//#include <execinfo.h>
#include <libunwind.h>
#include <fstream>
#include <mpi.h>
using namespace std;



// these macros can be used for colorful output
#define TPRT_NOCOLOR "\033[0m"
#define TPRT_RED "\033[1;31m"
#define TPRT_GREEN "\033[1;32m"
#define TPRT_YELLOW "\033[1;33m"
#define TPRT_BLUE "\033[1;34m"
#define TPRT_MAGENTA "\033[1;35m"
#define TPRT_CYAN "\033[1;36m"
#define TPRT_REVERSE "\033[7m"

#define LOG_INFO(fmt, ...) fprintf(stderr,TPRT_GREEN fmt TPRT_NOCOLOR, __VA_ARGS__);
#define LOG_ERROR(fmt, ...) fprintf(stderr,TPRT_RED fmt TPRT_NOCOLOR, __VA_ARGS__);
#define LOG_WARN(fmt, ...) fprintf(stderr,TPRT_MAGENTA fmt TPRT_NOCOLOR, __VA_ARGS__);
#define LOG_LINE fprintf(stderr,TPRT_BLUE "line=%d\n" TPRT_NOCOLOR, __LINE__);
//


#define MODULE_INITED 1
#define PTHREAD_VERSION "GLIBC_2.3.2"


#define ADDRLOGSIZE 20000
#define MAX_STACK_DEPTH 100
#define MAX_CALL_PATH_LEN 1000 // MAX_STACK_DEPTH * 10
#define MAX_NODE_NUM 1000


#define DEFAULT_SAMPLE_COUNT  (2300000) // 10ms

typedef struct callPathStruct{
	char callPath[MAX_CALL_PATH_LEN];
	unsigned long long int count;
}CPS;


int mpiRank = -1; // -1 for error

int EventSet = PAPI_NULL;


static int SAMPLE_COUNT ;
static int module_init = 0;

static CPS callPathAddrLog[ADDRLOGSIZE];  //160MB
static unsigned long long int callPathLogPointer = 0;


static char* addr_threshold;

ofstream outputStream;
ofstream logStream;

static decltype(chrono::high_resolution_clock::now()) timer;
static double sumTime = 0;

static void startTimer() {
	timer = chrono::high_resolution_clock::now();
}

static void stopTimer() {
	auto end = chrono::high_resolution_clock::now();
	chrono::duration<double> duration = end - timer;
	sumTime += duration.count();
}

static void write_addr_log(){
	//stopTimer();

	//if(mpiRank == 0){
	//MPI_Comm_rank(MPI_COMM_WORLD,&mpiRank);
	ofstream outputStream((string("SAMPLE") + to_string(mpiRank) + string(".TXT")), ios_base::app);
	//ofstream outputStream((string("SAMPLE") + to_string(0) + string(".TXT")), ios_base::app);
	if (!outputStream.good()) {
		cerr << "Failed to open sample file\n";
		return;
	}

	for (int i = 0; i < callPathLogPointer; i++){
		//cerr << i <<"\n";
		outputStream << callPathAddrLog[i].callPath <<" | " << callPathAddrLog[i].count <<'\n';
	}
	LOG_INFO("WRITE %d ADDR to TXT\n",callPathLogPointer);

	//addr_log_pointer = 0;
	callPathLogPointer = 0;

	outputStream.close();
	//startTimer();
}

void papi_handler(int EventSet, void *address, long_long overflow_vector, void *context){
	TRY(PAPI_stop(EventSet, NULL), PAPI_OK);
	void *buffer[MAX_STACK_DEPTH];
	unsigned int i, depth = 0;
	memset(buffer, 0, sizeof(buffer));
	depth = unw_backtrace(buffer, MAX_STACK_DEPTH);
	//depth = backtrace(buffer, MAX_STACK_DEPTH);

	//LOG_INFO("papi_handler: %d\n",callPathLogPointer)
	unsigned int addr_log_pointer = 0;
	void* address_log[MAX_STACK_DEPTH]={0};

	for (i = 0; i < depth; ++ i)
	{

		//LOG_INFO("%08x\n",buffer[i]);
		if( (void*)buffer[i] != NULL && (char*)buffer[i] < addr_threshold ){ 
			address_log[addr_log_pointer] = (void*)(buffer[i]-2);
			addr_log_pointer++;
			//LOG_INFO("%08x\n",buffer[i]);
			//break;
		}

	}

	char callPath[MAX_CALL_PATH_LEN] = {0};
	int offset = 0;
	if(addr_log_pointer > 0){
		for (i = 0; i < addr_log_pointer; ++ i){
			//callPath += to_string((long long)(address_log[i])) + string(" ");
			offset+=snprintf(callPath+offset,MAX_CALL_PATH_LEN - offset - 4 ,"%x ",address_log[i]);
			//LOG_INFO("%08x\n",address_log[i]);
		}
	}

	bool hasRecordFlag = false;

	for (i = 0; i < callPathLogPointer; i++){
		if (strcmp(callPath, callPathAddrLog[i].callPath) == 0){
			callPathAddrLog[i].count ++;
			hasRecordFlag = true;
			break;
		}
	}

	//cout << callPath<<endl;

	if(hasRecordFlag == false){
		if(strlen(callPath) >= MAX_CALL_PATH_LEN){
			LOG_WARN("Call path string length (%d) is longer than %d", strlen(callPath),MAX_CALL_PATH_LEN);
		}else{
			strcpy(callPathAddrLog[callPathLogPointer].callPath,callPath);
			//LOG_INFO("callPathLogPointer : %lld\n",callPathLogPointer);
			callPathAddrLog[callPathLogPointer].count = 1;
			callPathLogPointer++;
		}
	}


	if(callPathLogPointer >= ADDRLOGSIZE-100){
		write_addr_log();
	}
 
//LOG_LINE;

TRY(PAPI_start(EventSet), PAPI_OK);
}


void static set_papi_overflow()
{
	EventSet = PAPI_NULL;
	TRY(PAPI_create_eventset(&EventSet), PAPI_OK);
	TRY(PAPI_add_event(EventSet, PAPI_TOT_CYC), PAPI_OK);
	//TRY(PAPI_add_event(EventSet, PAPI_L3_TCM), PAPI_OK);
	//TRY(PAPI_overflow(EventSet, PAPI_L3_TCM, SAMPLE_COUNT, 0, papi_handler), PAPI_OK);
	TRY(PAPI_overflow(EventSet, PAPI_TOT_CYC, SAMPLE_COUNT, 0, papi_handler), PAPI_OK);
	TRY(PAPI_start(EventSet), PAPI_OK);
	//printf("set_papi_overflow() PAPI_start(EventSet), PAPI_OK\n");
}


static void init() __attribute__((constructor));
static void fini() __attribute__((destructor));

static void init() {
	if(module_init == MODULE_INITED) return ;
	module_init = MODULE_INITED;	
	addr_threshold =(char*)malloc( sizeof(char));
	//unsetenv("LD_PRELOAD");

	// PAPI setup for main thread
	char* str = getenv("SAMPLE_COUNT");
	SAMPLE_COUNT = (str ? atoi(str) : DEFAULT_SAMPLE_COUNT);

	//LOG_INFO("SET sample interval to %d cycles\n", SAMPLE_COUNT);
	TRY(PAPI_library_init(PAPI_VER_CURRENT), PAPI_VER_CURRENT);
	//TRY(PAPI_thread_init(pthread_self), PAPI_OK);
	set_papi_overflow();
	startTimer();
}

static void fini(){
	//LOG_LINE;
	TRY(PAPI_stop(EventSet, NULL), PAPI_OK);
	stopTimer();
	if(mpiRank == 0){
		printf("TOTAL TIME is %.4f\n",sumTime);
	}
	write_addr_log();
	free(addr_threshold);
}
