import os
import sys
rootdir = "./"
while not "ai-race-sim" in os.listdir(rootdir):
  rootdir += "../"
rootdir += "ai-race-sim/"
sys.path.append(rootdir + "gazebo/plugins")
import signal
import time
import gz_core
import gz_rccar
import gz_d435i
import cv2
import numpy as np
from util import Log
import math
import gym
from gym import spaces

# TODO(Tim): make future version fully compatible with openai's framework
# take a look at gym.envs.registration.register

class RacetrackEnv:
  def __init__(self):
    self.gz = gz_core.GzCore(render=True, world="racetrack.world")
    self.rccar = gz_rccar.GzRCCar()
    self.d435i = gz_d435i.GzD435i()

    # OpenAI compatibility
    self._elapsed_steps = 0
    self._max_elapsed_steps = 200
    self._episode_started_at = None
    self._info = {}

    # observation: joint pos[12], pitch, roll
    self.observation_space = spaces.box.Box(
        low=-3.141592654, high=3.141592654, shape=[3])
    self.action_space = spaces.box.Box(low=-1, high=1, shape=[2])

  def __del__(self):
    self.stop()

  def step(self, action):
    self.rccar.set_actuators(action[0], action[1])
    self._elapsed_steps += 1
    reward, done = self.rewardFn(self._info), self.doneFn(self._info)
    return reward, done, self._info

  def observe(self):
    c, d = self.d435i.read()
    c = np.flip(c.astype(np.float), 2) / 255.0
    g, t = self.rccar.get_sensors()
    self._info = {
        "timestamp": t,
        "gyro": g,
        "colorFrame": c,
        "depthFrame": d
        }
    roll, pitch, yaw = g
    if self._elapsed_steps == 0:
      for k in self._info:
        if isinstance(self._info[k], np.ndarray):
          self._episode_started_at[k] = self._info[k].copy()
        else:
          self._episode_started_at[k] = self._info[k]
    return g, c, d

  def reset(self):
    self.rccar.reset()
    # wait for everything
    self.rccar.wait_for_reset()
    self._elapsed_steps = 0
    self._episode_started_at = {}

  def stop(self):
    self.gz.stop()
    time.sleep(0.5)

  def render(self):
    depth = self.depthToRGB(self._info["depthFrame"])
    cv2.imshow("Gazebo D435i", np.hstack([self._info["colorFrame"], depth]))
    cv2.waitKey(1)

  def rewardFn(self, state):
    return 0

  def doneFn(self, state):
    return self._elapsed_steps >= self._max_elapsed_steps

  def depthToRGB(self, frame):
    return gz_d435i.convertDepthToRGB(frame)
