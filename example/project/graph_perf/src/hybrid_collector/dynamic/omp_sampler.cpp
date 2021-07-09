#define BAGUA
#define _GNU_SOURCE
#include <dlfcn.h>
#include <omp.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#ifdef BAGUA
#include "baguatool.h"
#include "dbg.h"
#endif
#include <unistd.h>
#include "omp_init.h"

#define MODULE_INITED 1
#define RESOLVE_SYMBOL_VERSIONED 1
#define RESOLVE_SYMBOL_UNVERSIONED 2
#define PTHREAD_VERSION "GLIBC_2.3.2"

#define NUM_EVENTS 1

#define MAX_CALL_PATH_DEPTH 100

#ifdef BAGUA
baguatool::core::PerfData *perf_data = nullptr;
baguatool::collector::Sampler *sampler = nullptr;
#endif

static void (*original_GOMP_parallel)(void (*fn)(void *), void *data, unsigned num_threads, unsigned int flags) = NULL;

static int CYC_SAMPLE_COUNT = 0;
static int module_init = 0;

int mpi_rank = 0;

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

#ifdef BAGUA
void RecordCallPath(int y) {
  baguatool::type::addr_t call_path[MAX_CALL_PATH_DEPTH] = {0};
  int call_path_len = sampler->GetBacktrace(call_path, MAX_CALL_PATH_DEPTH);
  perf_data->RecordVertexData(call_path, call_path_len, 0 /* process_id */, thread_gid /* thread_id */, 1);
}
#endif

static void *resolve_symbol(const char *symbol_name, int config) {
  void *result;
  static void *handle = NULL;
  if (config == RESOLVE_SYMBOL_VERSIONED) {
    result = dlvsym(RTLD_NEXT, symbol_name, PTHREAD_VERSION);
    if (result == NULL) {
      LOG_ERROR("Unable to resolve symbol %s@%s\n", symbol_name, PTHREAD_VERSION);
      // exit(1);
    }
  } else if (config == RESOLVE_SYMBOL_UNVERSIONED) {
    handle = dlopen("libgomp.so.1", RTLD_LAZY);
    result = dlsym(handle, symbol_name);
    // result = dlsym(RTLD_NEXT, symbol_name);
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
  if (module_init == MODULE_INITED) {
    return;
  }

#ifdef BAGUA
  // sampler = (baguatool::collector::Sampler*)malloc( sizeof(baguatool::collector::Sampler));
  sampler = new baguatool::collector::Sampler();
  // sampler->Sampler();
  // TODO one perf_data corresponds to one metric, export it to an array
  // perf_data = (baguatool::core::PerfData*) malloc( sizeof(baguatool::core::PerfData ));
  perf_data = new baguatool::core::PerfData();
  // perf_data->PerfData();
  dbg("here");
#endif

  original_GOMP_parallel =
      (decltype(original_GOMP_parallel))resolve_symbol("GOMP_parallel", RESOLVE_SYMBOL_UNVERSIONED);
  printf("original_GOMP_parallel = %p\n", original_GOMP_parallel);
  module_init = MODULE_INITED;

  thread_global_id = 0;
  thread_gid = new_thread_gid();

#ifdef BAGUA
  sampler->Setup();
  sampler->SetSamplingFreq(CYC_SAMPLE_COUNT);

  void (*RecordCallPathPointer)(int) = &(RecordCallPath);
  sampler->SetOverflow(RecordCallPathPointer);
  sampler->Start();
#endif
}

// User-defined what to do at destructor
static void fini_mock() {
#ifdef BAGUA
  sampler->Stop();
  perf_data->Dump("SAMPLE.TXT");
#endif
  // sampler->~Sampler();
  // perf_data->~PerfData();
  // free();
  // sampler->RecordLdLib();
}

struct fn_wrapper_arg {
  void (*fn)(void *);
  void *data;
};

static void fn_wrapper(void *arg) {
  struct fn_wrapper_arg *args_ = (struct fn_wrapper_arg *)arg;
  void (*fn)(void *) = args_->fn;
  void *data = args_->data;

  // TRY(PAPI_register_thread(), PAPI_OK);
  // set_papi_overflow();

  thread_gid = new_thread_gid();
  printf("Thread Start, thread_gid = %d\n", thread_gid);
// args_->create_thread_id = thread_gid;

#ifdef BAGUA
  dbg("here");
  sampler->AddThread();
  sampler->SetOverflow(&RecordCallPath);
  sampler->Start();
#endif

  // void * ret = fn(data); // acutally launch new fn
  // void *ret = nullptr;

  fn(data);  // acutally launch new fn
#ifdef BAGUA
  sampler->Stop();
  sampler->UnsetOverflow();
  sampler->RemoveThread();
#endif

  close_thread_gid();
  printf("Thread Finish, thread_gid = %d\n", thread_gid);

  // return ret;
  return;
}

void GOMP_parallel(void (*fn)(void *), void *data, unsigned num_threads, unsigned int flags) {
  if (module_init != MODULE_INITED) {
    init_mock();
  }
  struct fn_wrapper_arg *arg = (struct fn_wrapper_arg *)malloc(sizeof(struct fn_wrapper_arg));
  arg->fn = fn;
  arg->data = data;

// TRY(PAPI_stop(EventSet, NULL), PAPI_OK);
// remove_papi_overflow();
// GOMP_parallel_start(fn_wrapper, arg, num_threads);

#ifdef BAGUA
  dbg("here");
  sampler->Stop();
  sampler->UnsetOverflow();
#endif

  (*original_GOMP_parallel)(fn_wrapper, arg, num_threads, flags);

#ifdef BAGUA
  sampler->SetOverflow(&RecordCallPath);
  sampler->Start();
#endif
  // GOMP_parallel_end();
  // set_papi_overflow();
  free(arg);
}