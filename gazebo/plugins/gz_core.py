import os
import subprocess
import time
import ctypes

rootdir = "./"
while not "ai-race-sim" in os.listdir(rootdir):
  rootdir += "../"
rootdir += "ai-race-sim/"
path = rootdir + "gazebo/build/plugins/libgz_core.so"
libgz_core = ctypes.cdll.LoadLibrary(path)

class GzCore:
  def __init__(self, render=False, world="racetrack.world"):
    if render:
      os.system("pkill gzclient")
      os.system("pkill gzserver")
      self.C = subprocess.Popen(["gzclient", "--verbose"])
      time.sleep(0.1)
    else:
      self.C = None
    self.S = subprocess.Popen(["gzserver", "--verbose",
      rootdir + "gazebo/resources/" + world])
    libgz_core.gz_core_constructor()
    time.sleep(5) # wait until the simulation is opened
    self.killed = False
    self.world = world

  def stop(self):
    if not self.killed:
      libgz_core.gz_core_destructor()
      if self.C != None:
        cpid = self.C.pid
        self.C.terminate()
        time.sleep(0.5)
        try:
          os.kill(cpid, 0)
          self.C.kill()
          print("\033[01;31m[E] [gzcore] Forced kill client\x1b[0m")
        except:
          pass
      spid = self.S.pid
      self.S.terminate()
      time.sleep(0.5)
      try:
        os.kill(spid, 0)
        self.S.kill()
        print("\033[01;31m[E] [gzcore] Forced kill server\x1b[0m")
      except:
        pass
      self.killed = True

  def __del__(self):
    self.stop()
