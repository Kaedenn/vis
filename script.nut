

local RED = [1, 0.2, 0.2, 0.3, 0.1, 0.1];
local GREEN = [0.2, 1.0, 0.2, 0.1, 0.3, 0.1];
local BLUE = [0.2, 0.2, 1.0, 0.1, 0.1, 0.3];

function println(what) {
  print(what);
  print("\n");
}

// n x y ux uy rad urad ds uds theta utheta life ulife
//   r g b ur ug ub force limit blender

Bowser <- {}

Bowser.Tracks <- [
  /* 1 */
  [[1.13, 1.51],
   [1.50, 1.87],
   [1.86, 2.24],
   [2.23, 3.51],
   [3.34, 3.75],
   [3.58, 4.00],
   [4.13, 4.55],
   [4.38, 4.79],
   [4.93, 5.34],
   [5.18, 5.59],
   [7.02, 7.39],
   [7.38, 7.76],
   [7.75, 8.13],
   [8.12, 9.39],
   [9.22, 9.64],
   [9.47, 9.88],
   [9.71, 10.37],
   [10.51, 11.17],
   [11.31, 11.96],
   [13.33, 13.71],
   [13.70, 14.07],
   [14.06, 14.44],
   [14.43, 15.71],
   [15.54, 15.95],
   [15.78, 16.20],
   [16.33, 16.75],
   [16.58, 16.99],
   [17.13, 17.54],
   [17.38, 17.79]]
]

function schedule_emit(when, nparticles, where, radius, speed, angle, life,
                       color, force, limit, blender) {
  emit(__flist_instance__, when, nparticles,
    where[0], where[1], where[2], where[3],
    radius[0], radius[1],
    speed[0], speed[1],
    angle[0], angle[1],
    life[0], life[1],
    color[0], color[1], color[2], color[3], color[4], color[5],
    force, limit, blender);
}

function schedule_bowser() {
  audio("media/Bowser.wav");
  foreach (track in Bowser.Tracks) {
    foreach (times in track) {
      local start = times[0];
      local stop = times[1];
      println("Emitting from " + start + " to " + stop);
      local when = floor(start * (FPS_LIMIT-1)); /* schedule based on the fps */
      local nparticles = 100;
      local where = [WIDTH/2, HEIGHT/2, 0, 0];
      local radius = [1, 0.2];
      local speed = [2, 1];
      local angle = [PI, PI];
      local life = [100, 20];
      local force = FRICTION;
      local limit = SPRINGBOX;
      local blender = BLEND_LINEAR;
      schedule_emit(when, nparticles, where radius, speed, angle, life, BLUE, force, limit, blender);
    }
  }
}

function schedule_test() {
  audio("media/Bowser.wav");
  local where = [WIDTH/2, HEIGHT/2, WIDTH/2, 0];
  local radius = [2.0, 0.5];
  local speed = [1.0, 1.2];
  local life = [FPS_LIMIT*3/2, FPS_LIMIT/2];
  local force = GRAVITY;
  local limit = SPRINGBOX;
  local blender = BLEND_LINEAR;
  local times = [113, 123, 133];
  foreach (t in times) {
    schedule_emit(t-75, 8000, where, radius, speed, [PI/2, 0], life, GREEN, force, limit, blender);
  }
}

schedule_bowser()

