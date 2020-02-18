#pragma once

#include <gazebo/gazebo.hh>
#include <gazebo/physics/physics.hh>
#include <gazebo/transport/transport.hh>
#include <gazebo/msgs/msgs.hh>
#include <gazebo/plugins/CameraPlugin.hh>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <cstdint>
#include "gz_d435i.h"

namespace gazebo {
class D435iColorPlugin : public CameraPlugin {
 public:
  D435iColorPlugin(void);
  ~D435iColorPlugin(void);

  void Load(sensors::SensorPtr _model, sdf::ElementPtr _sdf);
  void OnNewFrame(const uint8_t *_image,
      unsigned int _width, unsigned int _height, unsigned int _depth,
      const std::string &_format);

 private:
  sensors::SensorPtr model;
  key_t shmkey;
  int shmid;
  bool shmvalid;
  nvar::d435i_color3buf_t *buf;
  bool sem_invalid;
};

GZ_REGISTER_SENSOR_PLUGIN(D435iColorPlugin);
}
