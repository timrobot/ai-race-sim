#pragma once

#include <gazebo/gazebo.hh>
#include <gazebo/physics/physics.hh>
#include <gazebo/transport/transport.hh>
#include <gazebo/msgs/msgs.hh>
#include <gazebo/plugins/DepthCameraPlugin.hh>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include "gz_d435i.h"

namespace gazebo {
class D435iDepthPlugin : public DepthCameraPlugin {
 public:
  D435iDepthPlugin(void);
  ~D435iDepthPlugin(void);

  void Load(sensors::SensorPtr _model, sdf::ElementPtr _sdf);
  void OnNewDepthFrame(const float *_image,
      unsigned int _width, unsigned int _height, unsigned int _depth,
      const std::string &_format);

 private:
  sensors::SensorPtr model;
  key_t shmkey;
  int shmid;
  bool shmvalid;
  nvar::d435i_depth3buf_t *buf;
  bool sem_invalid;
};

GZ_REGISTER_SENSOR_PLUGIN(D435iDepthPlugin);
}
