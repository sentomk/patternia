#include <benchmark/benchmark.h>
#include <random>
#include <vector>
#include <cmath>
#include <ptn/patternia.hpp>

using namespace ptn;

// thresholds: 32 cuts, 33 buckets
static constexpr int CUTS[] = {
    -1875, -1750, -1625, -1500, -1375, -1250, -1125, -1000, -875, -750, -625,
    -500,  -375,  -250,  -125,  0,     125,   250,   375,   500,  625,  750,
    875,   1000,  1125,  1250,  1375,  1500,  1625,  1750,  1875, 2000};
static constexpr int NB = sizeof(CUTS) / sizeof(CUTS[0]) + 1;

// if-else chain
inline int classify_ifelse(int x) {
#define IFRET(c, idx)                                                          \
  if (x < (c))                                                                 \
    return (idx);
  IFRET(-1875, 0)
  else IFRET(-1750, 1) else IFRET(-1625, 2) else IFRET(
      -1500, 3) else IFRET(-1375, 4) else IFRET(-1250, 5) else IFRET(-1125, 6) else IFRET(-1000, 7) else IFRET(-875, 8) else IFRET(-750, 9) else IFRET(-625, 10) else IFRET(-500, 11) else IFRET(-375, 12) else IFRET(-250, 13) else IFRET(-125, 14) else IFRET(0, 15) else IFRET(125, 16) else IFRET(250, 17) else IFRET(375, 18) else IFRET(500, 19) else IFRET(625, 20) else IFRET(750, 21) else IFRET(875, 22) else IFRET(1000, 23) else IFRET(1125, 24) else IFRET(1250, 25) else IFRET(1375, 26) else IFRET(1500, 27) else IFRET(1625, 28) else IFRET(1750, 29) else IFRET(1875, 30) else IFRET(2000, 31) else return 32;
#undef IFRET
}

// switch-case version (dense jump table)
inline int classify_switch(int x) {
  int idx = (x - (-1875)) / 125 + 1;
  if (idx < 0)
    idx = 0;
  if (idx > 32)
    idx = 32;
  switch (idx) {
  case 0:
    return 0;
  case 1:
    return 1;
  case 2:
    return 2;
  case 3:
    return 3;
  case 4:
    return 4;
  case 5:
    return 5;
  case 6:
    return 6;
  case 7:
    return 7;
  case 8:
    return 8;
  case 9:
    return 9;
  case 10:
    return 10;
  case 11:
    return 11;
  case 12:
    return 12;
  case 13:
    return 13;
  case 14:
    return 14;
  case 15:
    return 15;
  case 16:
    return 16;
  case 17:
    return 17;
  case 18:
    return 18;
  case 19:
    return 19;
  case 20:
    return 20;
  case 21:
    return 21;
  case 22:
    return 22;
  case 23:
    return 23;
  case 24:
    return 24;
  case 25:
    return 25;
  case 26:
    return 26;
  case 27:
    return 27;
  case 28:
    return 28;
  case 29:
    return 29;
  case 30:
    return 30;
  case 31:
    return 31;
  default:
    return 32;
  }
}

// patternia DSL version
inline int classify_patternia(int x) {
  return match(x)
      .when(lt(-1875) >> 0)
      .when(lt(-1750) >> 1)
      .when(lt(-1625) >> 2)
      .when(lt(-1500) >> 3)
      .when(lt(-1375) >> 4)
      .when(lt(-1250) >> 5)
      .when(lt(-1125) >> 6)
      .when(lt(-1000) >> 7)
      .when(lt(-875) >> 8)
      .when(lt(-750) >> 9)
      .when(lt(-625) >> 10)
      .when(lt(-500) >> 11)
      .when(lt(-375) >> 12)
      .when(lt(-250) >> 13)
      .when(lt(-125) >> 14)
      .when(lt(0) >> 15)
      .when(lt(125) >> 16)
      .when(lt(250) >> 17)
      .when(lt(375) >> 18)
      .when(lt(500) >> 19)
      .when(lt(625) >> 20)
      .when(lt(750) >> 21)
      .when(lt(875) >> 22)
      .when(lt(1000) >> 23)
      .when(lt(1125) >> 24)
      .when(lt(1250) >> 25)
      .when(lt(1375) >> 26)
      .when(lt(1500) >> 27)
      .when(lt(1625) >> 28)
      .when(lt(1750) >> 29)
      .when(lt(1875) >> 30)
      .when(lt(2000) >> 31)
      .otherwise(32);
}

