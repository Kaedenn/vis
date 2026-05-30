/**
 * Emit context, which encapsulates the state of a single emission rule
 *
 * Final particle position is calculated via the following steps:
 *
 * angle = randrange(theta - utheta, theta + utheta)
 * offset = randrange(s - us, s + us)
 * px = randrange(x - ux, x + ux) + offset * Math.cos(angle)
 * py = randrange(y - uy, y + uy) + offset * Math.sin(angle)
 *
 */

export const ForceFunc = Object.freeze({
    DEFAULT_FORCE: 'Vis.DEFAULT_FORCE',
    FORCE_GRAVITY: 'Vis.FORCE_GRAVITY',
});

export const LimitFunc = Object.freeze({
    DEFAULT_LIMIT: 'Vis.DEFAULT_LIMIT',
    LIMIT_BOX: 'Vis.LIMIT_BOX',
    LIMIT_SPRINGBOX: 'Vis.LIMIT_SPRINGBOX',
});

export const BlendFunc = Object.freeze({
    DEFAULT_BLEND: 'Vis.DEFAULT_BLEND',
    BLEND_LINEAR: 'Vis.BLEND_LINEAR',
    BLEND_PARABOLIC: 'Vis.BLEND_PARABOLIC',
    BLEND_QUARTIC: 'Vis.BLEND_QUARTIC',
    BLEND_SINE: 'Vis.BLEND_SINE',
    BLEND_NEGGAMMA: 'Vis.BLEND_NEGGAMMA',
    BLEND_EASING: 'Vis.BLEND_EASING',
});

export class EmitContext {
    constructor() {
        // Count
        this._count = 1000;

        // Position and position variance
        this._x = 0; this._y = 0;
        this._ux = 0; this._uy = 0;

        // Linear Position along Theta and its variance
        this._s = 1; this._us = 200;

        // Velocity and velocity variance
        this._ds = 0; this._uds = 0;

        // Radius and radius variance
        this._radius = 1; this._uradius = 0;

        // Rotation/Angle and their variance
        this._theta = 0; this._utheta = Math.PI;

        // Lifetime and lifetime variance (in milliseconds; 1 for single frame)
        this._life = 1; this._ulife = 0;

        // Color and color variance
        this._r = 1; this._g = 1; this._b = 1;
        this._ur = 0; this._ug = 0; this._ub = 0;

        // Generic parameters
        this._depth = 0;
        this._force = ForceFunc.DEFAULT_FORCE;
        this._limit = LimitFunc.DEFAULT_LIMIT;
        this._blender = BlendFunc.DEFAULT_BLEND;
        this._tag = 0;
    }

    toJSON() {
        return {
            count: this._count,
            x: this._x, y: this._y,
            ux: this._ux, uy: this._uy,
            s: this._s, us: this._us,
            ds: this._ds, uds: this._uds,
            radius: this._radius, uradius: this._uradius,
            theta: this._theta, utheta: this._utheta,
            life: this._life, ulife: this._ulife,
            r: this._r, g: this._g, b: this._b,
            ur: this._ur, ug: this._ug, ub: this._ub,
            depth: this._depth,
            force: this._force,
            limit: this._limit,
            blender: this._blender,
            tag: this._tag
        };
    }

    // Count
    get count() { return this._count; }
    set count(v) { this._count = typeof v === 'number' ? v : 0; }

    // --- Position (x, y, ux, uy) ---
    get x() { return this._x; }
    set x(v) { this._x = v; }
    get y() { return this._y; }
    set y(v) { this._y = v; }
    get ux() { return this._ux; }
    set ux(v) { this._ux = v; }
    get uy() { return this._uy; }
    set uy(v) { this._uy = v; }

    updatePosition(x, y, ux = this._ux, uy = this._uy) {
        if (x !== undefined) this._x = x;
        if (y !== undefined) this._y = y;
        if (ux !== undefined) this._ux = ux;
        if (uy !== undefined) this._uy = uy;
    }

    // --- Linear Displacement ---
    get s() { return this._s; }
    set s(v) { this._s = v; }
    get us() { return this._us; }
    set us(v) { this._us = v; }

    // --- Velocity ---
    get ds() { return this._ds; }
    set ds(v) { this._ds = v; }
    get uds() { return this._uds; }
    set uds(v) { this._uds = v; }

    updateDisplacement(s, us = this._us, ds = this._ds, uds = this._uds) {
        if (s !== undefined) this._s = s;
        if (us !== undefined) this._us = us;
        if (ds !== undefined) this._ds = ds;
        if (uds !== undefined) this._uds = uds;
    }

    // --- Radius ---
    get radius() { return this._radius; }
    set radius(v) { this._radius = v; }
    get uradius() { return this._uradius; }
    set uradius(v) { this._uradius = v; }

    updateRadius(radius, uradius = this._uradius) {
        if (radius !== undefined) this._radius = radius;
        if (uradius !== undefined) this._uradius = uradius;
    }

    // --- Theta ---
    get theta() { return this._theta; }
    set theta(v) { this._theta = v; }
    get utheta() { return this._utheta; }
    set utheta(v) { this._utheta = v; }

