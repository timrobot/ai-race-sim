#include <gazebo/transport/transport.hh>
#include <gazebo/msgs/msgs.hh>
#include <gazebo/gazebo_client.hh>
#include <iostream>
#include <cstdio>
#include <string>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "gz_rccar.h"
#include "log.h"

static nvar::rccar_sensor_values_t *sensor_info;
static nvar::rccar_actuator_values_t *actuator_info;
static bool isReset;
static int sensor_shmvalid;
static int sensor_shmid;
static int actuator_shmvalid;
static int actuator_shmid;

static double gettime(void) {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return (double)tv.tv_sec + (double)tv.tv_usec / 1000000;
}

void gz_rccar_constructor(void) {
  // Once the gazebo server is started from gz_core, then the following shared
  // memory modules should also be created
  sensor_shmvalid = true;
  while ((sensor_shmid = shmget(nvar::SENSOR_KEY,
          sizeof(nvar::rccar_sensor_values_t), 0666)) == -1) {
  }
  actuator_shmvalid = true;
  while ((actuator_shmid = shmget(nvar::ACTUATOR_KEY,
          sizeof(nvar::rccar_actuator_values_t), 0666)) == -1) {
  }

  if ((sensor_info = (nvar::rccar_sensor_values_t *)shmat(sensor_shmid,
          nullptr, 0666)) == (nvar::rccar_sensor_values_t *)-1) {
    LOGE("gz_rccar", "shmat sensor info");
    sensor_shmvalid = false;
  }
  if ((actuator_info = (nvar::rccar_actuator_values_t *)shmat(actuator_shmid,
          nullptr, 0666)) == (nvar::rccar_actuator_values_t *)-1) {
    LOGE("gz_rccar", "shmat actuator info");
    actuator_shmvalid = false;
  }
}

void gz_rccar_destructor(void) {
  if (shmdt(sensor_info) == -1) {
    LOGE("gz_rccar", "shmdt sensor info");
    sensor_shmvalid = false;
  }
  if (shmdt(actuator_info) == -1) {
    LOGE("gz_rccar", "shmdt actuator info");
    actuator_shmvalid = false;
  }
}

void gz_rccar_set_actuators(double steer, double motor) {
  sem_wait(&actuator_info->sem);
  //actuator_info->timestamp = gettime();
  actuator_info->steer = steer;
  actuator_info->motor = motor;
  sem_post(&actuator_info->sem);
}

void gz_rccar_get_sensors(double *gyro, double *timestamp) {
  nvar::rccar_sensor_values_t sensor;
  sem_wait(&sensor_info->sem);
  memcpy(&sensor, sensor_info, sizeof(nvar::rccar_sensor_values_t));
  sem_post(&sensor_info->sem);

  gyro[0] = sensor.roll;
  gyro[1] = sensor.pitch;
  gyro[2] = sensor.yaw;
  *timestamp = sensor.timestamp;
}

void gz_rccar_reset(void) {
  isReset = false;
  sem_wait(&actuator_info->sem);
  actuator_info->signal = nvar::RESET_SIGNAL;
  sem_post(&actuator_info->sem);
}

void gz_rccar_wait_for_reset(void) {
  for (int i = 0; !isReset && i < 400; i++) { // wait at most .2 seconds
    sem_wait(&sensor_info->sem);
    if (sensor_info->signal == nvar::RESET_SIGNAL) {
      sensor_info->signal = nvar::STEP_SIGNAL;
      isReset = true;
    }
    sem_post(&sensor_info->sem);
    usleep(500);
  }
  if (!isReset) {
    LOGW("c++", "Warning: Did not rec'v reset signal, however we can't hang.");
  }
}
