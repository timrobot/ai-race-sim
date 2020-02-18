#pragma once

#include <gazebo/gazebo.hh>
#include <gazebo/physics/physics.hh>
#include <gazebo/transport/transport.hh>
#include <gazebo/msgs/msgs.hh>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <map>
#include "gz_rccar.h"

namespace gazebo {

class RCCarPlugin : public ModelPlugin {
 public:
  RCCarPlugin(void);
  ~RCCarPlugin(void);

  void Load(physics::ModelPtr _model, sdf::ElementPtr _sdf);
  void OnUpdate(const common::UpdateInfo& _info);

 private:
  // Update event connection
  event::ConnectionPtr updateConnection;
  // Pointer to the model
  physics::ModelPtr model;
  // Pointer to all the joints
  physics::Joint_V joints;
  // Pointer to the central link
  physics::LinkPtr body;
  // Map from the scoped name to the joint axes
  std::map<std::string, int> jointAxisMap;

  // pointer to shared memory for sensors
  nvar::rccar_sensor_values_t *sensors;
  bool sensor_sem_invalid;
  key_t sensor_shmkey;
  int sensor_shmid;
  bool sensor_shmvalid;
  // pointer to shared memory for actuators
  nvar::rccar_actuator_values_t *actuators;
  bool actuator_sem_invalid;
  key_t actuator_shmkey;
  int actuator_shmid;
  bool actuator_shmvalid;

  void SendRobotSensors(bool reset_confirm=false);
  void UpdateResetSignal(void);
  void UpdateRobotActuators(void);
};

GZ_REGISTER_MODEL_PLUGIN(RCCarPlugin);
}
