#define _GNU_SOURCE

#include "omp.h"
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include "baguatool.h"
#include "dbg.h"

#define MODULE_INITED 1
#define RESOLVE_SYMBOL_VERSIONED 1
#define RESOLVE_SYMBOL_UNVERSIONED 2
#define PTHREAD_VERSION "GLIBC_2.3.2"

#define NUM_EVENTS 1

#define MAX_CALL_PATH_DEPTH 100

std::unique_ptr<baguatool::core::PerfData> perf_data = nullptr;

std::unique_ptr<baguatool::collector::Sampler> sampler = nullptr;

static void (*original_GOMP_parallel)(void *(*fn)(void *), void *data, unsigned num_threads, unsigned int flags) = NULL;

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

void print_thread_id(pthread_t id) {
  size_t i;
  for (i = sizeof(i); i; --i) printf("%02x", *(((unsigned char *)&id) + i - 1));
}

void RecordCallPath(int y) {
  baguatool::type::addr_t call_path[MAX_CALL_PATH_DEPTH] = {0};
  int call_path_len = sampler->GetBacktrace(call_path, MAX_CALL_PATH_DEPTH);
  perf_data->RecordVertexData(call_path, call_path_len, 0 /* process_id */, thread_gid /* thread_id */, 1);
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

  sampler = std::make_unique<baguatool::collector::Sampler>();
  // TODO one perf_data corresponds to one metric, export it to an array
  perf_data = std::make_unique<baguatool::core::PerfData>();

  original_GOMP_parallel =
      (decltype(original_GOMP_parallel))resolve_symbol("GOMP_parallel", RESOLVE_SYMBOL_UNVERSIONED);

  thread_global_id = 0;
  thread_gid = new_thread_gid();

  sampler->Setup();
  sampler->SetSamplingFreq(CYC_SAMPLE_COUNT);

  void (*RecordCallPathPointer)(int) = &(RecordCallPath);
  sampler->SetOverflow(RecordCallPathPointer);

  sampler->Start();
}

// User-defined what to do at destructor
static void fini_mock() {
  sampler->Stop();
  perf_data->Dump("SAMPLE.TXT");
  // sampler->RecordLdLib();
}

struct fn_wrapper_arg {
  void *(*fn)(void *);
  void *data;
};

static void *fn_wrapper(void *arg) {
  struct fn_wrapper_arg *args_ = (struct fn_wrapper_arg *)arg;
  void *(*fn)(void *) = args_->fn;
  void *data = args_->data;

  dbg("here");

  // TRY(PAPI_register_thread(), PAPI_OK);
  // set_papi_overflow();

  thread_gid = new_thread_gid();
  LOG_INFO("Thread Start, thread_gid = %d\n", thread_gid);
  // args_->create_thread_id = thread_gid;
  sampler->AddThread();
  sampler->SetOverflow(&RecordCallPath);
  sampler->Start();

  // void * ret = fn(data); // acutally launch new fn
  void *ret = nullptr;

  sampler->Stop();
  sampler->UnsetOverflow();
  sampler->RemoveThread();
  close_thread_gid();
  LOG_INFO("Thread Finish, thread_gid = %d\n", thread_gid);

  return ret;
}

void GOMP_parallel(void *(*fn)(void *), void *data, unsigned num_threads, unsigned int flags) {
  dbg("here");
  if (module_init != MODULE_INITED) {
    init_mock();
  }
  struct fn_wrapper_arg *arg = new (struct fn_wrapper_arg)();
  arg->fn = fn;
  arg->data = data;

  // TRY(PAPI_stop(EventSet, NULL), PAPI_OK);
  // remove_papi_overflow();
  sampler->Stop();
  sampler->UnsetOverflow();
  (*original_GOMP_parallel)(fn_wrapper, arg, num_threads, flags);
  sampler->SetOverflow(&RecordCallPath);
  sampler->Start();
  // set_papi_overflow();
}