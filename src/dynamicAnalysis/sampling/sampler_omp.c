#define _GNU_SOURCE
#define UNW_LOCAL_ONLY

#include "IRStruct.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>
#include <dlfcn.h>
#include <libunwind.h>
#include <papi.h>
#include <execinfo.h>
#include <pthread.h>
#include <omp.h>
#include <unistd.h>

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

#define MODULE_INITED 1
#define RESOLVE_SYMBOL_VERSIONED 1
#define RESOLVE_SYMBOL_UNVERSIONED 2
#define PTHREAD_VERSION "GLIBC_2.3.2"

#define DEFAULT_SAMPLE_COUNT  (1000) // 1ms

#define ADDRLOGSIZE 80000
#define MAX_STACK_DEPTH 100
#define MAX_CALL_PATH_LEN 1300 // MAX_STACK_DEPTH * 13
#define NUM_EVENTS 1

#ifndef __cplusplus
 
#define bool	_Bool
#define true	1
#define false	0

#endif

struct fn_wrapper_arg {
  void *(*fn)(void*);
  void *data;
};

typedef struct callPathStruct{
	char callPath[MAX_CALL_PATH_LEN] ;//= {0};
	int thread_id ; //= -1;
	unsigned long long int count ;//= 0;
}CPS;

int mpiRank = -1;
static int SAMPLE_COUNT = 10000;
static int module_init = 0;
static void (*original_GOMP_parallel)(void *(*fn) (void *), void *data, unsigned num_threads, unsigned int flags) = NULL;


static __thread int thread_gid;
//thread_local int thread_id;
static __thread int EventSet = PAPI_NULL;
static int thread_global_id;

FILE* fp = NULL;

static CPS callPathAddrLog[NUM_EVENTS][ADDRLOGSIZE];  // (20 * NUM_EVENTS) MB
static int callPathLogPointer[NUM_EVENTS] = {0};


int new_thread_gid()
{
  thread_gid = __sync_fetch_and_add(&thread_global_id, 1);
  //LOG_INFO("GET thread_gid = %d\n", thread_gid);
  return thread_gid;
}
void close_thread_gid()
{
  thread_gid = __sync_sub_and_fetch(&thread_global_id, 1);
  //LOG_INFO("GET thread_gid = %d\n", thread_gid);
  //return thread_gid; 
  return ;
}