// data distributions
enum class Dist {
  Uniform,
  Adversarial,
  Zipf
};

static std::vector<int> make_data(size_t N, Dist d) {
  std::vector<int> v;
  v.reserve(N);
  std::mt19937_64 rng(123456);
  if (d == Dist::Uniform) {
    std::uniform_int_distribution<int> uni(-2500, 2500);
    for (size_t i = 0; i < N; i++)
      v.push_back(uni(rng));
  }
  else if (d == Dist::Adversarial) {
    for (size_t rep = 0; rep < N / 64; rep++)
      for (int c : CUTS) {
        v.push_back(c - 1);
        v.push_back(c);
      }
    while (v.size() < N)
      v.push_back(CUTS[0]);
  }
  else {
    std::uniform_real_distribution<double> U(0.0, 1.0);
    auto                                   invcdf = [](double u) {
      double s = 1.25, HN = 0;
      for (int k = 1; k <= NB; k++)
        HN += 1.0 / std::pow(k, s);
      double acc = 0;
      for (int k = 1; k <= NB; k++) {
        acc += 1.0 / std::pow(k, s);
        if (u * HN <= acc)
          return k - 1;
      }
      return NB - 1;
    };
    for (size_t i = 0; i < N; i++) {
      int b  = invcdf(U(rng));
      int lo = (b == 0) ? CUTS[0] - 1000 : CUTS[b - 1];
      int hi = (b < (int) (sizeof(CUTS) / sizeof(CUTS[0]))) ? CUTS[b]
                                                            : CUTS[31] + 1000;
      v.push_back(lo + (hi - lo) / 2);
    }
  }
  return v;
}

// benchmark templates
template <Dist D>
static void BM_IfElse(benchmark::State &st) {
  auto data = make_data(st.range(0), D);
  int  acc  = 0;
  for (auto _ : st)
    for (int x : data)
      benchmark::DoNotOptimize(acc += classify_ifelse(x));
  benchmark::ClobberMemory();
}
template <Dist D>
static void BM_Switch(benchmark::State &st) {
  auto data = make_data(st.range(0), D);
  int  acc  = 0;
  for (auto _ : st)
    for (int x : data)
      benchmark::DoNotOptimize(acc += classify_switch(x));
  benchmark::ClobberMemory();
}
template <Dist D>
static void BM_Patternia(benchmark::State &st) {
  auto data = make_data(st.range(0), D);
  int  acc  = 0;
  for (auto _ : st)
    for (int x : data)
      benchmark::DoNotOptimize(acc += classify_patternia(x));
  benchmark::ClobberMemory();
}

constexpr int N = 100000;
BENCHMARK_TEMPLATE(BM_IfElse, Dist::Uniform)->Arg(N);
BENCHMARK_TEMPLATE(BM_Switch, Dist::Uniform)->Arg(N);
BENCHMARK_TEMPLATE(BM_Patternia, Dist::Uniform)->Arg(N);

BENCHMARK_TEMPLATE(BM_IfElse, Dist::Adversarial)->Arg(N);
BENCHMARK_TEMPLATE(BM_Switch, Dist::Adversarial)->Arg(N);
BENCHMARK_TEMPLATE(BM_Patternia, Dist::Adversarial)->Arg(N);

BENCHMARK_TEMPLATE(BM_IfElse, Dist::Zipf)->Arg(N);
BENCHMARK_TEMPLATE(BM_Switch, Dist::Zipf)->Arg(N);
BENCHMARK_TEMPLATE(BM_Patternia, Dist::Zipf)->Arg(N);
