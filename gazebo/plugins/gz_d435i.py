import ctypes
import os
import numpy as np

rootdir = "./"
while not "ai-race-sim" in os.listdir(rootdir):
  rootdir += "../"
rootdir += "ai-race-sim/"
path = rootdir + "gazebo/build/plugins/libgz_d435i.so"
libgz_d435i = ctypes.cdll.LoadLibrary(path)
libgz_d435i.gz_d435i_constructor.restype = ctypes.c_int
libgz_d435i.gz_d435i_destructor.restype = ctypes.c_int
libgz_d435i.gz_d435i_grab_frames.argtypes = [ctypes.c_void_p, ctypes.c_void_p]

def convertDepthToRGB(frame, colormap="rgb"):
  img = np.clip(frame / 10.0, 0, 1) # divide by the max depth
  if colormap == "rgb":
    ones = np.ones(img.shape)
    r = np.multiply(img <= .2, ones) + \
        np.multiply(img > .2, np.multiply(img <= .4, (.2 - img + .2) * 5))
    g = np.multiply(img <= .2, img * 5) + \
        np.multiply(img > .2, np.multiply(img <= .6, ones)) + \
        np.multiply(img > .6, np.multiply(img <= .8, (.2 - img + .6) * 5))
    b = np.multiply(img > .4, np.multiply(img <= .6, (img - .4) * 5)) + \
        np.multiply(img > .6, np.multiply(img <= .8, ones)) + \
        np.multiply(img > .8, (.2 - img + .8) * 5)
    newframe = np.array([b, g, r])
    newframe = np.moveaxis(newframe, 0, -1)
    return newframe
  else:
    img = np.expand_dims(img, axis=-1)
    return np.concatenate([img, img, img], axis=-1)

class GzD435i:
  def __init__(self):
    self.valid = libgz_d435i.gz_d435i_constructor()
    self.width = 640
    self.height = 360
    self.depthbuf = ctypes.cast(
        (ctypes.c_float * self.width * self.height)(),
        ctypes.POINTER(ctypes.c_float))
    self.colorbuf = ctypes.cast(
        (ctypes.c_uint8 * self.width * self.height * 3)(),
        ctypes.POINTER(ctypes.c_uint8))

    self.depthFrame = np.ctypeslib.as_array(self.depthbuf,
        shape=(self.height, self.width))
    self.colorFrame = np.ctypeslib.as_array(self.colorbuf,
        shape=(self.height, self.width, 3))

  def __del__(self):
    if libgz_d435i != None:
      libgz_d435i.gz_d435i_destructor()

  def read(self):
    if self.valid == 2:
      libgz_d435i.gz_d435i_grab_frames(self.depthbuf, self.colorbuf)
    return self.colorFrame, self.depthFrame

  @property
  def depth_uint16(self):
    return (self.depthFrame * 1000.0).asType(np.uint16)
