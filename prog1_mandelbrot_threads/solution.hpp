#include <getopt.h>
#include <stdio.h>

#include <algorithm>
#include <expected>

#include "CycleTimer.h"

#define FRACTAL_WIDTH 1600
#define FRACTAL_HEIGHT 1200
#define MAX_ITERS 256

#define NUM_THREADS 2

extern void mandelbrotSerial(float x0, float y0, float x1, float y1, int width,
                             int height, int startRow, int numRows,
                             int maxIterations, int output[]);

extern void mandelbrotThread(int numThreads, float x0, float y0, float x1,
                             float y1, int width, int height, int maxIterations,
                             int output[]);

extern void writePPMImage(int* data, int width, int height,
                          const char* filename, int maxIterations);

void scaleAndShift(float& x0, float& x1, float& y0, float& y1, float scale,
                   float shiftX, float shiftY);

void usage(const char* progname);

bool verifyResult(int* gold, int* result, int width, int height);

double serialCreateFractal(int* output_serial, int width, int height,
                           int maxIterations, float x0, float y0, float x1,
                           float y1);

double threadCreateFractal(int* output_thread, int numThreads, int width,
                           int height, int maxIterations, float x0, float y0,
                           float x1, float y1);

// ====================================================================== //
//                                 Solution                               //
// ====================================================================== //

struct ResultMainCommon {
  int* output_serial;
  int* output_thread;

  int numThreads;

  float x0;
  float x1;
  float y0;
  float y1;
};

// Take the standard program inputs, return output_serial and output_thread int*
std::expected<ResultMainCommon, int> mainCommon(int argc, char** argv);

// Run an optimal configuration for this computer (AMD® Ryzen 7 6800h)
std::pair<double, int> mainFindOptimalNumThreads(int* output_thread,
                                                 int numThreads, int width,
                                                 int height, int maxIterations,
                                                 float x0, float y0, float x1,
                                                 float y1);