#include "d435i_depth_plugin.h"
#include "log.h"

using namespace gazebo;

D435iDepthPlugin::D435iDepthPlugin(void) :
  shmkey(nvar::DEPTH_KEY), shmid(-1), shmvalid(false), buf(nullptr) {
}

D435iDepthPlugin::~D435iDepthPlugin(void) {
  if (buf >= 0) {
    if (!sem_invalid) {
      sem_destroy(&buf->sem);
    }
    if (shmdt(buf) == -1) {
      LOGE("D435iDepthPlugin", "Failed to detach");
      return;
    }
    struct shmid_ds sds;
    shmctl(shmid, IPC_RMID, &sds);
  }
}

void D435iDepthPlugin::Load(sensors::SensorPtr _model, sdf::ElementPtr _sdf) {
  DepthCameraPlugin::Load(_model, _sdf);
  model = _model;

  if ((shmid = shmget(shmkey, sizeof(nvar::d435i_depth3buf_t),
          0666 | IPC_CREAT)) == -1) {
    struct shmid_ds sds;
    LOGW("D435iDepthPlugin", "Shared memory already exists");
    shmctl(shmid, IPC_RMID, &sds);
    if ((shmid = shmget(shmkey, sizeof(nvar::d435i_depth3buf_t),
            0666 | IPC_CREAT)) == -1) {
      LOGE("D435iDepthPlugin", "Shared memory still exists, exiting");
      return;
    }
  }

  buf = (nvar::d435i_depth3buf_t *)shmat(shmid, nullptr, 0666 | IPC_CREAT);
  buf->writeind = 0;
  buf->readind = 1;
  buf->swapind = 2;
  if (buf == (nvar::d435i_depth3buf_t *)-1) {
    LOGE("D435iDepthPlugin", "Failed to attach shm");
    return;
  } else if ((sem_invalid = sem_init(&buf->sem, 1, 1) < 0)) {
    LOGE("D435iDepthPlugin", "Failed to initialize semaphore");
    return;
  }
  shmvalid = true;
}

void D435iDepthPlugin::OnNewDepthFrame(const float *_image,
    unsigned int _width, unsigned int _height, unsigned int _depth,
    const std::string &_format) {
  if (!model->IsActive()) {
    parentSensor->SetActive(true);
  } else {
    memcpy((void *)buf->data[buf->writeind], (void *)_image, nvar::DEPTH_SIZE);
    sem_wait(&buf->sem);
    std::swap(buf->writeind, buf->swapind);
    buf->lastop = 1;
    sem_post(&buf->sem);
  }
}
