#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "baguatool.h"
#include "dbg.h"

#define MODULE_INITED 1
#define RESOLVE_SYMBOL_VERSIONED 1
#define RESOLVE_SYMBOL_UNVERSIONED 2
#define PTHREAD_VERSION "GLIBC_2.3.2"

#define NUM_EVENTS 1

#define MAX_CALL_PATH_DEPTH 100

// struct for pthread instrumentation start_routine
struct start_routine_wrapper_arg {
  void *(*start_routine)(void *);
  void *real_arg;
};

static int (*original_pthread_create)(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *),
                                      void *arg) = NULL;
static int (*original_pthread_mutex_lock)(pthread_mutex_t *thread) = NULL;
static int (*original_pthread_mutex_unlock)(pthread_mutex_t *thread) = NULL;
static int (*original_pthread_join)(pthread_t thread, void **value_ptr) = NULL;

std::unique_ptr<baguatool::graph_sd::Sampler> sampler = nullptr;
std::unique_ptr<baguatool::core::PerfData> perf_data = nullptr;
std::unique_ptr<baguatool::core::PerfData> perf_data_pthread = nullptr;
std::map<pthread_mutex_t *, std::string> mutex_to_lock_callsite;

static int CYC_SAMPLE_COUNT = 0;
static int module_init = 0;

static __thread int thread_gid;
// thread_local int thread_id;
static int thread_global_id;

int new_thread_gid() {
  thread_gid = __sync_fetch_and_add(&thread_global_id, 1);
  // LOG_INFO("GET thread_gid = %d\n", thread_gid);
  return thread_gid;
}
void close_thread_gid() {
  thread_gid = __sync_sub_and_fetch(&thread_global_id, 1);
  // LOG_INFO("GET thread_gid = %d\n", thread_gid);
  return;
}

void print_thread_id(pthread_t id) {
  size_t i;
  for (i = sizeof(i); i; --i) printf("%02x", *(((unsigned char *)&id) + i - 1));
}

void RecordCallPath(int y) {
  baguatool::graph_sd::addr_t call_path[MAX_CALL_PATH_DEPTH] = {0};

  int call_path_len = sampler->GetBacktrace(call_path, MAX_CALL_PATH_DEPTH);

  perf_data->Record(call_path, call_path_len, 0, thread_gid, 1);
}

static void *resolve_symbol(const char *symbol_name, int config) {
  void *result;
  if (config == RESOLVE_SYMBOL_VERSIONED) {
    result = dlvsym(RTLD_NEXT, symbol_name, PTHREAD_VERSION);
    if (result == NULL) {
      LOG_ERROR("Unable to resolve symbol %s@%s\n", symbol_name, PTHREAD_VERSION);
      // exit(1);
    }
  } else if (config == RESOLVE_SYMBOL_UNVERSIONED) {
    result = dlsym(RTLD_NEXT, symbol_name);
    if (result == NULL) {
      LOG_ERROR("Unable to resolve symbol %s\n", symbol_name);
      // exit(1);
    }
  }
  return result;
}

static void init_mock() __attribute__((constructor));
static void fini_mock() __attribute__((destructor));

// User-defined what to do at constructor
static void init_mock() {
  if (module_init == MODULE_INITED) return;
  module_init = MODULE_INITED;

  // sampler = new Sampler();
  sampler = std::make_unique<baguatool::graph_sd::Sampler>();
  perf_data = std::make_unique<baguatool::core::PerfData>();
  perf_data_pthread = std::make_unique<baguatool::core::PerfData>();

  // original_GOMP_parallel = resolve_symbol("GOMP_parallel", RESOLVE_SYMBOL_UNVERSIONED);
  original_pthread_create =
      (decltype(original_pthread_create))resolve_symbol("pthread_create", RESOLVE_SYMBOL_UNVERSIONED);
  original_pthread_mutex_lock =
      (decltype(original_pthread_mutex_lock))resolve_symbol("pthread_mutex_lock", RESOLVE_SYMBOL_UNVERSIONED);
  original_pthread_mutex_unlock =
      (decltype(original_pthread_mutex_unlock))resolve_symbol("pthread_mutex_unlock", RESOLVE_SYMBOL_UNVERSIONED);
  original_pthread_join = (decltype(original_pthread_join))resolve_symbol("pthread_join", RESOLVE_SYMBOL_UNVERSIONED);

  thread_global_id = 0;
  thread_gid = new_thread_gid();

  // sampler setup for main thread
  sampler->SetSamplingFreq(CYC_SAMPLE_COUNT);
  sampler->Setup();

  void (*RecordCallPathPointer)(int) = &(RecordCallPath);
  sampler->SetOverflow(RecordCallPathPointer);

  sampler->Start();
}

