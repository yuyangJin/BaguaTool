#define __USE_GNU
//#define _GNU_SOURCE
#define UNW_LOCAL_ONLY

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
#include <unistd.h>
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


#define ADDRLOGSIZE 80000
#define MAX_STACK_DEPTH 100
#define MAX_CALL_PATH_LEN 1300 // MAX_STACK_DEPTH * 13
#define NUM_EVENTS 4

#define DEFAULT_CYC_SAMPLE_COUNT  (2300000000) // 1000ms
#define DEFAULT_INS_SAMPLE_COUNT  (20000000) // 10ms
#define DEFAULT_CM_SAMPLE_COUNT  (100000) // 10ms


typedef struct callPathStruct{
	char callPath[MAX_CALL_PATH_LEN] = {0};
	unsigned long long int count = 0;
}CPS;


int mpiRank = -1;

int EventSet = PAPI_NULL;

static int CYC_SAMPLE_COUNT ;
static int INS_SAMPLE_COUNT ;
static int CM_SAMPLE_COUNT ;
static int module_init = 0;


//unordered_map<string, unsigned int> callPathAddrLog; // first string is for call path, second unsigned int is for count
static CPS callPathAddrLog[NUM_EVENTS][ADDRLOGSIZE];  // (20 * NUM_EVENTS) MB
static unsigned long long int callPathLogPointer[NUM_EVENTS] = {0};

//static char* addr_threshold;

ofstream outputStream;

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
	//ofstream outputStream((string("SAMPLE") + to_string(mpiRank) + string(".TXT")), ios_base::app);
	//if(mpiRank == 0){
	//MPI_Comm_rank(MPI_COMM_WORLD,&mpiRank);
	for(int i = 0; i < NUM_EVENTS; i++){
		ofstream outputStream((string("SAMPLE") + to_string(mpiRank) + string("-") + to_string(i) + string(".TXT")), ios_base::app);
		//ofstream outputStream((string("SAMPLE") + to_string(0) + string(".TXT")), ios_base::app);
		if (!outputStream.good()) {
			cerr << "Failed to open sample file\n";
			return;
		}
		LOG_INFO("Rank %d : WRITE %llu ADDR to %d TXT\n", mpiRank, callPathLogPointer[i], i);
		for (int j = 0; j < callPathLogPointer[i]; j++){
			outputStream << callPathAddrLog[i][j].callPath <<" | " << callPathAddrLog[i][j].count <<'\n';
		}
		
		callPathLogPointer[i] = 0;
	
		outputStream.close();
	}
	//startTimer();
}


int my_backtrace(unw_word_t *buffer, int max_depth) {
	unw_cursor_t cursor;
	unw_context_t context;

	// Initialize cursor to current frame for local unwinding.
	unw_getcontext(&context);
	unw_init_local(&cursor, &context);

	// Unwind frames one by one, going up the frame stack.
	int depth = 0;
	while(unw_step(&cursor) > 0 && depth < max_depth) {
		//unw_word_t offset;
		unw_word_t pc;
		unw_get_reg(&cursor, UNW_REG_IP, &pc);
		if (pc == 0) {
			break;
		}
		//printf("0x%lx:",pc);
		buffer[depth] = pc;
		depth ++;

		// char sym[256];
		// if(unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
		// 	printf("(%s+0x%lx)\n", sym, offset);
		// } else {
		// 	printf(" -- error: unable to obtain symbol name for thisframe\n");
		// }
	}
	return depth;
}




