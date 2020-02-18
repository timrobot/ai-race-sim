#pragma once

#include <cstdint>
#include <semaphore.h>

namespace nvar {

const int DEPTH_KEY = 9001;
const int COLOR_KEY = 9002;
const int RESSIZE = 640 * 360;
const int DEPTH_SIZE = RESSIZE * sizeof(float);
const int COLOR_SIZE = RESSIZE * 3;

typedef struct {
  sem_t sem;
  int readind;
  int writeind;
  int swapind;
  int lastop; /* 0 = read, 1 = write */
  float data[3][RESSIZE];
} d435i_depth3buf_t;

typedef struct {
  sem_t sem;
  int readind;
  int writeind;
  int swapind;
  int lastop; /* 0 = read, 1 = write */
  uint8_t data[3][COLOR_SIZE];
} d435i_color3buf_t;

}

extern "C" {

  /** Constructor
   *  @return the number of connected camera streams
   */
  int gz_d435i_constructor(void);

  /** Destructor
   *  @return the number of disconnected camera streams
   */
  int gz_d435i_destructor(void);

  /** Grab the next frames from their streams
   *  @param[out] depthFramebuf
   *  @param[out] colorFramebuf
   */
  void gz_d435i_grab_frames(float *depthBuf, uint8_t *colorBuf);

}
