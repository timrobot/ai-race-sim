import ctypes
import os
import numpy as np

rootdir = "./"
while not "ai-race-sim" in os.listdir(rootdir):
  rootdir += "../"
rootdir += "ai-race-sim/"
path = rootdir + "gazebo/build/plugins/libgz_rccar.so"
libgz_rccar = ctypes.cdll.LoadLibrary(path)
libgz_rccar.gz_rccar_set_actuators.argtypes = [\
    ctypes.c_double, ctypes.c_double]
libgz_rccar.gz_rccar_get_sensors.argtypes = \
    [ctypes.c_void_p, ctypes.c_void_p]

class GzRCCar:
  def __init__(self):
    libgz_rccar.gz_rccar_constructor()
    self.gyro_array = ctypes.cast((ctypes.c_double * 3)(),
        ctypes.POINTER(ctypes.c_double))
    self.timestamp_array = ctypes.cast((ctypes.c_double * 1)(),
        ctypes.POINTER(ctypes.c_double))

  def __del__(self):
    if libgz_rccar != None:
      libgz_rccar.gz_rccar_destructor()

  def set_actuators(self, steer=0, motor=0):
    """
    Args:
      steer: int,
      motor: int
    """
    libgz_rccar.gz_rccar_set_actuators(steer, motor)

  def get_sensors(self):
    """
    Returns:
      [ gyro_x, gyro_y, gyro_z ],
      timestamp
    """
    libgz_rccar.gz_rccar_get_sensors(
        self.gyro_array, self.timestamp_array)
    return np.array([
        self.gyro_array[0], self.gyro_array[1], self.gyro_array[2]
      ]), self.timestamp_array[0]

  def reset(self):
    libgz_rccar.gz_rccar_reset()

  def wait_for_reset(self):
    # wait until finished
    libgz_rccar.gz_rccar_wait_for_reset()
