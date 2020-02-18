#include <gazebo/gazebo_client.hh>
#include "gz_core.h"

void gz_core_constructor(void) {
  gazebo::client::setup();
}

void gz_core_destructor(void) {
  gazebo::client::shutdown();
}
