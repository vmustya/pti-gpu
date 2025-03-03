//==============================================================
// Copyright (C) Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================

#include "ze_api_collector.h"

static ZeApiCollector* collector = nullptr;
static std::chrono::steady_clock::time_point start;

// External Tool Interface ////////////////////////////////////////////////////

extern "C"
#if defined(_WIN32)
__declspec(dllexport)
#endif
void Usage() {
  std::cout <<
    "Usage: ./ze_hot_functions[.exe] <application> <args>" <<
    std::endl;
}

extern "C"
#if defined(_WIN32)
__declspec(dllexport)
#endif
int ParseArgs(int argc, char* argv[]) {
  return 1;
}

extern "C"
#if defined(_WIN32)
__declspec(dllexport)
#endif
void SetToolEnv() {
  utils::SetEnv("ZE_ENABLE_TRACING_LAYER", "1");
}

// Internal Tool Functionality ////////////////////////////////////////////////

static void PrintResults() {
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  std::chrono::duration<uint64_t, std::nano> time = end - start;

  PTI_ASSERT(collector != nullptr);
  const ZeFunctionInfoMap& function_info_map = collector->GetFunctionInfoMap();
  if (function_info_map.size() == 0) {
    return;
  }

  uint64_t total_duration = 0;
  for (auto& value : function_info_map) {
    total_duration += value.second.total_time;
  }

  std::cerr << std::endl;
  std::cerr << "=== API Timing Results: ===" << std::endl;
  std::cerr << std::endl;
  std::cerr << "Total Execution Time (ns): " << time.count() << std::endl;
  std::cerr << "Total API Time (ns): " << total_duration << std::endl;
  std::cerr << std::endl;

  if (total_duration > 0) {
    ZeApiCollector::PrintFunctionsTable(function_info_map);
  }

  std::cerr << std::endl;
}

// Internal Tool Interface ////////////////////////////////////////////////////

void EnableProfiling() {
  ze_result_t status = ZE_RESULT_SUCCESS;
  status = zeInit(ZE_INIT_FLAG_GPU_ONLY);
  PTI_ASSERT(status == ZE_RESULT_SUCCESS);

  collector = ZeApiCollector::Create();
  start = std::chrono::steady_clock::now();
}

void DisableProfiling() {
  if (collector != nullptr) {
    collector->DisableTracing();
    PrintResults();
    delete collector;
  }
}