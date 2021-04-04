#include "sampler.h"
#include "baguatool.h"

namespace baguatool::graph_sd {

Sampler::Sampler() { this->sa = std::make_unique<SamplerImpl>(); }

Sampler::~Sampler() {}

void Sampler::SetSamplingFreq(int freq) { sa->SetSamplingFreq(freq); }
void Sampler::Setup() { sa->Setup(); }
void Sampler::SetOverflow(void (*FUNC_AT_OVERFLOW)(int)) { sa->SetOverflow(FUNC_AT_OVERFLOW); }
void Sampler::Start() { sa->Start(); }
void Sampler::Stop() { sa->Stop(); }
int Sampler::GetOverflowEvent(LongLongVec* overflow_vector) { return sa->GetOverflowEvent(overflow_vector); }
void Sampler::GetBacktrace(char* call_path_str, int max_call_path_str_len) {
  sa->GetBacktrace(call_path_str, max_call_path_str_len);
}
// int Sampler::my_backtrace(unw_word_t *buffer, int max_depth) {my_backtrace(buffer, max_depth); }
// static void Sampler:: papi_handler(int EventSet, void *address, long_long overflow_vector, void *context) {}

void (*func_at_overflow_1)(int) = nullptr;

static void* resolve_symbol(const char* symbol_name, int config) {
  void* result;
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

void SamplerImpl::SetSamplingFreq(int freq) {
  // PAPI setup for main thread
  // char* str = getenv("CYC_SAMPLE_COUNT");
  // CYC_SAMPLE_COUNT = (str ? atoi(str) : DEFAULT_CYC_SAMPLE_COUNT);
  this->cyc_sample_count = (3.1 * 1e9) / (freq ? freq : DEFAULT_CYC_SAMPLE_COUNT);
  LOG_INFO("SET sample interval to %d cycles\n", this->cyc_sample_count);
}

void SamplerImpl::Setup() {
  TRY(PAPI_library_init(PAPI_VER_CURRENT), PAPI_VER_CURRENT);
  TRY(PAPI_thread_init(pthread_self), PAPI_OK);
}

void SamplerImpl::SetOverflow(void (*FUNC_AT_OVERFLOW)(int)) {
  int Events[NUM_EVENTS];

  this->EventSet = PAPI_NULL;
  TRY(PAPI_create_eventset(&(this->EventSet)), PAPI_OK);

  Events[0] = PAPI_TOT_CYC;
  // Events[1] = PAPI_L2_DCM;
  // Events[1] = PAPI_L1_DCM;
  // Events[1] = PAPI_TOT_INS;
  // Events[1] = PAPI_LD_INS;
  // Events[2] = PAPI_SR_INS;
  // Events[3] = PAPI_L1_DCM;
  // Events[3] = PAPI_L3_DCA;

  TRY(PAPI_add_events(this->EventSet, (int*)Events, NUM_EVENTS), PAPI_OK);

  // PAPI_overflow_handler_t *_papi_overflow_handler = (PAPI_overflow_handler_t*) &(this->papi_handler) (int EventSet,
  // void *address, long_long overflow_vector, void *context );
  // void (*)(int, void*, long long int, void*)
  // PAPI_overflow_handler_t _papi_overflow_handler = void (*)(int, void*, long long int, void*) &(papi_handler);
  PAPI_overflow_handler_t _papi_overflow_handler = (PAPI_overflow_handler_t) & (papi_handler);
  TRY(PAPI_overflow(this->EventSet, PAPI_TOT_CYC, this->cyc_sample_count, 0, _papi_overflow_handler), PAPI_OK);
  // TRY(PAPI_overflow(EventSet, PAPI_LD_INS, INS_SAMPLE_COUNT, 0, papi_handler), PAPI_OK);
  // TRY(PAPI_overflow(EventSet, PAPI_SR_INS, INS_SAMPLE_COUNT, 0, papi_handler), PAPI_OK);
  // TRY(PAPI_overflow(EventSet, PAPI_L1_DCM, CM_SAMPLE_COUNT, 0, papi_handler), PAPI_OK);
  // TRY(PAPI_overflow(EventSet, PAPI_L3_DCA, CM_SAMPLE_COUNT, 0, papi_handler), PAPI_OK);

  this->func_at_overflow = FUNC_AT_OVERFLOW;
  func_at_overflow_1 = FUNC_AT_OVERFLOW;
}

void SamplerImpl::Start() { TRY(PAPI_start(this->EventSet), PAPI_OK); }

void SamplerImpl::Stop() { TRY(PAPI_stop(this->EventSet, NULL), PAPI_OK); }

int SamplerImpl::GetOverflowEvent(LongLongVec* overflow_vector) {
  int Events[NUM_EVENTS], number, x, y;
  number = NUM_EVENTS;

  TRY(PAPI_get_overflow_event_index(this->EventSet, overflow_vector->overflow_vector, Events, &number), PAPI_OK);

  for (x = 0; x < number; x++) {
    for (y = 0; y < NUM_EVENTS; y++) {
      if (Events[x] == y) {
        break;
      }
    }
  }
  return y;
}

void SamplerImpl::GetBacktrace(char* call_path_str, int max_call_path_str_len) {
#ifdef MY_BT
  unw_word_t buffer[MAX_STACK_DEPTH] = {0};
#else
  void* buffer[MAX_STACK_DEPTH];
  memset(buffer, 0, sizeof(buffer));
#endif
  unsigned int i, depth = 0;

#ifdef MY_BT
  depth = my_backtrace(buffer, MAX_STACK_DEPTH);
#else
  depth = unw_backtrace(buffer, MAX_STACK_DEPTH);
#endif
  unsigned int addr_log_pointer = 0;

#ifdef MY_BT
  unw_word_t address_log[MAX_STACK_DEPTH] = {0};
#else
  void* address_log[MAX_STACK_DEPTH] = {0};
#endif
  int offset = 0;

#ifdef MY_BT
  for (i = 4; i < depth; ++i) {
    // if( (void*)buffer[i] != NULL && (char*)buffer[i] < addr_threshold ){
    if (buffer[i] != 0) {
      address_log[addr_log_pointer] = ((long unsigned int)(buffer[i]) - 2);
      addr_log_pointer++;
    }
  }

  if (addr_log_pointer > 0) {
    for (i = 0; i < addr_log_pointer; ++i) {
      offset += snprintf(call_path_str + offset, max_call_path_str_len - offset - 4, "%lx ",
                         (long unsigned int)address_log[i]);
      // LOG_INFO("%08x\n",address_log[i]);
    }
  }
#else
  for (i = 4; i < depth; ++i) {
    // if( (void*)buffer[i] != NULL && (char*)buffer[i] < addr_threshold ){
    if ((void*)buffer[i] != NULL) {
      address_log[addr_log_pointer] = (void*)((long unsigned int)(buffer[i]) - 2);
      addr_log_pointer++;
      // LOG_INFO("%08x\n",buffer[i]);
    }
  }

  if (addr_log_pointer > 0) {
    for (i = 0; i < addr_log_pointer; ++i) {
      offset += snprintf(call_path_str + offset, max_call_path_str_len - offset - 4, "%lx ",
                         (long unsigned int)address_log[i]);
      // LOG_INFO("%08x\n",address_log[i]);
    }
  }
#endif
}

int my_backtrace(unw_word_t* buffer, int max_depth) {
  unw_cursor_t cursor;
  unw_context_t context;

  // Initialize cursor to current frame for local unwinding.
  unw_getcontext(&context);
  unw_init_local(&cursor, &context);

  // Unwind frames one by one, going up the frame stack.
  int depth = 0;
  while (unw_step(&cursor) > 0 && depth < max_depth) {
    unw_word_t pc;
    unw_get_reg(&cursor, UNW_REG_IP, &pc);
    if (pc == 0) {
      break;
    }
    buffer[depth] = pc;
    depth++;
  }
  return depth;
}

void papi_handler(int EventSet, void* address, long_long overflow_vector, void* context) {
  // this->Stop();
  TRY(PAPI_stop(EventSet, NULL), PAPI_OK);

  // int y = this->GetOverflowEvent(overflow_vector);

  int Events[NUM_EVENTS], number, x, y;
  number = NUM_EVENTS;

  TRY(PAPI_get_overflow_event_index(EventSet, overflow_vector, Events, &number), PAPI_OK);

  for (x = 0; x < number; x++) {
    for (y = 0; y < NUM_EVENTS; y++) {
      if (Events[x] == y) {
        break;
      }
    }
  }
  // return y;

  printf("interrupt\n");
  (*(func_at_overflow_1))(y);

  TRY(PAPI_start(EventSet), PAPI_OK);
  // this->Start();
}
}