    updateTheta(theta, utheta = this._utheta) {
        if (theta !== undefined) this._theta = theta;
        if (utheta !== undefined) this._utheta = utheta;
    }

    // --- Life ---
    get life() { return this._life; }
    set life(v) { this._life = v; }
    get ulife() { return this._ulife; }
    set ulife(v) { this._ulife = v; }

    updateLife(life, ulife = this._ulife) {
        if (life !== undefined) this._life = life;
        if (ulife !== undefined) this._ulife = ulife;
    }

    // --- Color (r, g, b, ur, ug, ub) ---
    get r() { return this._r; }
    set r(v) { this._r = v; }
    get g() { return this._g; }
    set g(v) { this._g = v; }
    get b() { return this._b; }
    set b(v) { this._b = v; }
    get ur() { return this._ur; }
    set ur(v) { this._ur = v; }
    get ug() { return this._ug; }
    set ug(v) { this._ug = v; }
    get ub() { return this._ub; }
    set ub(v) { this._ub = v; }

    updateColor(r, g, b, ur = this._ur, ug = this._ug, ub = this._ub) {
        if (r !== undefined) this._r = r;
        if (g !== undefined) this._g = g;
        if (b !== undefined) this._b = b;
        if (ur !== undefined) this._ur = ur;
        if (ug !== undefined) this._ug = ug;
        if (ub !== undefined) this._ub = ub;
    }

    // --- Additional properties (depth, force, limit, blender, tag) ---
    get depth() { return this._depth; }
    set depth(v) { this._depth = v; }
    get force() { return this._force; }
    set force(v) { this._force = typeof v === 'string' ? v : ForceFunc.DEFAULT_FORCE; }
    get limit() { return this._limit; }
    set limit(v) { this._limit = typeof v === 'string' ? v : LimitFunc.DEFAULT_LIMIT; }
    get blender() { return this._blender; }
    set blender(v) { this._blender = typeof v === 'string' ? v : BlendFunc.DEFAULT_BLEND; }
    get tag() { return this._tag; }
    set tag(v) { this._tag = typeof v === 'number' ? v : 0; }

    serialize(isNativeMode, timeMs, oneLine = false, frameName = null) {
        // Build a Lua table string that can be parsed by merge_emit_table
        let lua = "{\n";
        if (!isNativeMode) {
            lua += `    count = ${this._count},\n`;
        }
        lua += `    x = Vis.WIDTH / 2 + ${this._x}, y = Vis.HEIGHT / 2 + ${this._y},\n`;
        lua += `    ux = ${this._ux}, uy = ${this._uy},\n`;
        lua += `    s = ${this._s}, us = ${this._us},\n`;
        lua += `    ds = ${this._ds}, uds = ${this._uds},\n`;
        lua += `    radius = ${this._radius}, uradius = ${this._uradius},\n`;
        lua += `    theta = ${this._theta}, utheta = ${this._utheta},\n`;
        lua += `    life = ${this._life}, ulife = ${this._ulife},\n`;
        lua += `    r = ${this._r}, g = ${this._g}, b = ${this._b},\n`;
        lua += `    ur = ${this._ur}, ug = ${this._ug}, ub = ${this._ub},\n`;
        lua += `    depth = ${this._depth},\n`;
        lua += `    force = ${this._force},\n`;
        lua += `    limit = ${this._limit},\n`;
        lua += `    blender = ${this._blender},\n`;
        lua += `    tag = ${this._tag === 0 ? Math.floor(timeMs) : this._tag}\n`;
        lua += "}";

        if (oneLine) {
            lua = lua.replace(/\n\s*/g, ' ');
        }

        let result = "";
        if (isNativeMode) {
            result = `Vis.emit(Vis.flist, ${this._count}, BASE + ${Math.floor(timeMs)}, ${lua})`;
        } else {
            result = `Emit:new(${lua}):emit_at(BASE + ${Math.floor(timeMs)})`;
        }

        if (frameName) {
            result += ` -- ${frameName}`;
        }
        return result;
    }

    static FromJSON(data) {
        const ctx = new EmitContext();
        if (data.count !== undefined) ctx.count = data.count;
        ctx.updatePosition(data.x, data.y, data.ux, data.uy);
        ctx.updateDisplacement(data.s, data.us, data.ds, data.uds);
        ctx.updateRadius(data.radius, data.uradius);
        ctx.updateTheta(data.theta, data.utheta);
        ctx.updateLife(data.life, data.ulife);
        ctx.updateColor(data.r, data.g, data.b, data.ur, data.ug, data.ub);
        if (data.depth !== undefined) ctx.depth = data.depth;
        if (data.force !== undefined) ctx.force = data.force;
        if (data.limit !== undefined) ctx.limit = data.limit;
        if (data.blender !== undefined) ctx.blender = data.blender;
        if (data.tag !== undefined) ctx.tag = data.tag;
        return ctx;
    }
}

// vim: set ts=4 sts=4 sw=4:
