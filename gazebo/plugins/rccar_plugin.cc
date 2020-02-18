#include "rccar_plugin.h"
#include "log.h"

using namespace gazebo;

RCCarPlugin::RCCarPlugin(void) : ModelPlugin(),
  sensor_shmkey(nvar::SENSOR_KEY), sensor_shmid(-1),
  sensor_shmvalid(false), actuator_shmkey(nvar::ACTUATOR_KEY),
  actuator_shmid(-1), actuator_shmvalid(false) {}

RCCarPlugin::~RCCarPlugin(void) {
  if (sensors >= 0) {
    if (!sensor_sem_invalid) {
      sem_destroy(&sensors->sem);
    }
    if (shmdt(sensors) == -1) {
      LOGE("RCCarPluginSensor", "Failed to detach");
    } else {
      struct shmid_ds sds;
      shmctl(sensor_shmid, IPC_RMID, &sds);
    }
  }
  if (actuators >= 0) {
    if (!actuator_sem_invalid) {
      sem_destroy(&actuators->sem);
    }
    if (shmdt(actuators) == -1) {
      LOGE("RCCarPluginActuator", "Failed to detach");
    } else {
      struct shmid_ds sds;
      shmctl(actuator_shmid, IPC_RMID, &sds);
    }
  }
}

void RCCarPlugin::Load(physics::ModelPtr _model,
    sdf::ElementPtr _sdf) {
  // Safety check
  if (_model->GetJointCount() == 0) {
    LOGE("rccar_plugin", "Invalid joint count, rccar plugin not loaded");
    return;
  }

  // Store the model, joints pointer, and body pointer for convenience.
  model = _model;
  std::string modelname = model->GetName();
  joints = model->GetJoints();
  physics::Link_V links = model->GetLinks();
  for (size_t i = 0; i < links.size(); i++) {
    if (links[i]->GetScopedName() == modelname + "::rccar::chassis") {
      body = links[i];
    }
  }

  // Store the axis for each joint's scoped name
  jointAxisMap[modelname + "::rccar::chassis_front_left_balljoint"]   = 2;
  jointAxisMap[modelname + "::rccar::chassis_front_right_balljoint"]  = 2;
  jointAxisMap[modelname + "::rccar::chassis_back_left_wheel"]        = 0;
  jointAxisMap[modelname + "::rccar::chassis_back_right_wheel"]       = 0;

  // For all the joints, attach PID controllers
  for (auto kv : jointAxisMap) {
    // kv is <string, int>
    model->GetJointController()->SetVelocityPID
      (kv.first, common::PID(1, 0, 0));
  }

  // Set up a shared memory module for the sensors and actuators
  if ((sensor_shmid = shmget(sensor_shmkey,
          sizeof(nvar::rccar_sensor_values_t), 0666 | IPC_CREAT)) == -1) {
    struct shmid_ds sds;
    LOGW("RCCarPluginSensor", "Shared memory already exists");
    shmctl(sensor_shmid, IPC_RMID, &sds);
    if ((sensor_shmid = shmget(sensor_shmkey,
            sizeof(nvar::rccar_sensor_values_t), 0666 | IPC_CREAT)) == -1) {
      LOGE("RCCarPluginSensor", "Shared memory still exists, exiting");
      return;
    }
  }

  if ((actuator_shmid = shmget(actuator_shmkey,
          sizeof(nvar::rccar_actuator_values_t), 0666 | IPC_CREAT)) == -1) {
    struct shmid_ds sds;
    LOGW("RCCarPluginActuator", "Shared memory already exists");
    shmctl(actuator_shmid, IPC_RMID, &sds);
    if ((actuator_shmid = shmget(actuator_shmkey,
            sizeof(nvar::rccar_actuator_values_t), 0666 | IPC_CREAT)) == -1) {
      LOGE("RCCarPluginActuator", "Shared memory still exists, exiting");
      return;
    }
  }

  sensors = (nvar::rccar_sensor_values_t *)shmat(
      sensor_shmid, nullptr, 0666 | IPC_CREAT);
  if (sensors == (nvar::rccar_sensor_values_t *)-1) {
    LOGE("RCCarPluginSensor", "Failed to attach shm");
    return;
  } else if ((sensor_sem_invalid =
        sem_init(&sensors->sem, 1, 1) < 0)) {
    LOGE("RCCarPluginSensor", "Failed to initialize semaphore");
    return;
  }
  sensor_shmvalid = true;

  actuators = (nvar::rccar_actuator_values_t *)shmat(
      actuator_shmid, nullptr, 0666 | IPC_CREAT);
  if (actuators == (nvar::rccar_actuator_values_t *)-1) {
    LOGE("RCCarPluginActuator", "Failed to attach shm");
    return;
  } else if ((actuator_sem_invalid =
        sem_init(&actuators->sem, 1, 1) < 0)) {
    LOGE("RCCarPluginActuator", "Failed to initialize semaphore");
    return;
  }
  actuator_shmvalid = true;

  // Listen to the update event and broadcast the positions
  LOGI("rccar_plugin", "Binding event updates...");
  updateConnection = event::Events::ConnectWorldUpdateBegin
    (boost::bind(&RCCarPlugin::OnUpdate, this, _1));

  LOGI("rccar_plugin", "Done");
}

void RCCarPlugin::OnUpdate(const common::UpdateInfo& _info) {
  SendRobotSensors();
  UpdateRobotActuators();
  UpdateResetSignal();
}

void RCCarPlugin::UpdateRobotActuators(void) {
  nvar::rccar_actuator_values_t actuator;
  sem_wait(&actuators->sem);
  memcpy(&actuator, actuators, sizeof(nvar::rccar_actuator_values_t));
  sem_post(&actuators->sem);
  physics::JointControllerPtr jointController = model->GetJointController();
  if (actuator.signal == nvar::RESET_SIGNAL) {
    return;
  }
  std::string modelname = model->GetName();
  jointController->SetVelocityTarget(
      modelname + "::rccar::chassis_back_left_wheel",
      actuator.motor);
  jointController->SetVelocityTarget(
      modelname + "::rccar::chassis_back_right_wheel",
      -actuator.motor);
  jointController->SetPositionTarget(
      modelname + "::rccar::chassis_front_left_balljoint",
      actuator.steer);
  jointController->SetPositionTarget(
      modelname + "::rccar::chassis_front_right_balljoint",
      actuator.steer);
}

void RCCarPlugin::UpdateResetSignal(void) {
  bool doReset = false;
  sem_wait(&actuators->sem);
  if (actuators->signal == nvar::RESET_SIGNAL) {
    doReset = true;
  }
  sem_post(&actuators->sem);
  if (doReset) {
    model->Reset();
    physics::Link_V links = model->GetLinks();
    for (size_t i = 0; i < links.size(); i++) {
      links[i]->Reset();
    }
    for (size_t i = 0; i < jointAxisMap.size(); i++) {
      joints[i]->Reset();
    }
  }

  SendRobotSensors(doReset);
}

void RCCarPlugin::SendRobotSensors(bool reset_confirm) {
  nvar::rccar_sensor_values_t sensor = {0};
  ignition::math::Vector3d orientation = body->WorldPose().Rot().Euler();
  sensor.roll = orientation.X();
  sensor.pitch = orientation.Y();
  sensor.yaw = orientation.Z();

  std::string modelname = model->GetName();

  double currtime = model->GetWorld()->SimTime().Double();
  sem_wait(&sensors->sem);
  sensors->roll = sensor.roll;
  sensors->pitch = sensor.pitch;
  sensors->yaw = sensor.yaw;
  sensors->timestamp = currtime;
  if (reset_confirm) {
    sensors->signal = nvar::RESET_SIGNAL;
  }
  sem_post(&sensors->sem);

  if (reset_confirm) {
    sem_wait(&actuators->sem);
    actuators->signal = nvar::STEP_SIGNAL;
    sem_post(&actuators->sem);
  }
}
