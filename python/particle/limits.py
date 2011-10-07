#!/usr/bin/env python

def Box(top, left, bottom, right):
  "Box(top, left, bottom, right): clamps the particle inside the box"
  def limit(particle):
    x, y, r = particle.getX(), particle.getY(), particle.getRadius()
    if x < left + r:
      x = left + r
    elif x > right - r:
      x = right - r
    if y < top + r:
      y = top + r
    elif y + r > bottom:
      y = bottom - r
    return x, y, particle.getDx(), particle.getDy()
  return limit

def SpringBox(top, left, bottom, right):
  "SpringBox(top, left, bottom, right): particles bounce off the walls"
  def limit(particle):
    x, y, r = particle.getX(), particle.getY(), particle.getRadius()
    dx, dy = particle.getDx(), particle.getDy()
    if x < left + r:
      x = left + r
      dx = -dx
    elif x > right - r:
      x = right - r
      dx = -dx
    if y < top + r:
      y = top + r
      dy = -dy
    elif y + r > bottom:
      y = bottom - r
      dy = -dy
    return x, y, dx, dy
  return limit