// User-defined what to do at destructor
static void fini_mock() {
  sampler->Stop();

  // sampler->RecordLdLib();

  perf_data->Dump("SAMPLE.TXT");
  perf_data_pthread->Dump("PTHREAD.TXT");
}

static void *start_routine_wrapper(void *arg) {
  auto args_ = (start_routine_wrapper_arg *)arg;
  void *(*start_routine)(void *) = args_->start_routine;
  void *real_arg = args_->real_arg;

  thread_gid = new_thread_gid();
  LOG_INFO("Thread Start, thread_gid = %d\n", thread_gid);
  sampler->AddThread();
  sampler->SetOverflow(&RecordCallPath);
  sampler->Start();

  dbg(start_routine);

  void *ret = start_routine(real_arg);  // acutally launch new thread

  sampler->Stop();
  sampler->UnsetOverflow();
  sampler->RemoveThread();
  // close_thread_gid();
  LOG_INFO("Thread Finish, thread_gid = %d\n", thread_gid);

  return ret;
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *real_arg) {
  //    LOG_INFO("My pthread_create called by TID  %d, in_mutex %d, t_status %d\n", tid, in_mutex, tool_thread_status);
  if (module_init != MODULE_INITED) {
    init_mock();
  }
  // printf("create - ");
  // print_thread_id(*thread);
  // dbg("create",*thread);
  auto *arg = (start_routine_wrapper_arg *)malloc(sizeof(struct start_routine_wrapper_arg));
  arg->start_routine = start_routine;
  arg->real_arg = real_arg;

  // dbg(&start_routine, start_routine, *start_routine, *(*start_routine));
  int ret = (*original_pthread_create)(thread, attr, start_routine_wrapper, arg);
  baguatool::graph_sd::addr_t call_path[MAX_CALL_PATH_DEPTH] = {0};
  int call_path_len = sampler->GetBacktrace(call_path, MAX_CALL_PATH_DEPTH);
  dbg("create", *thread);

  perf_data_pthread->Record(call_path, call_path_len, (int)*thread, thread_gid, -1);

  return ret;
}

int pthread_join(pthread_t thread, void **value_ptr) {
  if (module_init != MODULE_INITED) {
    init_mock();
  }
  // printf("join - ");
  // print_thread_id(thread);
  dbg("join", thread);

  baguatool::graph_sd::addr_t call_path[MAX_CALL_PATH_DEPTH] = {0};
  int call_path_len = sampler->GetBacktrace(call_path, MAX_CALL_PATH_DEPTH);

  struct timeval start;
  struct timeval end;

  gettimeofday(&start, NULL);

  int ret = (*original_pthread_join)(thread, value_ptr);

  gettimeofday(&end, NULL);

  baguatool::core::perf_data_t time = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;

  perf_data_pthread->Record(call_path, call_path_len, (int)thread, thread_gid, time);
  return ret;
}

// int pthread_mutex_lock(pthread_mutex_t *thread) {
//   if (module_init != MODULE_INITED) {
//     init_mock();
//   }
//   // baguatool::graph_sd::addr_t call_path[MAX_CALL_PATH_DEPTH] = {0};
//   // int call_path_len = sampler->GetBacktrace(call_path, MAX_CALL_PATH_DEPTH);
//   // for (int i = 0; i < call_path_len; i++) {
//   //   printf("%x ", call_path[i]);
//   // }
//   // printf("\n");
//   //dbg(thread_gid, thread, &thread);
//   //mutex_to_lock_callsite[thread] = std::to_string(thread_gid);
//   int ret = (*original_pthread_mutex_lock)(thread);

//   dbg(thread_gid, thread->__data.__kind, thread->__data.__spins);

//   return ret;
// }

// int pthread_mutex_unlock(pthread_mutex_t *thread) {
//   if (module_init != MODULE_INITED) {
//     init_mock();
//   }
//   //dbg(thread_gid, thread, &thread);
//   int ret = (*original_pthread_mutex_unlock)(thread);
//   // baguatool::graph_sd::addr_t call_path[MAX_CALL_PATH_DEPTH] = {0};
//   // int call_path_len = sampler->GetBacktrace(call_path, MAX_CALL_PATH_DEPTH);
//   // for (int i = 0; i < call_path_len; i++) {
//   //   printf("%x ", call_path[i]);
//   // }
//   // printf("\n");
//   // for (auto& item: mutex_to_lock_callsite) {
//   //   if (*(item.first) == *thread) {
//   //     std::cout << item.second << endl;
//   //   }
//   // }
//   dbg(thread_gid, thread->__data.__kind, thread->__data.__spins);

//   return ret;
// }