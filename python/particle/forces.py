#!/usr/bin/env python

def Gravity(dy):
  "Gravity(dy): creates a constant force of '<0, dy>'"
  def force(particle):
    return dy
  return force

def Wind(dx, dy):
  "Wind(dx, dy): creates a constant force of '<dx, dy>'"
  def force(particle):
    return dx, dy

