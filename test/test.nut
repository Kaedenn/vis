local COLOR_RED =   [1.0, 0.2, 0.2, 0.3, 0.1, 0.1];
local COLOR_GREEN = [0.2, 1.0, 0.2, 0.1, 0.3, 0.1];
local COLOR_BLUE =  [0.2, 0.2, 1.0, 0.1, 0.1, 0.3];
local COLOR_WHITE = [1.0, 1.0, 1.0, 0.0, 0.0, 0.0];

function println(what) {
    print(what);
    print("\n");
}

function fmod(val, mod) {
    while (val > mod) { val -= mod; }
    return val;
}

function join(str, seq) {
    return seq.reduce(function(prev, next) {
        if (prev != "") {
            return prev + str + next;
        } else {
            return next;
        }
    })
}

function item_to_string(label, midp, adj) {
    return label + "=" + midp + "+-" + adj;
}

function emit_to_string(when, n, pos, rad, vel, ang, age, c, force, lim, blend) {
    print("emit(_");
    print(", n=" + n);
    print(", " + item_to_string("x", pos[0], pos[2]));
    print(", " + item_to_string("y", pos[1], pos[3]));
    print(", " + item_to_string("r", rad[0], rad[1]));
    print(", " + item_to_string("vel", vel[0], vel[1]));
    print(", " + item_to_string("t", ang[0], ang[1]));
    print(", " + item_to_string("age", age[0], age[1]));
    print(", " + item_to_string("r", c[0], c[3]));
    print(", " + item_to_string("g", c[1], c[4]));
    print(", " + item_to_string("b", c[2], c[5]));
    print(", force=" + force);
    print(", limit=" + lim);
    print(", blend=" + blend);
    print(")\n");
}

function schedule_emit(when, nparticles, where, radius, speed, angle, life,
                       color, force, limit, blender) {
    emit_to_string(when, nparticles, where, radius, speed, angle, life, color,
                   force, limit, blender);
    emit(__flist_instance__,
         when,      nparticles,
         where[0],  where[1],   where[2], where[3],
         radius[0], radius[1],
         speed[0],  speed[1],
         angle[0],  angle[1],
         life[0],   life[1],
         color[0],  color[1],   color[2],
         color[3],  color[4],   color[5],
         force, limit, blender);
}

// n x y ux uy rad urad ds uds theta utheta life ulife
//     r g b ur ug ub force limit blender

function test_impl(when=0, n=1, where=[VIS_WIDTH/2, VIS_HEIGHT/2, 0, 0], radius=[2, 1], speed=[2, 1], ang=[0, 0], life=[100, 0], color=COLOR_WHITE, force=VIS_FRICTION, limit=VIS_SPRINGBOX, blender=VIS_BLEND_LINEAR) {
    schedule_emit(when, n, where, radius, speed, ang, life, color, force, limit, blender);
}

function test_swipe() {
    local start = 0;
    local end = 2;
    local duration = start - end;
    local angle_l = 4*PI/3;
    local angle_r = 5*PI/3;
    local grains = 20;
    local dt_l = angle_l / grains;
    local dt_r = angle_r / grains;
    local where = [VIS_WIDTH/2, VIS_HEIGHT/2, 0, 0];
    local radius = [1, 0.2];
    local speed = [2, 1];
    local life = [100, 20];
    for (local grain = 0; grain < grains; grain += 1) {
        local ang_l = fmod(angle_l - dt_l * grain, 2*PI);// * 180 / PI;
        local ang_r = fmod(angle_r + dt_r * grain, 2*PI);// * 180 / PI;
        local when = floor((start + duration * (grain / grains)) * VIS_FPS_LIMIT);
        //test_impl(when, 1000, where, radius, speed, [ang_l, 0], life);
        //test_impl(when, 1000, where, radius, speed, [ang_r, 0], life);
        println("grain " + grain + " of " + grains + ": " + 2*PI*grain/grains);
        test_impl(when, 1000, where, radius, speed, [(2*PI*grain/grains), 0], life);
    }
}

function test_basic() {

}

test_swipe()
