//
// Created by Ahmed Abou-Aliaa on 25.02.20.
//

#include "mri.h"
#include "mri2020.hpp"
#include "mri_convert_lib.hpp"

#include <benchmark/benchmark.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wglobal-constructors"

constexpr auto magic_20 = 20;
constexpr auto magic_40 = 40;

static auto get_mri() {
  static auto mri = MRIread("/Users/aboualiaa/Downloads/mini.nii");
  return mri;
}

static void BM_MRIGetVoxVal(benchmark::State &state) { // NOLINT
  for (auto _ : state) {                               // NOLINT
    for (int f = 0; f < get_mri()->nframes; f++) {
      for (int i = 0; i < get_mri()->width; i++) {
        for (int j = 0; j < get_mri()->height; j++) {
          for (int k = 0; k < get_mri()->depth; k++) {
            auto val = MRIgetVoxVal(get_mri(), i, j, k, f);
            benchmark::DoNotOptimize(val);
          }
        }
      }
    }
  }
}

static void BM_MRISetVoxVal(benchmark::State &state) { // NOLINT
  for (auto _ : state) {                               // NOLINT
    for (int f = 0; f < get_mri()->nframes; f++) {
      for (int i = 0; i < get_mri()->width; i++) {
        for (int j = 0; j < get_mri()->height; j++) {
          for (int k = 0; k < get_mri()->depth; k++) {
            MRIsetVoxVal(get_mri(), i, j, k, f, 0);
          }
        }
      }
    }
  }
}

static void BM_SemiNewMRIGetVoxVal(benchmark::State &state) { // NOLINT
  fs::mri::semi_new_vox_getter vox_func =
      fs::mri::get_typed_semi_new_vox_getter_chunked(get_mri());
  for (auto _ : state) { // NOLINT
    for (int f = 0; f < get_mri()->nframes; f++) {
      for (int i = 0; i < get_mri()->width; i++) {
        for (int j = 0; j < get_mri()->height; j++) {
          for (int k = 0; k < get_mri()->depth; k++) {
            auto val = vox_func(get_mri(), i, j, k, f);
            benchmark::DoNotOptimize(val);
          }
        }
      }
    }
  }
}

static void BM_SemiNewMRISetVoxVal(benchmark::State &state) { // NOLINT
  fs::mri::semi_new_vox_setter vox_func =
      fs::mri::get_typed_semi_new_vox_setter_chunked(get_mri());
  for (auto _ : state) { // NOLINT
    for (int f = 0; f < get_mri()->nframes; f++) {
      for (int i = 0; i < get_mri()->width; i++) {
        for (int j = 0; j < get_mri()->height; j++) {
          for (int k = 0; k < get_mri()->depth; k++) {
            vox_func(get_mri(), i, j, k, f, magic_20);
          }
        }
      }
    }
  }
}

static void BM_NewMRIGetVoxVal(benchmark::State &state) { // NOLINT
  fs::mri::new_vox_getter vox_func =
      fs::mri::get_typed_new_vox_getter_chunked(get_mri());

  for (auto _ : state) { // NOLINT
    for (size_t index{0}; index < get_mri()->vox_total; index++) {
      auto val = vox_func(get_mri(), index);
      benchmark::DoNotOptimize(val);
    }
  }
}

static void BM_NewMRISetVoxVal(benchmark::State &state) { // NOLINT

  fs::mri::new_vox_setter vox_func =
      fs::mri::get_typed_new_vox_setter_chunked(get_mri());

  for (auto _ : state) { // NOLINT
    for (size_t index{0}; index < get_mri()->vox_total; index++) {
      vox_func(get_mri(), index, magic_40);
    }
  }
}

// Voxel value getters
BENCHMARK(BM_MRIGetVoxVal)->Unit(benchmark::kMillisecond);        // NOLINT
BENCHMARK(BM_SemiNewMRIGetVoxVal)->Unit(benchmark::kMillisecond); // NOLINT
BENCHMARK(BM_NewMRIGetVoxVal)->Unit(benchmark::kMillisecond);     // NOLINT

// Voxel value setters
BENCHMARK(BM_MRISetVoxVal)->Unit(benchmark::kMillisecond);        // NOLINT
BENCHMARK(BM_SemiNewMRISetVoxVal)->Unit(benchmark::kMillisecond); // NOLINT
BENCHMARK(BM_NewMRISetVoxVal)->Unit(benchmark::kMillisecond);     // NOLINT

auto main(int argc, char **argv) -> int {
  ::benchmark::Initialize(&argc, argv);
  if (::benchmark::ReportUnrecognizedArguments(argc, argv)) {
    return 1;
  }
  ::benchmark::RunSpecifiedBenchmarks();
}

#pragma GCC diagnostic pop
