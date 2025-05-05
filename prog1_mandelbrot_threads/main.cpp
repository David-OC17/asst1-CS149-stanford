/**
 * CONCLUSION:
 *
 * The best performance for my computer (AMD® Ryzen 7 6800h) seems to show
 * around 27-28 threads. It yields around a 10.40x improvement over the single
 * threaded execution.
 *
 * It also seems only to benefit from a higher than 28 count of threads if there
 * are multiple executions of the code, back to back.
 *
 */

#include <getopt.h>
#include <stdio.h>

#include <algorithm>
#include <expected>

#include "CycleTimer.h"
#include "solution.hpp"

int main(int argc, char** argv) {
  std::expected<ResultMainCommon, int> res = mainCommon(argc, argv);

  if (!res) return res.error();  // EXIT_FAILURE

  //
  // Run the serial version
  //
  double minSerial =
      serialCreateFractal(res.value().output_serial, FRACTAL_WIDTH, FRACTAL_HEIGHT, MAX_ITERS,
                          res.value().x0, res.value().y0, res.value().x1, res.value().y1);

  //
  // Run the threaded version
  //

  // 1. Original thread (spatial decomposition)
  // double minThread = threadCreateFractal(
  //     res.value().output_thread, res.value().numThreads, FRACTAL_WIDTH,
  //     FRACTAL_HEIGHT, MAX_ITERS, res.value().x0, res.value().y0,
  //     res.value().x1, res.value().y1);

  // 2. Finding optimal configuration
  auto [minThread, optimalNumThreads] = mainFindOptimalNumThreads(
      res.value().output_thread, res.value().numThreads, FRACTAL_WIDTH, FRACTAL_HEIGHT, MAX_ITERS,
      res.value().x0, res.value().y0, res.value().x1, res.value().y1);

  if (!verifyResult(res.value().output_serial, res.value().output_thread, FRACTAL_WIDTH,
                    FRACTAL_HEIGHT)) {
    printf("Error : Output from threads does not match serial output\n");

    return EXIT_FAILURE;
  }

  //
  // Compute speedup
  //
  printf("\t\t\t\t(%.2fx speedup from %d threads)\n", minSerial / minThread,
         res.value().numThreads);

  //
  // Optimal number of threads for AMD® Ryzen 7 6800h
  //
  printf("Optimal number of threads: %i. \n", optimalNumThreads);

  return EXIT_SUCCESS;
}
