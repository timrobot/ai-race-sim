#pragma once

namespace nvar {

const int STEP_SIGNAL = 0;
const int RESET_SIGNAL = 1;
const int ACTUATOR_KEY = 10001;
const int SENSOR_KEY = 10002;

typedef struct {
  sem_t sem;
  double roll;
  double pitch;
  double yaw;
  double timestamp;
  int signal;
} rccar_sensor_values_t;

typedef struct {
  sem_t sem;
  double steer;
  double motor;
  int signal;
} rccar_actuator_values_t;

}

extern "C" {

  /** Constructor
   */
  void gz_rccar_constructor(void);

  /** Destructor
   */
  void gz_rccar_destructor(void);

  /** Set the joint velocities
   *  @param[in] steer
   *  @param[in] motor
   */
  void gz_rccar_set_actuators(double steer, double motor);

  /** Get the joint positions
   *  @param[out] gyro where to store the gyro values into from the IMU
   *  @param[out] timestamp
   */
  void gz_rccar_get_sensors(double *gyro, double *timestamp);

  /** Reset the simulation models
   */
  void gz_rccar_reset(void);
  void gz_rccar_wait_for_reset(void);

}
