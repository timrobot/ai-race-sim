#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstring>
#include <iostream>
#include "gz_d435i.h"
#include "log.h"

#define DEPTHID 0
#define COLORID 1

static int shmvalid[2];
static int shmid[2];
static nvar::d435i_depth3buf_t *depthFrame;
static nvar::d435i_color3buf_t *colorFrame;
static int connectCamera(int camid);
static int disconnectCamera(int camid);

int gz_d435i_constructor(void) {
  return connectCamera(DEPTHID) + connectCamera(COLORID);
}

int gz_d435i_destructor(void) {
  return disconnectCamera(DEPTHID) + disconnectCamera(COLORID);
}

void gz_d435i_grab_frames(float *depthBuf, uint8_t *colorBuf) {
  if (shmvalid[DEPTHID]) {
    sem_wait(&depthFrame->sem);
    if (depthFrame->lastop == 1) {
      std::swap(depthFrame->readind, depthFrame->swapind);
      depthFrame->lastop = 0;
    }
    sem_post(&depthFrame->sem);
    memcpy((void *)depthBuf, (void *)depthFrame->data[depthFrame->readind],
        nvar::DEPTH_SIZE);
  }
  if (shmvalid[COLORID]) {
    sem_wait(&colorFrame->sem);
    if (colorFrame->lastop == 1) {
      std::swap(colorFrame->readind, colorFrame->swapind);
      colorFrame->lastop = 0;
    }
    sem_post(&colorFrame->sem);
    memcpy((void *)colorBuf, (void *)colorFrame->data[colorFrame->readind],
        nvar::COLOR_SIZE);
  }
}

static int connectCamera(int camid) {
  int shmflg = 0666;
  
  if (camid == DEPTHID) {
    // grab the shm for the depth frame
    shmvalid[DEPTHID] = true;
    if ((shmid[DEPTHID] =
          shmget(nvar::DEPTH_KEY, nvar::DEPTH_SIZE, shmflg)) == -1) {
      LOGE("gz_d435i", "shmget depth frame");
      shmvalid[DEPTHID] = false;
    }
    // attach the depth frame buffer to the process
    if (shmvalid[DEPTHID] && (depthFrame =
          (nvar::d435i_depth3buf_t *)shmat(shmid[DEPTHID], nullptr, shmflg)) ==
        (nvar::d435i_depth3buf_t *)-1) {
      LOGE("gz_d435i", "shmat depth frame");
      shmvalid[DEPTHID] = false;
    }
  } else if (camid == COLORID) {
    shmvalid[COLORID] = true;
    if ((shmid[COLORID] =
          shmget(nvar::COLOR_KEY, nvar::COLOR_SIZE, shmflg)) == -1) {
      LOGE("gz_d435i", "shmget color frame");
      shmvalid[COLORID] = false;
    }
    if (shmvalid[COLORID] && (colorFrame =
          (nvar::d435i_color3buf_t *)shmat(shmid[COLORID], nullptr, shmflg)) ==
        (nvar::d435i_color3buf_t *)-1) {
      LOGE("gz_d435i", "shmat color frame");
      shmvalid[COLORID] = false;
    }
  }

  return shmvalid[camid] ? 1 : 0;
}

static int disconnectCamera(int camid) {
  if (!shmvalid[camid]) {
    return 0;
  }
  switch (camid) {
    case DEPTHID:
      if (shmdt(depthFrame) == -1) {
        LOGE("gz_d435i", "shmdt depth frame");
        return 0;
      } else {
        return 1;
      }
      break;
    case COLORID:
      if (shmdt(colorFrame) == -1) {
        LOGE("gz_d435i", "shmdt color frame");
        return 0;
      } else {
        return 1;
      }
      break;
    default:
      return 0;
  }
}
