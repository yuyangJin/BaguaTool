#define _GNU_SOURCE
#include <dlfcn.h>
#include <omp.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "baguatool.h"
#include "dbg.h"
#include "mpi_init.h"
#include "omp_init.h"

#define MODULE_INITED 1
#define RESOLVE_SYMBOL_VERSIONED 1
#define RESOLVE_SYMBOL_UNVERSIONED 2
#define PTHREAD_VERSION "GLIBC_2.3.2"
#define NUM_EVENTS 1
#define MAX_CALL_PATH_DEPTH 100
#define MAX_THREAD_PER_PROCS 65530  // cat /proc/sys/vm/max_map_count

#define gettid() syscall(__NR_gettid)

std::unique_ptr<baguatool::collector::Sampler> sampler = nullptr;
std::unique_ptr<baguatool::core::PerfData> perf_data = nullptr;
static void (*original_GOMP_parallel)(void (*fn)(void *), void *data, unsigned num_threads, unsigned int flags) = NULL;

static int CYC_SAMPLE_COUNT = 0;
static int module_init = 0;

int mpi_rank = 0;
static __thread int thread_gid;
static int main_thread_gid;
static int main_tid;
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
int get_thread_gid() { return thread_global_id; }

void RecordCallPath(int y) {
  baguatool::type::addr_t call_path[MAX_CALL_PATH_DEPTH] = {0};
  int call_path_len = sampler->GetBacktrace(call_path, MAX_CALL_PATH_DEPTH);
  if (main_tid != gettid()) {
    perf_data->RecordVertexData(call_path, call_path_len, mpi_rank /* process_id */, thread_gid /* thread_id */, 1);
  } else {
    perf_data->RecordVertexData(call_path, call_path_len, mpi_rank /* process_id */, main_thread_gid /* thread_id */,
                                1);
  }
}

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

/** User-defined what to do at constructor */
static void init_mock() {
  if (module_init == MODULE_INITED) {
    return;
  }

  // TODO one perf_data corresponds to one metric, export it to an array
  sampler = std::make_unique<baguatool::collector::Sampler>();
  perf_data = std::make_unique<baguatool::core::PerfData>();

  original_GOMP_parallel =
      (decltype(original_GOMP_parallel))resolve_symbol("GOMP_parallel", RESOLVE_SYMBOL_UNVERSIONED);
  printf("original_GOMP_parallel = %p\n", original_GOMP_parallel);
  module_init = MODULE_INITED;

  thread_global_id = 0;
  main_thread_gid = new_thread_gid();
  main_tid = gettid();

  sampler->Setup();
  sampler->SetSamplingFreq(CYC_SAMPLE_COUNT);

  void (*RecordCallPathPointer)(int) = &(RecordCallPath);
  sampler->SetOverflow(RecordCallPathPointer);
  sampler->Start();
}

/** User-defined what to do at destructor */
static void fini_mock() {
  sampler->Stop();
  perf_data->Dump("SAMPLE.TXT");

  // sampler->RecordLdLib();
}

/** -------------------------------------------------------------------------
 * For OpenMP
 * --------------------------------------------------------------------------
*/

struct fn_wrapper_arg {
  void (*fn)(void *);
  void *data;
  baguatool::type::addr_t call_path[MAX_CALL_PATH_DEPTH] = {0};
  int call_path_len;
};

/** fn wrapper
 * one of openmp threads is the main thread
 */
static void fn_wrapper(void *arg) {
  struct fn_wrapper_arg *args_ = (struct fn_wrapper_arg *)arg;
  void (*fn)(void *) = args_->fn;
  void *data = args_->data;

  thread_gid = new_thread_gid();
  LOG_INFO("Thread Start, thread_gid = %d\n", thread_gid);

  /** recording which GOMP_parallel create which threads */
  if (main_tid != gettid()) {
    dbg(thread_gid);
    perf_data->RecordEdgeData(args_->call_path, args_->call_path_len, (baguatool::type::addr_t *)nullptr, 0, mpi_rank,
                              mpi_rank, main_thread_gid, thread_gid, -2);
  }
  sampler->AddThread();
  sampler->SetOverflow(&RecordCallPath);
  sampler->Start();

  /** ------------------------- */
  /** execute real fn */
  fn(data);  // acutally launch fn
  /** ------------------------- */

  sampler->Stop();
  sampler->UnsetOverflow();
  sampler->RemoveThread();

  // close_thread_gid();
  LOG_INFO("Thread Finish, thread_gid = %d\n", thread_gid);

  return;
}

void GOMP_parallel(void (*fn)(void *), void *data, unsigned num_threads, unsigned int flags) {
  if (module_init != MODULE_INITED) {
    init_mock();
  }
  sampler->Stop();
  sampler->UnsetOverflow();

  /** ------------------------------------------------------------------------- */
  struct fn_wrapper_arg *arg = new (struct fn_wrapper_arg)();
  arg->fn = fn;
  arg->data = data;
  arg->call_path_len = sampler->GetBacktrace(arg->call_path, MAX_CALL_PATH_DEPTH);
  /** execute real GOMP_parallel */
  (*original_GOMP_parallel)(fn_wrapper, arg, num_threads, flags);
  /** ------------------------------------------------------------------------- */

  sampler->SetOverflow(&RecordCallPath);
  sampler->Start();

  delete arg;
}