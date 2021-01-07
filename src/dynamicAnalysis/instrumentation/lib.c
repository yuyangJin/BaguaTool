#include <iostream>
#include <string.h>
#include <string>
#include <fstream>
#include <map>
#include <mpi.h>
#include <chrono>
#include <papi.h>
//#include <papi.h>
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

#define TRY(func, flag) \
{ \
        int retval = func;\
        if (retval != flag) LOG_ERROR("%s, ErrCode: %d\n", #func, retval);\
}



#define NUM_EVENTS 1
char EventName[NUM_EVENTS][20] = {
	"PAPI_TOT_CYC",
	// PAPI_FP_INS is unavailable
	//"PAPI_LST_INS",
	//"PAPI_L1_DCM",
	//"PAPI_L2_DCM",
	//"PAPI_L2_DCA",
	//"PAPI_L3_DCA"
};
int EventCode[NUM_EVENTS] = {PAPI_TOT_CYC}; //, PAPI_LST_INS, PAPI_L1_DCM, PAPI_L2_DCM, PAPI_L2_DCA, PAPI_L3_DCA};
//int EventCode[NUM_EVENTS] = {PAPI_TOT_INS, PAPI_TOT_INS, PAPI_TOT_INS, PAPI_TOT_INS, PAPI_TOT_INS, PAPI_TOT_INS}; 
//long long int values[NUM_EVENTS], count[NUM_EVENTS];
int EventSet = PAPI_NULL, counter = 0;

//map< string , int> loops_counter;
map<string, long long int* > loops_pmu_counter;
map<string, long long int* > loops_pmu_value;

map<string, int> loops_counter;

// map<string, chrono::system_clock::time_point> loops_st;
// map<string, chrono::system_clock::time_point> loops_ed;
// map<string, double> loops_time;

int myrank = 0;
bool flag = true;
chrono::system_clock::time_point st,ed;
double func_time = 0.0;


void code_to_inject_entry(){
  if (flag){
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    flag = false;
    int num;
    if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT) 
      fprintf(stderr, "PAPI library init error!\n");
    if ((num = PAPI_get_opt(PAPI_MAX_HWCTRS,NULL)) <= 0){
      LOG_ERROR("Failed to get counter number %d \n",num);
    }
    TRY(PAPI_create_eventset(&EventSet), PAPI_OK);
    TRY(PAPI_add_events(EventSet, EventCode, NUM_EVENTS), PAPI_OK);
    TRY(PAPI_start(EventSet), PAPI_OK);
  }
  counter++;
  // TRY(PAPI_read(EventSet,values), PAPI_OK);
  // for (int i = 0; i < NUM_EVENTS; ++i)
  // 	count[i] -= values[i];
  st = chrono::system_clock::now();
}

void code_to_inject_exit(){
  ed = chrono::system_clock::now();
  func_time += chrono::duration_cast<chrono::microseconds>(ed - st).count();
  // TRY(PAPI_read(EventSet, values), PAPI_OK);
  // for (int i = 0; i < NUM_EVENTS; ++i)
  //   count[i] += values[i];
}

void code_to_inject_loop_entry(char* str){
  //LOG_INFO("Here is %s entry.\n", str)
  /* 
  if (loops_counter.find(str) == loops_counter.end()){
    loops_counter[str] = 1;
  }else{
    loops_counter[str]++;
  }
  */
  // string x = string(str);
  // for( map< string , long long int *>::iterator iter=loops_pmu_counter.begin(); iter != loops_pmu_counter.end(); iter++) {
  //   if (x.compare(iter->first) == 0){
  //     for( map< string , long long int *>::iterator iter_1=loops_pmu_value.begin(); iter_1 != loops_pmu_value.end(); iter_1++) {
  //       if (x.compare(iter_1->first) == 0){
  //         TRY(PAPI_read(EventSet, iter_1->second), PAPI_OK);
  //         for (int i = 0; i < NUM_EVENTS; ++i)
  //           iter->second[i] -= iter_1->second[i];
  //         return ;
  //       }
  //     }
  //   }
  // }
  // long long int * values = (long long int *) malloc(NUM_EVENTS * sizeof(long long int));
  // long long int * count = (long long int *) malloc(NUM_EVENTS * sizeof(long long int));
  // memset(values, 0, NUM_EVENTS * sizeof(long long int));
  // memset(count, 0, NUM_EVENTS * sizeof(long long int));
  // TRY(PAPI_read(EventSet, values), PAPI_OK);
  // for (int i = 0; i < NUM_EVENTS; ++i)
  // 	count[i] -= values[i];
  // loops_pmu_counter.insert ( pair<string,long long int *>(x,count) );
  // loops_pmu_value.insert ( pair<string,long long int *>(x,values) );
}

void code_to_inject_loop_exit(char* str){

  //LOG_INFO("Here is %s exit.\n", str)
  // string x = string(str);
  // for( map< string , long long int *>::iterator iter=loops_pmu_counter.begin(); iter != loops_pmu_counter.end(); iter++) {
  //   if (x.compare(iter->first) == 0){
  //     for( map< string , long long int *>::iterator iter_1=loops_pmu_value.begin(); iter_1 != loops_pmu_value.end(); iter_1++) {
  //       if (x.compare(iter_1->first) == 0){
  //         TRY(PAPI_read(EventSet, iter_1->second), PAPI_OK);
  //         for (int i = 0; i < NUM_EVENTS; ++i)
  // 	        iter->second[i] += iter_1->second[i];
  //         return ;
  //       }
  //     }
  //   }
  // }

}

void code_to_inject_loop_start(const char* str){
  string x = string(str);
  for( map< string ,int>::iterator iter=loops_counter.begin(); iter != loops_counter.end(); iter++) {
    if (x.compare(iter->first) == 0){
      iter->second ++;
      return ;
    }
  }

  loops_counter.insert ( pair<string,int>(x,1) );
}

void code_to_inject_loop_end(char* str){
  ;//LOG_INFO("[Rank %d] Here is %s end\n", str)
}


void print_at_prog_exit(){
  //LOG_INFO("instrumented function is invoked by %d times.\n", counter)

  //LOG_INFO("Rank: %d TOT_INS = %lld, TOT_CYC = %lld, TOT_LST = %lld, L2_DCM = %lld \n",rank,total_pmu_count,total_pmu_count_1,total_pmu_count_2, total_pmu_count_3);
  //unordered_map<char*,int>::iterator iter;
  if (myrank == 0){
    for(map<string,int>::iterator iter=loops_counter.begin(); iter!=loops_counter.end() ; iter++){
    //for(map<char*,int>::iterator iter=loops_counter.begin(); iter!=loops_counter.end() ; iter++){
      LOG_INFO("[Rank 0] %s is executed for %d times.\n",iter->first.c_str(), iter->second);
    }
    for(map<string,long long int*>::iterator iter=loops_pmu_counter.begin(); iter!=loops_pmu_counter.end() ; iter++){
    //for(map<char*,int>::iterator iter=loops_counter.begin(); iter!=loops_counter.end() ; iter++){
      for (int i = 0; i < NUM_EVENTS; ++i){
        LOG_INFO("[Rank %d] %s - %s: %lld\n", myrank, iter->first.c_str() ,EventName[i], iter->second[i])
      }
    }
    LOG_INFO("[Rank 0] function execution time is %.2f ms.\n",func_time*1e-3);
    LOG_INFO("[Rank 0] instrumented function is invoked by %d times.\n", counter)
    //for (int i = 0; i < NUM_EVENTS; ++i)
    //  LOG_INFO("[Rank %d] %s: %lld\n", myrank, EventName[i], count[i])
  }
}


