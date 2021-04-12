#include <stdio.h>
#include <string.h>
#include "baguatool.h"

#define MODULE_INITED 1

#ifndef ADDR_LOG_SIZE
#define ADDR_LOG_SIZE 80000
#endif

#define NUM_EVENTS 1

#define MAX_CALL_PATH_LEN 1300  // MAX_STACK_DEPTH * 13

// struct for openmp instrumentation OMP_parallel
// struct fn_wrapper_arg {
//   void *(*fn)(void*);
//   void *data;
// };

typedef struct callPathStruct {
  char call_path_str[MAX_CALL_PATH_LEN];  //= {0};
  // int thread_id ; //= -1;
  unsigned long long int count;  //= 0;
} CPS;

// static void (*original_GOMP_parallel)(void *(*fn) (void *), void *data, unsigned num_threads, unsigned int flags) =
// nullptr;

std::unique_ptr<baguatool::graph_sd::Sampler> sampler = nullptr;

bool first_flag = true;

FILE *(fp)[NUM_EVENTS] = {NULL};

static int CYC_SAMPLE_COUNT = 0;
static int module_init = 0;

static CPS call_path_addr_log[NUM_EVENTS][ADDR_LOG_SIZE];  // (20 * NUM_EVENTS) MB
static int call_path_addr_log_pointer[NUM_EVENTS] = {0};

int mpiRank = -1;

// static __thread int thread_gid;
// //thread_local int thread_id;
// static int thread_global_id;

// int new_thread_gid()
// {
//   thread_gid = __sync_fetch_and_add(&thread_global_id, 1);
//   //LOG_INFO("GET thread_gid = %d\n", thread_gid);
//   return thread_gid;
// }
// void close_thread_gid()
// {
//   thread_gid = __sync_sub_and_fetch(&thread_global_id, 1);
//   //LOG_INFO("GET thread_gid = %d\n", thread_gid);
//   return ;
// }

static void DumpAddrLog(bool last_flag) {
  if (first_flag == true) {
    first_flag = false;
    for (int i = 0; i < NUM_EVENTS; i++) {
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

      fp[i] = fopen(file_name, "w");

      if (!fp[i]) {
        LOG_INFO("Failed to open %s\n", file_name);
        call_path_addr_log_pointer[i] = __sync_and_and_fetch(&call_path_addr_log_pointer[i], 0);
        return;
      }
    }
  }

  for (int i = 0; i < NUM_EVENTS; i++) {
    LOG_INFO("Rank %d : WRITE %d ADDR to %d TXT\n", mpiRank, call_path_addr_log_pointer[i], i);
    for (int j = 0; j < call_path_addr_log_pointer[i]; j++) {
      // fprintf(fp[i], "%s | %lld | %d\n", call_path_addr_log[i][j].call_path_str,
      //        call_path_addr_log[i][j].count, call_path_addr_log[i][j].thread_id);
      fprintf(fp[i], "%s | %lld\n", call_path_addr_log[i][j].call_path_str, call_path_addr_log[i][j].count);
      fflush(fp[i]);
    }
    call_path_addr_log_pointer[i] = __sync_and_and_fetch(&call_path_addr_log_pointer[i], 0);
  }

  if (last_flag == true) {
    for (int i = 0; i < NUM_EVENTS; i++) {
      fclose(fp[i]);
    }
  }
}

