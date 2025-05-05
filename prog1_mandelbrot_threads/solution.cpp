#include "solution.hpp"

#include <getopt.h>
#include <stdio.h>

#include <algorithm>
#include <cmath>
#include <expected>
#include <utility>

#include "CycleTimer.h"

#define FRACTAL_WIDTH 1600
#define FRACTAL_HEIGHT 1200
#define MAX_ITERS 256

#define NUM_THREADS 2

extern void mandelbrotSerial(float x0, float y0, float x1, float y1, int width, int height,
                             int startRow, int numRows, int maxIterations, int output[]);

extern void mandelbrotThread(int numThreads, float x0, float y0, float x1, float y1, int width,
                             int height, int maxIterations, int output[]);

extern void writePPMImage(int* data, int width, int height, const char* filename,
                          int maxIterations);

void scaleAndShift(float& x0, float& x1, float& y0, float& y1, float scale, float shiftX,
                   float shiftY) {
  x0 *= scale;
  x1 *= scale;
  y0 *= scale;
  y1 *= scale;
  x0 += shiftX;
  x1 += shiftX;
  y0 += shiftY;
  y1 += shiftY;
}

void usage(const char* progname) {
  printf("Usage: %s [options]\n", progname);
  printf("Program Options:\n");
  printf("  -t  --threads <N>  Use N threads\n");
  printf("  -v  --view <INT>   Use specified view settings\n");
  printf("  -?  --help         This message\n");
}

bool verifyResult(int* gold, int* result, int width, int height) {
  int i, j;

  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
      if (gold[i * width + j] != result[i * width + j]) {
        printf("Mismatch : [%d][%d], Expected : %d, Actual : %d\n", i, j, gold[i * width + j],
               result[i * width + j]);
        return 0;
      }
    }
  }

  return 1;
}

double serialCreateFractal(int* output_serial, int width, int height, int maxIterations, float x0,
                           float y0, float x1, float y1) {
  double minSerial = 1e30;
  for (int i = 0; i < 5; ++i) {
    memset(output_serial, 0, width * height * sizeof(int));
    double startTime = CycleTimer::currentSeconds();
    mandelbrotSerial(x0, y0, x1, y1, width, height, 0, height, maxIterations, output_serial);
    double endTime = CycleTimer::currentSeconds();
    minSerial = std::min(minSerial, endTime - startTime);
  }

  printf("[mandelbrot serial]:\t\t[%.3f] ms\n", minSerial * 1000);
  writePPMImage(output_serial, width, height, "mandelbrot-serial.ppm", maxIterations);

  return minSerial;
}

double threadCreateFractal(int* output_thread, int numThreads, int width, int height,
                           int maxIterations, float x0, float y0, float x1, float y1) {
  double minThread = 1e30;
  for (int i = 0; i < 5; ++i) {
    memset(output_thread, 0, width * height * sizeof(int));
    double startTime = CycleTimer::currentSeconds();
    mandelbrotThread(numThreads, x0, y0, x1, y1, width, height, maxIterations, output_thread);
    double endTime = CycleTimer::currentSeconds();
    minThread = std::min(minThread, endTime - startTime);
  }

  printf("[mandelbrot thread]:\t\t[%.3f] ms\n", minThread * 1000);
  writePPMImage(output_thread, width, height, "mandelbrot-thread.ppm", maxIterations);

  return minThread;
}

// ====================================================================== //
//                                 Solution                               //
// ====================================================================== //

// Take the standard program inputs, return output_serial and output_thread int*
std::expected<ResultMainCommon, int> mainCommon(int argc, char** argv) {
  const unsigned int width = FRACTAL_WIDTH;
  const unsigned int height = FRACTAL_HEIGHT;
  int numThreads = NUM_THREADS;

  float x0 = -2;
  float x1 = 1;
  float y0 = -1;
  float y1 = 1;

  // parse commandline options ////////////////////////////////////////////
  int opt;
  static struct option long_options[] = {
      {"threads", 1, 0, 't'}, {"view", 1, 0, 'v'}, {"help", 0, 0, '?'}, {0, 0, 0, 0}};

  while ((opt = getopt_long(argc, argv, "t:v:?", long_options, NULL)) != EOF) {
    switch (opt) {
      case 't': {
        numThreads = atoi(optarg);
        break;
      }
      case 'v': {
        int viewIndex = atoi(optarg);
        // change view settings
        if (viewIndex == 2) {
          float scaleValue = .015f;
          float shiftX = -.986f;
          float shiftY = .30f;
          scaleAndShift(x0, x1, y0, y1, scaleValue, shiftX, shiftY);
        } else if (viewIndex > 1) {
          fprintf(stderr, "Invalid view index\n");
          return std::unexpected(EXIT_FAILURE);
        }
        break;
      }
      case '?':
      default:
        usage(argv[0]);
        return std::unexpected(EXIT_FAILURE);
    }
  }
  // end parsing of commandline options

  ResultMainCommon res;
  res.output_serial = new int[width * height];
  res.output_thread = new int[width * height];

  res.numThreads = numThreads;

  res.x0 = x0;
  res.x1 = x1;
  res.y0 = y0;
  res.y1 = y1;

  return res;
}

// Run an optimal configuration for this computer (AMD® Ryzen 7 6800h)
std::pair<double, int> mainFindOptimalNumThreads(int* output_thread, int numThreads, int width,
                                                 int height, int maxIterations, float x0, float y0,
                                                 float x1, float y1) {
  std::pair<double, int> optimalConfig = std::make_pair(1e30, 1);

  for (numThreads = 2; numThreads <= 32; numThreads++) {
    double tempOptimal = optimalConfig.first;

    // NOTE: will re-write the output each time it runs
    optimalConfig.first = std::min(threadCreateFractal(output_thread, numThreads, FRACTAL_WIDTH,
                                                       FRACTAL_HEIGHT, MAX_ITERS, x0, y0, x1, y1),
                                   optimalConfig.first);

    if (tempOptimal != optimalConfig.first) {
      optimalConfig.second = numThreads;
    }
  }

  return optimalConfig;
}
