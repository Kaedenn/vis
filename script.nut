
const pi = 3.141592653569;

// n x y ux uy rad urad ds uds theta utheta life ulife r g b ur ug ub force limit blender

function schedule_emit(when, nparticles, where, radius, speed, angle, life, color, force, limit, blender) {
  emit(__flist_instance__, when, nparticles,
    where[0], where[1], where[2], where[3],
    radius[0], radius[1],
    speed[0], speed[1],
    angle[0], angle[1],
    life[0], life[1],
    color[0], color[1], color[2], color[3], color[4], color[5],
    force, limit, blender);
}

function main() {
  local pos = [0, 0, 0, 0];
  local radius = [2.0, 0.5];
  local speed = [1.0, 1.2];
  local life = [FPS_LIMIT*3/2, FPS_LIMIT/2];
  local red = [1, 0.2, 0.2, 0.3, 0.1, 0.1];
  local green = [0.2, 1.0, 0.2, 0.1, 0.3, 0.1];
  local blue = [0.2, 0.2, 1.0, 0.1, 0.1, 0.3];
  local force = GRAVITY;
  local limit = SPRINGBOX;
  local blender = BLEND_LINEAR;
  print("scheduling\n");
  for (local i = 0; i < 100; i += 1) {
    if ((i*2) % 20 == 0) {
      schedule_emit(i*5, 8000, [WIDTH/2, HEIGHT/2, WIDTH/2, 0], radius, speed, [pi/2, 0], life, green, force, limit, blender);
    } else if ((i*2) % 10 == 0) {
      schedule_emit(i*5, 8000, [WIDTH/2, HEIGHT/2, WIDTH/2, 0], radius, speed, [3*pi/2, 0], life, blue, force, limit, blender);
    }
  }
}

main()


