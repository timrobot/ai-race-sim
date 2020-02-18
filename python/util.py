import os
import numpy as np
import time
import datetime

class Log:
  VERBOSE = 5
  DEBUG = 4
  INFO = 3
  WARNING = 2
  ERROR = 1
  NONE = 0
  __log_level = VERBOSE
  __fp = None

  @staticmethod
  def setLevel(lvl):
    Log.__log_level = lvl

  @staticmethod
  def timestr():
    t = time.time()
    msec = str(int(t * 1000) % 1000)
    s = datetime.datetime.fromtimestamp(t).strftime("%Y-%m-%d::%H:%M:%S")
    m = "." + ("0" * (3 - len(msec))) + msec
    return s + m + (" " * (22 - len(s)))

  @staticmethod
  def __call__(s):
    print("\033[01;35m[V] %s[%s]  %s\x1b[0m" % (Log.timestr(), "log", str(s)))
    if Log.__fp:
      Log.__fp.write("%s\n" % s)

  @staticmethod
  def V(tag, s):
    if Log.__log_level >= Log.VERBOSE:
      print("\033[01;34m[V] %s[%s]  %s\x1b[0m" % (Log.timestr(), tag, str(s)))
      if Log.__fp:
        Log.__fp.write("%s\n" % s)

  @staticmethod
  def D(tag, s):
    if Log.__log_level >= Log.DEBUG:
      print("\033[01;36m[D] %s[%s]  %s\x1b[0m" % (Log.timestr(), tag, str(s)))
      if Log.__fp:
        Log.__fp.write("%s\n" % s)

  @staticmethod
  def I(tag, s):
    if Log.__log_level >= Log.INFO:
      print("\033[01;32m[I] %s[%s]  %s\x1b[0m" % (Log.timestr(), tag, str(s)))
      if Log.__fp:
        Log.__fp.write("%s\n" % s)

  @staticmethod
  def W(tag, s):
    if Log.__log_level >= Log.WARNING:
      print("\033[01;33m[W] %s[%s]  %s\x1b[0m" % (Log.timestr(), tag, str(s)))
      if Log.__fp:
        Log.__fp.write("%s\n" % s)

  @staticmethod
  def E(tag, s):
    if Log.__log_level >= Log.ERROR:
      print("\033[01;31m[E] %s[%s]  %s\x1b[0m" % (Log.timestr(), tag, str(s)))
      if Log.__fp:
        Log.__fp.write("%s\n" % s)

  @staticmethod
  def setFile(fname):
    Log.__fp = open(fname, "w")
