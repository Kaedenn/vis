#!/usr/bin/env python

import math
import random
import select
import sys

import async
import dllist
import particle.particle
import particle.draw
import particle.forces
import particle.limits

import helper
from helper import debug

import pyglet
from pyglet.gl import *
from pyglet.window import key

def argparse(argv):
  from optparse import OptionParser
  parser = OptionParser()
  options = (
    (("", "--width"), {"default": 800, "metavar": "W",
                       "help": "use W for the window width (default 800)"}),
    (("", "--height"), {"default": 600, "metavar": "H",
                        "help": "use H for the window height (default 600)"}),
    (("-d", "--debug"), {"default": False, "action": "store_true",
                         "help": "enable debug mode"}),
    (("", "--bgcolor"), {"default": "0,0,0", "metavar": "R,G,B",
                         "help": "set the background color (default: 0,0,0)"}),
    (("", "--fps"), {"default": 40, "type": "int", "metavar": "FPS",
                     "help": "set the fps (default: 40)"})
  )
  for a, kw in options:
    parser.add_option(*a, **kw)
  opts, args = parser.parse_args()
  opts.bgcolor = tuple(int(n) for n in opts.bgcolor.split(","))
  return opts, args

def makewindow(opts):
  window = pyglet.window.Window()
  window.set_size(opts.width, opts.height)
  window.set_caption("Visualization")
  return window

def makeparticle(x, y, r, ds, theta, opts):
  # lifetime between 10 and 20 seconds
  life = int(opts.fps * helper.rand(10, 20))
  dx = ds * math.cos(theta)
  dy = ds * math.sin(theta)
  # mid blue color
  attrs = {"color": (0, 100, 200)}
  p = particle.particle.Particle(x, y, r, life, **attrs)
  # bounce off the walls
  p.addLimit(particle.limits.SpringBox(0, 0, opts.height, opts.width))
  p.push(dx, dy)
  return p

def kickcommand(tokens, particles, opts):
  try:
    for _ in xrange(int(tokens[1])):
      # start at the middle
      x = opts.width / 2
      y = opts.height / 2
      # random radius between 2 and 6
      r = helper.rand(2, 6)
      # uniform velocity between 2/r and 20/r
      ds = helper.rand(2, 20) / r
      # any angle
      theta = helper.rand(0, 2*math.pi)
      particles.pushback(makeparticle(x, y, r, ds, theta, opts))
  except ValueError, e:
    helper.writeto(sys.stderr, "usage: kick <number of particles>\n")

def strumcommand(tokens, particles, opts):
  try:
    for _ in xrange(int(tokens[1])):
      # random radius between 2 and 6
      r = helper.rand(2, 6)
      # start at the bottom middle
      x = opts.width / 2
      y = 0
      # uniform velocity between 2/r and 20/r
      ds = helper.rand(2, 20) / r
      # starting angle between pi+1/12 and 2pi-1/12
      theta = helper.rand(13*math.pi/12, 23*math.pi/12)
      particles.pushback(makeparticle(x, y, r, ds, theta, opts))
  except ValueError, e:
    helper.writeto(sys.stderr, "usage: strum <number of particles>\n")

def command(dt, stdin, particles, window, opts):
  cmd = async.read(stdin)
  tokens = cmd.split()
  if tokens != []:
    debug(opts.debug, "cmd: %r", cmd)
    if tokens[0] == "kick":
      kickcommand(tokens, particles, opts)
    elif tokens[0] == "strum":
      strumcommand(tokens, particles, opts)
    elif tokens[0] == "clear":
      particles.clear()
    else:
      helper.writeto(sys.stderr, "Invalid command\n")
  if cmd != "":
    helper.writeto(sys.stdout, "? ")
    sys.stdout.flush()

def draw(window, particles):
  glClear(GL_COLOR_BUFFER_BIT)
  for p in particles:
    particle.draw.draw(window, p.value())
    p.value().tick()
    if not p.value():
      particles.remove(p)

def mainloop(window, opts):
  stdin = async.initialize(sys.stdin)
  particles = dllist.dllist()
  pyglet.app.clock.set_fps_limit(opts.fps)
  pyglet.app.clock.schedule(command, stdin=stdin, particles=particles,
                            window=window, opts=opts)
  @window.event
  def on_draw():
    draw(window, particles)
  
  pyglet.app.run()
  
  #while True:
  #  # handle console commands
  #  cmd = async.read(stdin)
  #  tokens = cmd.split()
  #  if tokens != []:
  #    debug(opts.debug, "cmd: %r", cmd)
  #    # kick <number>
  #    if tokens[0] == "kick": kickcommand(tokens, particles, opts)
  #    # strum <number>
  #    elif tokens[0] == "strum": strumcommand(tokens, particles, opts)
  #    # clear
  #    elif tokens[0] == "clear": particles.clear()
  #    else:
  #      sys.stderr.write("Invalid command\n")
  #  if cmd != "":
  #    sys.stdout.write("? ")
  #    sys.stdout.flush()
  #  # draw background
  #  #surface.fill(opts.bgcolor)
  #  # handle Pygame events
  #  #pygame.event.pump()
  #  #for event in pygame.event.get():
  #  #  if event.type == pyg.QUIT:
  #  #    return
  #  #  if event.type == pyg.KEYDOWN:
  #  #    if event.key == pyg.K_p:
  #  #      particles.pushback(makeparticle(opts))
  #  # draw particles
  #  for p in particles:
  #    particle.draw.draw(surface, p.value())
  #    p.value().tick()
  #    if not p.value():
  #      particles.remove(p)
  #  # render to screen
  #  #window.blit(surface, (0,0))
  #  #pygame.display.flip()
  #  #clock.tick(fps)

def drive():
  opts, args = argparse(sys.argv)
  debug(opts.debug, "debugging enabled")
  debug(opts.debug, "parsed args: %r to %r", sys.argv, args)
  debug(opts.debug, "width, height: %s, %s", opts.width, opts.height)
  debug(opts.debug, "bgcolor: %r", opts.bgcolor)
  window = makewindow(opts)
  
  try:
    sys.stdout.write("? ")
    sys.stdout.flush()
    mainloop(window, opts)
  except (EOFError, KeyboardInterrupt), e:
    pass
  
  sys.stdout.write("\n")

if __name__ == "__main__":
  drive()

