

local RED =   [1.0, 0.2, 0.2, 0.3, 0.1, 0.1];
local GREEN = [0.2, 1.0, 0.2, 0.1, 0.3, 0.1];
local BLUE =  [0.2, 0.2, 1.0, 0.1, 0.1, 0.3];
local WHITE = [1.0, 1.0, 1.0, 0.0, 0.0, 0.0];

function println(what) {
  print(what);
  print("\n");
}

function fmod(val, mod) {
  while (val > mod) { val -= mod; }
  return val;
}

// n x y ux uy rad urad ds uds theta utheta life ulife
//   r g b ur ug ub force limit blender

Bowser <- {}

Bowser.Tracks <- [
  /* 1 */
  [[1.13, 1.51, 0],
   [1.50, 1.87, 0],
   [1.86, 2.24, 0],
    [2.23, 3.51, 1],
   [3.34, 3.75, 0],
   [3.58, 4.00, 0],
   [4.13, 4.55, 0],
   [4.38, 4.79, 0],
   [4.93, 5.34, 0],
   [5.18, 5.59, 0],
   [7.02, 7.39, 0],
   [7.38, 7.76, 0],
   [7.75, 8.13, 0],
    [8.12, 9.39, 1],
   [9.22, 9.64, 0],
   [9.47, 9.88, 2],
   [9.71, 10.37, 2],
   [10.51, 11.17, 2],
   [11.31, 11.96, 2],
   [13.33, 13.71, 0],
   [13.70, 14.07, 0],
   [14.06, 14.44, 0],
    [14.43, 15.71, 1],
   [15.54, 15.95, 0],
   [15.78, 16.20, 0],
   [16.33, 16.75, 0],
   [16.58, 16.99, 0],
   [17.13, 17.54, 0],
   [17.38, 17.79, 0]]
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

function schedule_blue_radial(times) {
  local start = times[0];
  local stop = times[1];
  println("Emitting from " + start + " to " + stop);
  local when = floor(start * FPS_LIMIT); /* schedule based on the fps */
  local nparticles = 100;
  local where = [WIDTH/2, HEIGHT/2, 0, 0];
  local radius = [1, 0.2];
  local speed = [2, 1];
  local angle = [PI, PI];
  local life = [100, 20];
  local force = FRICTION;
  local limit = SPRINGBOX;
  local blender = BLEND_LINEAR;
  schedule_emit(when, nparticles, where, radius, speed, angle, life, BLUE,
                force, limit, blender);
}

function schedule_wings(times) {
  local start = times[0];
  local end = times[1];
  local duration = end - start;
  local angle_left = 4*PI/3;
  local angle_right = 5*PI/3;
  local grains = 10;
  local dtheta_left = angle_left / grains;
  local dtheta_right = angle_right / grains;
  local where = [WIDTH/2, HEIGHT/2, 0, 0];
  local radius = [1, 0.2];
  local speed = [2, 1];
  local life = [100, 20];
  local force = FRICTION;
  local limit = SPRINGBOX;
  local blender = BLEND_LINEAR;
  local grain = 0;
  while (grain < grains) {
    local angle_l = fmod(angle_left - dtheta_left * grain, 2*PI);
    local angle_r = fmod(angle_right + dtheta_right * grain, 2*PI);
    local when = floor((start + duration * (grain / grains)) * FPS_LIMIT);
    println("Scheduling wings at " + when + ", " + angle_l + ", " + angle_r);
    schedule_emit(when, 100, where, radius, speed, [angle_l, 0], life, WHITE,
                  force, limit, blender);
    schedule_emit(when, 100, where, radius, speed, [angle_r, 0], life, WHITE,
                  force, limit, blender);
    grain += 1;
  }
}

function schedule_bowser() {
  audio("media/Bowser.wav");
  foreach (track in Bowser.Tracks) {
    foreach (times in track) {
      if (times[2] == 0) {
        schedule_blue_radial(times);
      } else if (times[2] == 1) {
        local curr = times[0];
        while (curr < (times[1] - 0.2)) {
          schedule_blue_radial([curr, times[1]]);
          curr += 0.1;
        }
      } else if (times[2] == 2) {
        schedule_wings(times);
      }
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
    local when = floor(t * FPS_LIMIT);
    schedule_emit(when, 8000, where, radius, speed, [PI/2, 0], life, GREEN,
                  force, limit, blender);
  }
}

schedule_bowser()

