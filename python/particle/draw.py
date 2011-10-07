#!/usr/bin/env python

import pyglet
from pyglet.gl import *
import math

NUMSIDES = 50
TANFACTOR = math.tan(2*math.pi/NUMSIDES)
RADFACTOR = math.cos(2*math.pi/NUMSIDES)

PARTIMAGE = pyglet.image.load("particle/part.png")

def draw(window, particle):
  x, y, r = particle.getX(), particle.getY(), particle.getRadius()
  color = particle.getAttributes().get("color", (0,100,200))
  cx, cy = r, 0
  #glLoadIdentity()
  glBegin(GL_LINE_LOOP)
  glColor3f(*color)
  glVertex2f(x - r, y - r)
  glVertex2f(x + r, y - r)
  glVertex2f(x + r, y + r)
  glVertex2f(x - r, y + r)
  glEnd()

