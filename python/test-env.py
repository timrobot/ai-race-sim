#!/usr/bin/env python3
from env_racetrack import RacetrackEnv
import time

if __name__ == "__main__":
  env = RacetrackEnv()
  env.reset()

  while True:
    gyro, color, depth = env.observe()
    rew, done, _ = env.step([0, 3.0])
    env.render()
    time.sleep(0.1)