void papi_handler(int EventSet, void *address, long_long overflow_vector, void *context){
	//LOG_LINE;
	TRY(PAPI_stop(EventSet, NULL), PAPI_OK);

	//
	int Events[NUM_EVENTS], number, x, y;
	number = NUM_EVENTS;
	//LOG_LINE;
	TRY(PAPI_get_overflow_event_index(EventSet, overflow_vector, Events, &number), PAPI_OK);
	//LOG_LINE;
	for ( x=0; x<number; x++ ) {
		for ( y=0; y<NUM_EVENTS; y++ ) {
			if ( Events[x] == y ) {
				//LOG_LINE;
				
#ifdef MY_BT
				unw_word_t buffer[MAX_STACK_DEPTH] = {0};
#else
				void *buffer[MAX_STACK_DEPTH];
				memset(buffer, 0, sizeof(buffer));
#endif
				unsigned int i, depth = 0;
				//LOG_LINE;
				
				//LOG_LINE;
#ifdef MY_BT
				depth = my_backtrace(buffer, MAX_STACK_DEPTH);
#else
				depth = unw_backtrace(buffer, MAX_STACK_DEPTH);
				//depth = backtrace(buffer, MAX_STACK_DEPTH);
#endif
				unsigned int addr_log_pointer = 0;

#ifdef MY_BT
				unw_word_t address_log[MAX_STACK_DEPTH]={0};
#else
				void* address_log[MAX_STACK_DEPTH]={0};
#endif
				//LOG_LINE;

				char callPath[MAX_CALL_PATH_LEN] = {0};
				int offset = 0;

#ifdef MY_BT
				for (i = 4; i < depth; ++ i)
				{
					//LOG_INFO("%08x\n",buffer[i]);
					//if( (void*)buffer[i] != NULL && (char*)buffer[i] < addr_threshold ){ 
					if( buffer[i] != 0 ){ 
						address_log[addr_log_pointer] = (buffer[i]-2);
						addr_log_pointer++;
						//LOG_INFO("%08x\n",buffer[i]);
					}
				}

				if(addr_log_pointer > 0){
					for (i = 0; i < addr_log_pointer; ++ i){
						offset+=snprintf(callPath+offset,MAX_CALL_PATH_LEN - offset - 4 ,"%lx ",address_log[i]);
						//LOG_INFO("%08x\n",address_log[i]);
					}
				}
#else
				for (i = 4; i < depth; ++ i)
				{
					//if( (void*)buffer[i] != NULL && (char*)buffer[i] < addr_threshold ){ 
					if( (void*)buffer[i] != NULL ){ 
						address_log[addr_log_pointer] = (void*)(buffer[i]-2);
						addr_log_pointer++;
						//LOG_INFO("%08x\n",buffer[i]);
					}
				}

				if(addr_log_pointer > 0){
					for (i = 0; i < addr_log_pointer; ++ i){
						//callPath += to_string((long long)(address_log[i])) + string(" ");
						offset+=snprintf(callPath+offset,MAX_CALL_PATH_LEN - offset - 4 ,"%lx ",address_log[i]);
						//LOG_INFO("%08x\n",address_log[i]);
					}
				}
#endif
				//LOG_LINE;
				bool hasRecordFlag = false;

				for (i = 0; i < callPathLogPointer[y]; i++){
					if (strcmp(callPath, callPathAddrLog[y][i].callPath) == 0){
						callPathAddrLog[y][i].count ++;
						hasRecordFlag = true;
						break;
					}
				}

				if(hasRecordFlag == false){
					if(strlen(callPath) >= MAX_CALL_PATH_LEN){
						LOG_WARN("Call path string length (%d) is longer than %d", strlen(callPath),MAX_CALL_PATH_LEN);
					}else{
						strcpy(callPathAddrLog[y][callPathLogPointer[y]].callPath,callPath);
						//LOG_INFO("callPathLogPointer : %lld\n",callPathLogPointer[y]);
						callPathAddrLog[y][callPathLogPointer[y]].count = 1;
						callPathLogPointer[y]++;
					}
				}

				//LOG_LINE;

				if(callPathLogPointer[y] >= ADDRLOGSIZE-10){
					write_addr_log();
				}
			}
		}
	}
	//LOG_LINE;

	TRY(PAPI_start(EventSet), PAPI_OK);
}


void static set_papi_overflow()
{
	int Events[NUM_EVENTS] , i;

	EventSet = PAPI_NULL;
	TRY(PAPI_create_eventset(&EventSet), PAPI_OK);

	Events[0] = PAPI_TOT_CYC;
  //Events[1] = PAPI_L2_DCM;
	//Events[1] = PAPI_L1_DCM;
	//Events[1] = PAPI_TOT_INS;
	Events[1] = PAPI_LD_INS;
	Events[2] = PAPI_SR_INS;
	Events[3] = PAPI_L1_DCM;
	//Events[3] = PAPI_L3_DCA;


	TRY(PAPI_add_events(EventSet, (int *)Events, NUM_EVENTS), PAPI_OK);

	TRY(PAPI_overflow(EventSet, PAPI_TOT_CYC, CYC_SAMPLE_COUNT, 0, papi_handler), PAPI_OK);
	TRY(PAPI_overflow(EventSet, PAPI_LD_INS, INS_SAMPLE_COUNT, 0, papi_handler), PAPI_OK);
	TRY(PAPI_overflow(EventSet, PAPI_SR_INS, INS_SAMPLE_COUNT, 0, papi_handler), PAPI_OK);
	TRY(PAPI_overflow(EventSet, PAPI_L1_DCM, CM_SAMPLE_COUNT, 0, papi_handler), PAPI_OK);
	//TRY(PAPI_overflow(EventSet, PAPI_L3_DCA, CM_SAMPLE_COUNT, 0, papi_handler), PAPI_OK);

	TRY(PAPI_start(EventSet), PAPI_OK);
	printf("set_papi_overflow() PAPI_start(EventSet), PAPI_OK\n");
}


static void init() __attribute__((constructor));
static void fini() __attribute__((destructor));

static void init() {
	if(module_init == MODULE_INITED) return ;
	module_init = MODULE_INITED;	
	//addr_threshold =(char*)malloc( sizeof(char));
	//unsetenv("LD_PRELOAD");
	// PAPI setup for main thread
	char* str = getenv("CYC_SAMPLE_COUNT");
	CYC_SAMPLE_COUNT = (str ? atoi(str) : DEFAULT_CYC_SAMPLE_COUNT);
	str = getenv("INS_SAMPLE_COUNT");
	INS_SAMPLE_COUNT = (str ? atoi(str) : DEFAULT_INS_SAMPLE_COUNT);
	str = getenv("CM_SAMPLE_COUNT");
	CM_SAMPLE_COUNT = (str ? atoi(str) : DEFAULT_CM_SAMPLE_COUNT);

	LOG_INFO("SET sample interval to %d cycles\n", CYC_SAMPLE_COUNT);
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

	//free(addr_threshold);
	pid_t pid = getpid();
	string str = string("cat /proc/") + to_string(pid) + string("/maps > LDMAP") + to_string(mpiRank) + string(".TXT");
	system(str.c_str());

}
