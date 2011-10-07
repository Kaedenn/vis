#!/usr/bin/env python

"""
particle.py: Simple circular particle with the following attributes:
  x, y
  dx, dy
  radius
  lifetime: number of times to call tick() before the particle "dies"
  life: number of times left before the particle "dies"

Each particle has a list of "forces" and "limiters" of the following
form:

force: Particle -> (dx, dy)
limit: Particle -> (x, y, dx, dy)

The particle will add the values returned by a force to its own, while
limiters set the values.
"""

class Particle(object):
  def __init__(self, x, y, radius, lifetime, **attributes):
    self._x = x
    self._y = y
    self._dx = 0
    self._dy = 0
    self._forces = []
    self._limits = []
    self._radius = radius
    self._lifetime = lifetime
    self._life = lifetime
    self._attributes = attributes
  
  def __nonzero__(self):
    return self._life > 0
  
  def addForce(self, force):
    self._forces.append(force)
  
  def addLimit(self, limit):
    self._limits.append(limit)
  
  def push(self, dx, dy):
    self._dx += dx
    self._dy += dy
  
  def tick(self):
    if self._life > 0:
      self._x += self._dx
      self._y += self._dy
      for force in self._forces:
        self.push(*force(self))
      for limit in self._limits:
        x, y, dx, dy = limit(self)
        self._x = x
        self._y = y
        self._dx = dx
        self._dy = dy
      self._life -= 1
  
  def getX(self): return self._x
  def getY(self): return self._y
  def getDx(self): return self._dx
  def getDy(self): return self._dy
  def getRadius(self): return self._radius
  def getLife(self): return self._life
  def getAttributes(self): return self._attributes

