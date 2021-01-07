#include <iostream>
#include <fstream>
#include <mpi.h>
#include <papi.h>
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

int counter = 0, 
int rank = 0;
bool flag = true;

void code_to_inject_entry(){
  if (flag){
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    flag = false;
  }
  counter ++;
}

void code_to_inject_exit(){

}

void code_to_inject_loop_entry(char* str){
  LOG_INFO("[Rank %d] Here is %s entry\n", str)
}

void code_to_inject_loop_exit(char* str){
  LOG_INFO("[Rank %d] Here is %s exit\n", str)
}

void code_to_inject_loop_start(char* str){
  //LOG_INFO("[Rank %d] Here is %s start\n", str)
}

void code_to_inject_loop_end(char* str){
  //LOG_INFO("[Rank %d] Here is %s end\n", str)
}


void print_at_prog_exit(){
  LOG_INFO("[Rank %d] instrumented function is invoked by %d times\n", rank, counter)

  //LOG_INFO("Rank: %d TOT_INS = %lld, TOT_CYC = %lld, TOT_LST = %lld, L2_DCM = %lld \n",rank,total_pmu_count,total_pmu_count_1,total_pmu_count_2, total_pmu_count_3);

}