static void write_addr_log(){
	//stopTimer();
	for(int i = 0; i < NUM_EVENTS; i++){
    char file_name[30] = {0};
    char mpi_rank_str[10] = {0};
    sprintf(mpi_rank_str, "%d", mpiRank);
    char event_num_str[2] = {0};
    sprintf(event_num_str, "%d", i);

    strcpy(file_name, "SAMPLE");
    strcat(file_name, mpi_rank_str);
    strcat(file_name, "-");
    strcat(file_name, event_num_str);
    strcat(file_name, ".TXT");

    fp=fopen(file_name, "a");
		// ofstream outputStream((string("SAMPLE") + to_string(mpiRank) + string("-") + to_string(i) + string(".TXT")), ios_base::app);
		// //ofstream outputStream((string("SAMPLE") + to_string(0) + string(".TXT")), ios_base::app);
		// if (!outputStream.good()) {
		// 	cerr << "Failed to open sample file\n";
		// 	return;
		// }
    if (!fp){
      LOG_INFO("Failed to open %s\n", file_name);
      callPathLogPointer[i] = __sync_and_and_fetch(&callPathLogPointer[i], 0);
      return;
    }
		LOG_INFO("Rank %d : WRITE %d ADDR to %d TXT\n", mpiRank, callPathLogPointer[i], i);
		for (int j = 0; j < callPathLogPointer[i]; j++){
      fprintf(fp, "%s | %lld | %d\n", callPathAddrLog[i][j].callPath, callPathAddrLog[i][j].count, callPathAddrLog[i][j].thread_id);
		// 	outputStream << callPathAddrLog[i][j].callPath <<" | " << callPathAddrLog[i][j].count << " | " << callPathAddrLog[i][j].thread_id<<endl;
      fflush(fp);
		}

		
		callPathLogPointer[i] = __sync_and_and_fetch(&callPathLogPointer[i], 0);
    //callPathLogPointer[i] = 0;

		//outputStream.close();
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
					if (strcmp(callPath, callPathAddrLog[y][i].callPath) == 0 && callPathAddrLog[y][i].thread_id == thread_gid){
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
						callPathAddrLog[y][callPathLogPointer[y]].thread_id = thread_gid;
						unsigned long long int x = __sync_fetch_and_add(&callPathLogPointer[y], 1);
            //callPathLogPointer[y] ++;
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

void printA(int EventSet, void *address, long_long overflow_vector, void *context){
  TRY(PAPI_stop(EventSet, NULL), PAPI_OK);
  //printf("%d A\n", thread_gid);
  TRY(PAPI_start(EventSet), PAPI_OK);
}

// void static set_papi_overflow()
// {
//   EventSet = PAPI_NULL;
//   TRY(PAPI_create_eventset(&EventSet), PAPI_OK);
//   TRY(PAPI_add_event(EventSet, PAPI_TOT_CYC), PAPI_OK);
//   TRY(PAPI_overflow(EventSet, PAPI_TOT_CYC, SAMPLE_COUNT, 0, printA), PAPI_OK);
//   TRY(PAPI_start(EventSet), PAPI_OK);
// }

void static set_papi_overflow()
{
	int Events[NUM_EVENTS] , i;

	EventSet = PAPI_NULL;
	TRY(PAPI_create_eventset(&EventSet), PAPI_OK);

	Events[0] = PAPI_TOT_CYC;
  //Events[1] = PAPI_L2_DCM;
	//Events[1] = PAPI_L1_DCM;
	//Events[1] = PAPI_TOT_INS;
	//Events[1] = PAPI_LD_INS;
	//Events[2] = PAPI_SR_INS;
	//Events[3] = PAPI_L1_DCM;
	//Events[3] = PAPI_L3_DCA;


	TRY(PAPI_add_events(EventSet, (int *)Events, NUM_EVENTS), PAPI_OK);

	TRY(PAPI_overflow(EventSet, PAPI_TOT_CYC, SAMPLE_COUNT, 0, papi_handler), PAPI_OK);
	//TRY(PAPI_overflow(EventSet, PAPI_LD_INS, INS_SAMPLE_COUNT, 0, papi_handler), PAPI_OK);
	//TRY(PAPI_overflow(EventSet, PAPI_SR_INS, INS_SAMPLE_COUNT, 0, papi_handler), PAPI_OK);
	//TRY(PAPI_overflow(EventSet, PAPI_L1_DCM, CM_SAMPLE_COUNT, 0, papi_handler), PAPI_OK);
	//TRY(PAPI_overflow(EventSet, PAPI_L3_DCA, CM_SAMPLE_COUNT, 0, papi_handler), PAPI_OK);

	TRY(PAPI_start(EventSet), PAPI_OK);
	//printf("set_papi_overflow() PAPI_start(EventSet), PAPI_OK\n");
}

void static remove_papi_overflow()
{
	int Events[NUM_EVENTS] , i;
	Events[0] = PAPI_TOT_CYC;

	TRY(PAPI_stop(EventSet, NULL), PAPI_OK);

	TRY(PAPI_overflow(EventSet, PAPI_TOT_CYC, 0, 0, papi_handler), PAPI_OK);

	TRY(PAPI_remove_events(EventSet, (int *)Events, NUM_EVENTS), PAPI_OK);

	TRY(PAPI_destroy_eventset(&EventSet), PAPI_OK);
}

static void* resolve_symbol(const char* symbol_name, int config) 
{
  void* result;
  if (config == RESOLVE_SYMBOL_VERSIONED) {
    result = dlvsym(RTLD_NEXT, symbol_name, PTHREAD_VERSION);
    if(result == NULL) {
      LOG_ERROR("Unable to resolve symbol %s@%s\n", symbol_name, PTHREAD_VERSION);
      //exit(1);
    }
  }else if(config == RESOLVE_SYMBOL_UNVERSIONED) {
    result = dlsym(RTLD_NEXT, symbol_name);
    if(result == NULL) {
      LOG_ERROR("Unable to resolve symbol %s\n", symbol_name);
      //exit(1);
    }
  }
  return result;
}

static void init_mock() __attribute__((constructor));
static void fini_mock() __attribute__((destructor));


static void init_mock() {
  if(module_init == MODULE_INITED) return ;
  original_GOMP_parallel = resolve_symbol("GOMP_parallel", RESOLVE_SYMBOL_UNVERSIONED);
  module_init = MODULE_INITED;
  //unsetenv("LD_PRELOAD");

  // setup tree data structure 
  //sample_count_tree = tree_new_node();
  thread_global_id = 0;
  thread_gid = new_thread_gid();

  // PAPI setup for main thread
  char* str = getenv("SAMPLE_COUNT");
  SAMPLE_COUNT = (str ? atoi(str) : DEFAULT_SAMPLE_COUNT)*2500;
  LOG_INFO("SET sample interval to %d * 2500 cycles\n", SAMPLE_COUNT/2500);
  TRY(PAPI_library_init(PAPI_VER_CURRENT), PAPI_VER_CURRENT);
  TRY(PAPI_thread_init(pthread_self), PAPI_OK);
  set_papi_overflow();
}

static void fini_mock()
{
  TRY(PAPI_stop(EventSet, NULL), PAPI_OK);
  write_addr_log();

  pid_t pid = getpid();
	char cmdline_str[45] = {0};
  char pid_str[12] = {0};
  sprintf(pid_str, "%d", pid);
  char mpi_rank_str[6] = {0};
  sprintf(mpi_rank_str, "%d", mpiRank);

  strcpy(cmdline_str, "cat /proc/");
  strcat(cmdline_str, pid_str);
  strcat(cmdline_str, "/maps > LDMAP");
  strcat(cmdline_str, mpi_rank_str);
  strcat(cmdline_str, ".TXT");
  printf("%s\n",cmdline_str);

  //string str = string("cat /proc/") + to_string(pid) + string("/maps > LDMAP") + to_string(mpiRank) + string(".TXT");
	system(cmdline_str);
}

static void *fn_wrapper (void* arg) {
  struct fn_wrapper_arg* args_ = arg;
  void *(*fn)(void*) = args_ -> fn;
  void *data = args_ -> data;

  //sample_count_tree = tree_new_node();
  thread_gid = new_thread_gid();
  LOG_INFO("Thread Start, thread_gid = %d\n", thread_gid);
  TRY(PAPI_register_thread(), PAPI_OK);
  set_papi_overflow();

  void * ret = fn(data); // acutally launch new fn

  remove_papi_overflow();
  //TRY(PAPI_stop(EventSet, NULL), PAPI_OK);
  TRY(PAPI_unregister_thread(), PAPI_OK);
  close_thread_gid();
  LOG_INFO("Thread Finish, thread_gid = %d\n", thread_gid);
  //tree_print(sample_count_tree, thread_gid);

  return ret;
}

void GOMP_parallel (void *(*fn) (void *), void *data, unsigned num_threads, unsigned int flags)
{
  if(module_init != MODULE_INITED) init_mock();
  struct fn_wrapper_arg* arg = malloc(sizeof(struct fn_wrapper_arg));
  arg->fn = fn;
  arg->data = data;

  //TRY(PAPI_stop(EventSet, NULL), PAPI_OK);
  remove_papi_overflow();
  (*original_GOMP_parallel)(fn_wrapper, arg, num_threads, flags);
  set_papi_overflow();
}