void RecordCallPath(int y) {
  char call_path_str[MAX_CALL_PATH_LEN] = {0};

  sampler->GetBacktrace(call_path_str, MAX_CALL_PATH_LEN);

  bool hasRecordFlag = false;

  for (int i = 0; i < call_path_addr_log_pointer[y]; i++) {
    if (strcmp(call_path_str, call_path_addr_log[y][i].call_path_str) ==
        0) {  // && call_path_addr_log[y][i].thread_id == thread_gid){
      call_path_addr_log[y][i].count++;
      hasRecordFlag = true;
      break;
    }
  }

  if (hasRecordFlag == false) {
    if (strlen(call_path_str) >= MAX_CALL_PATH_LEN) {
      // LOG_WARN("Call path string length (%d) is longer than %d", strlen(call_path_str),MAX_CALL_PATH_LEN);
    } else {
      strcpy(call_path_addr_log[y][call_path_addr_log_pointer[y]].call_path_str, call_path_str);
      // LOG_INFO("call_path_addr_log_pointer : %lld\n",call_path_addr_log_pointer[y]);
      call_path_addr_log[y][call_path_addr_log_pointer[y]].count = 1;
      // call_path_addr_log[y][call_path_addr_log_pointer[y]].thread_id = thread_gid;
      unsigned long long int x = __sync_fetch_and_add(&call_path_addr_log_pointer[y], 1);
      // call_path_addr_log_pointer[y] ++;
    }
  }

  if (call_path_addr_log_pointer[y] >= ADDR_LOG_SIZE - 10) {
    DumpAddrLog(false);
  }
}

static void init_mock() __attribute__((constructor));
static void fini_mock() __attribute__((destructor));

// User-defined what to do at constructor
static void init_mock() {
  if (module_init == MODULE_INITED) return;
  module_init = MODULE_INITED;

  // sampler = new Sampler();
  sampler = std::make_unique<baguatool::graph_sd::Sampler>();

  // original_GOMP_parallel = resolve_symbol("GOMP_parallel", RESOLVE_SYMBOL_UNVERSIONED);

  // thread_global_id = 0;
  // thread_gid = new_thread_gid();

  // char* str = getenv("CYC_SAMPLE_COUNT");
  // CYC_SAMPLE_COUNT = atoi(str);

  sampler->SetSamplingFreq(CYC_SAMPLE_COUNT);
  sampler->Setup();

  void (*RecordCallPathPointer)(int) = &(RecordCallPath);
  sampler->SetOverflow(RecordCallPathPointer);

  sampler->Start();

  //   original_GOMP_parallel = resolve_symbol("GOMP_parallel", RESOLVE_SYMBOL_UNVERSIONED);

  //   //unsetenv("LD_PRELOAD");

  //   // setup tree data structure
  //   //sample_count_tree = tree_new_node();
  //   thread_global_id = 0;
  //   thread_gid = new_thread_gid();

  //   // PAPI setup for main thread
  //   char* str = getenv("CYC_SAMPLE_COUNT");
  //   CYC_SAMPLE_COUNT = (str ? atoi(str) : DEFAULT_CYC_SAMPLE_COUNT);
  //   LOG_INFO("SET sample interval to %d cycles\n", CYC_SAMPLE_COUNT);

  //   TRY(PAPI_library_init(PAPI_VER_CURRENT), PAPI_VER_CURRENT);
  //   TRY(PAPI_thread_init(pthread_self), PAPI_OK);
  //   set_papi_overflow();
}

// User-defined what to do at destructor
static void fini_mock() {
  sampler->Stop();

  // sampler->RecordLdLib();

  DumpAddrLog(true);

  //   TRY(PAPI_stop(EventSet, NULL), PAPI_OK);
  //   write_addr_log();

  //   pid_t pid = getpid();
  //   char cmdline_str[45] = {0};
  //   char pid_str[12] = {0};
  //   sprintf(pid_str, "%d", pid);
  //   char mpi_rank_str[6] = {0};
  //   sprintf(mpi_rank_str, "%d", mpiRank);

  //   strcpy(cmdline_str, "cat /proc/");
  //   strcat(cmdline_str, pid_str);
  //   strcat(cmdline_str, "/maps > LDMAP");
  //   strcat(cmdline_str, mpi_rank_str);
  //   strcat(cmdline_str, ".TXT");
  //   printf("%s\n",cmdline_str);

  //   //string str = string("cat /proc/") + to_string(pid) + string("/maps > LDMAP") + to_string(mpiRank) +
  //   string(".TXT");
  // 	system(cmdline_str);
}
