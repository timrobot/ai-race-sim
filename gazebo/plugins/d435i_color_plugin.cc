#include "d435i_color_plugin.h"
#include "log.h"

using namespace gazebo;

D435iColorPlugin::D435iColorPlugin(void) :
  shmkey(nvar::COLOR_KEY), shmid(-1), shmvalid(false), buf(nullptr) {
}

D435iColorPlugin::~D435iColorPlugin(void) {
  if (buf >= 0) {
    if (!sem_invalid) {
      sem_destroy(&buf->sem);
    }
    if (shmdt(buf) == -1) {
      LOGE("D435iColorPlugin", "Failed to detach");
      return;
    }
    struct shmid_ds sds;
    shmctl(shmid, IPC_RMID, &sds);
  }
}

void D435iColorPlugin::Load(sensors::SensorPtr _model,
    sdf::ElementPtr _sdf) {
  CameraPlugin::Load(_model, _sdf);
  model = _model;

  if ((shmid = shmget(shmkey, sizeof(nvar::d435i_color3buf_t),
          0666 | IPC_CREAT)) == -1) {
    struct shmid_ds sds;
    LOGW("D435iColorPlugin", "Shared memory already exists");
    shmctl(shmid, IPC_RMID, &sds);
    if ((shmid = shmget(shmkey, sizeof(nvar::d435i_color3buf_t),
            0666 | IPC_CREAT)) == -1) {
      LOGE("D435iColorPlugin", "Shared memory still exists, exiting");
      return;
    }
  }

  buf = (nvar::d435i_color3buf_t *)shmat(shmid, nullptr, 0666 | IPC_CREAT);
  buf->writeind = 0;
  buf->readind = 1;
  buf->swapind = 2;
  if (buf == (nvar::d435i_color3buf_t *)-1) {
    LOGE("D435iColorPlugin", "Failed to attach shm");
    return;
  } else if ((sem_invalid = sem_init(&buf->sem, 1, 1) < 0)) {
    LOGE("D435iColorPlugin", "Failed to initialize semaphore");
    return;
  }
  shmvalid = true;
}

void D435iColorPlugin::OnNewFrame(const uint8_t *_image,
    unsigned int _width, unsigned int _height, unsigned int _color,
    const std::string &_format) {
  if (!model->IsActive()) {
    parentSensor->SetActive(true);
  } else {
    memcpy((void *)buf->data[buf->writeind], (void *)_image, nvar::COLOR_SIZE);
    sem_wait(&buf->sem);
    std::swap(buf->writeind, buf->swapind);
    buf->lastop = 1;
    sem_post(&buf->sem);
  }
}
