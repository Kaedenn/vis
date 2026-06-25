import { EmitContext, ForceFunc, LimitFunc, BlendFunc } from './emit_context.js';

export function randrange(min, max) {
    return min + Math.random() * (max - min);
}

export function randint(min, max) {
    return Math.floor(min + Math.random() * (max - min + 1));
}

export class Particle {
    constructor() {
        this.x = 0; this.y = 0;
        this.dx = 0; this.dy = 0;
        this.dz = 0;
        this.radius = 0;
        this.depth = 0;
        this.lifetime = 0;
        this.life = 0;
        this.force = '';
        this.limit = '';
        this.r = 0; this.g = 0; this.b = 0; this.a = 0;
        this.blender = '';
        this.vertices = 0;
        this.angle = 0;
        this.tag = 0;
        this.friction_coeff = 0;
        this.gravity_coeff = 0;

        this.spawn_angle = 0;
        this.speed_rnd = 0;
        this.radius_rnd = 0;
        this.life_rnd = 0;
        this.r_rnd = 0;
        this.g_rnd = 0;
        this.b_rnd = 0;
    }
}

// Tool modes enum
export const ToolMode = Object.freeze({
    SQUARE: 'square',
    CIRCLE: 'circle'
});

export class EmitBuilder {
    constructor(canvas, canvasW, canvasH, fps = 30) {
        this._ctx = new EmitContext();
        this._mode = ToolMode.SQUARE;
        this._dragging = false;
        this._canvas = canvas;
        this._canvasW = canvasW;
        this._canvasH = canvasH;
        this._fps = fps;

        this._clickOffset = { x: 0, y: 0 };
    }

    get ctx() { return this._ctx; }
    set ctx(value) { this._ctx = value; }
    get mode() { return this._mode; }
    set mode(value) { this._mode = value; }
    get canvas() { return this._canvas; }
    set canvas(value) { this._canvas = value; }
    get canvasW() { return this._canvasW; }
    set canvasW(value) { this._canvasW = value; }
    get canvasH() { return this._canvasH; }
    set canvasH(value) { this._canvasH = value; }
    get fps() { return this._fps; }
    set fps(value) { this._fps = value; }

    get dragging() { return this._dragging; }
    set dragging(value) { this._dragging = value; }

    resize(canvasW, canvasH) {
        this._canvasW = canvasW;
        this._canvasH = canvasH;
    }

    globalToLocal(offsetX, offsetY) {
        const scale = Math.min(
            this._canvas.clientWidth / (2 * this._canvasW),
            this._canvas.clientHeight / (2 * this._canvasH));
        const padX = (this._canvas.clientWidth - 2 * this._canvasW * scale) / 2;
        const padY = (this._canvas.clientHeight - 2 * this._canvasH * scale) / 2;
        return {
            x: Math.round((offsetX - padX) / scale - this._canvasW),
            y: Math.round((offsetY - padY) / scale - this._canvasH)
        }
    }

    localToGlobal(mapX, mapY) {
        const scale = Math.min(
            this._canvas.clientWidth / (2 * this._canvasW),
            this._canvas.clientHeight / (2 * this._canvasH));
        const padX = (this._canvas.clientWidth - 2 * this._canvasW * scale) / 2;
        const padY = (this._canvas.clientHeight - 2 * this._canvasH * scale) / 2;
        return {
            x: Math.round((mapX + this._canvasW) * scale + padX),
            y: Math.round((mapY + this._canvasH) * scale + padY)
        }
    }

    mouseMove(e) {
        if (!this._dragging) return;
        const pos = this.globalToLocal(e.offsetX, e.offsetY);
        const clickPos = this._clickOffset;
        switch (this._mode) {
            case ToolMode.SQUARE:
                this._ctx.ux = Math.abs(pos.x - clickPos.x);
                this._ctx.uy = Math.abs(pos.y - clickPos.y);
                break;
            case ToolMode.CIRCLE:
                this._ctx.us = Math.sqrt(Math.pow(pos.x - clickPos.x, 2) + Math.pow(pos.y - clickPos.y, 2));
                break;
        }
    }

    mouseDown(e) {
        this._dragging = true;
        const pos = this.globalToLocal(e.offsetX, e.offsetY);
        this._clickOffset = { x: pos.x, y: pos.y };
    }

    mouseUp(e) {
        this._dragging = false;
    }

    msec2frame(ms) {
        return Math.round(ms * this.fps / 1000);
    }

    getPoints(emit) {
        const stateKey = [
            emit.count, emit.x, emit.y, emit.ux, emit.uy,
            emit.s, emit.us, emit.theta, emit.utheta,
            this.canvasW, this.canvasH, this.fps
        ].join(',');

        let isCached = (this._cachedPoints && this._cachedStateKey === stateKey && this._cachedPoints.length === emit.count);

        if (!isCached) {
            this._cachedPoints = [];
            for (let i = 0; i < emit.count; ++i) {
                this._cachedPoints.push(new Particle());
            }
            this._cachedStateKey = stateKey;
        }

        const points = this._cachedPoints;

        for (let i = 0; i < emit.count; ++i) {
            const p = points[i];

            if (!isCached) {
                p.spawn_angle = randrange(emit.theta - emit.utheta, emit.theta + emit.utheta);
                p.speed_rnd = Math.random();
                p.radius_rnd = Math.random();
                p.life_rnd = Math.random();
                p.r_rnd = Math.random();
                p.g_rnd = Math.random();
                p.b_rnd = Math.random();

                const offset = randrange(emit.s - emit.us, emit.s + emit.us);
                p.x = randrange(emit.x - emit.ux, emit.x + emit.ux) + offset * Math.cos(p.spawn_angle);
                p.y = randrange(emit.y - emit.uy, emit.y + emit.uy) + offset * Math.sin(p.spawn_angle);
            }

            const speed_min = emit.ds - emit.uds;
            const speed_max = emit.ds + emit.uds;
            const speed = speed_min + p.speed_rnd * (speed_max - speed_min);

            p.dx = emit.dx + speed * Math.cos(p.spawn_angle);
            p.dy = emit.dy + speed * Math.sin(p.spawn_angle);
            p.dz = emit.dz;

            const r_min = emit.radius - emit.uradius;
            const r_max = emit.radius + emit.uradius;
            p.radius = Math.round(r_min + p.radius_rnd * (r_max - r_min));
            p.depth = emit.depth;

            const life_min = emit.life - emit.ulife;
            const life_max = emit.life + emit.ulife;
            const lifetime_ms = life_min + p.life_rnd * (life_max - life_min);
            p.lifetime = this.msec2frame(lifetime_ms);
            p.life = p.lifetime;

            const rmin = emit.r - emit.ur; const rmax = emit.r + emit.ur;
            p.r = rmin + p.r_rnd * (rmax - rmin);
            const gmin = emit.g - emit.ug; const gmax = emit.g + emit.ug;
            p.g = gmin + p.g_rnd * (gmax - gmin);
            const bmin = emit.b - emit.ub; const bmax = emit.b + emit.ub;
            p.b = bmin + p.b_rnd * (bmax - bmin);
            p.a = 1.0;

            p.force = emit.force;
            p.limit = emit.limit;
            p.blender = emit.blender;
            p.vertices = emit.vertices;
            p.angle = emit.angle;
            p.tag = emit.tag;
            p.friction_coeff = emit.friction_coeff;
            p.gravity_coeff = emit.gravity_coeff;
        }

        return points;
    }

    particleTick(p) {
        p.x += p.dx;
        p.y += p.dy;
        p.depth += p.dz / 100.0;

        switch (p.force) {
            case ForceFunc.FORCE_FRICTION:
                p.dx *= p.friction_coeff;
                p.dy *= p.friction_coeff;
                break;
            case ForceFunc.FORCE_GRAVITY:
                p.dy += p.gravity_coeff;
                break;
        }

        switch (p.limit) {
            case LimitFunc.LIMIT_BOX:
                if (p.x < -this.canvasW) { p.x = -this.canvasW; p.dx = 0; }
                else if (p.x > this.canvasW) { p.x = this.canvasW; p.dx = 0; }
                if (p.y < -this.canvasH) { p.y = -this.canvasH; p.dy = 0; }
                else if (p.y > this.canvasH) { p.y = this.canvasH; p.dy = 0; }
                break;
            case LimitFunc.LIMIT_SPRINGBOX:
                if (p.x < -this.canvasW) { p.x = -this.canvasW; p.dx = -p.dx; }
                else if (p.x > this.canvasW) { p.x = this.canvasW; p.dx = -p.dx; }
                if (p.y < -this.canvasH) { p.y = -this.canvasH; p.dy = -p.dy; }
                else if (p.y > this.canvasH) { p.y = this.canvasH; p.dy = -p.dy; }
                break;
        }

        p.life -= 1;
    }

    applyBlender(p) {
        if (p.lifetime <= 0) return 1.0;
        const current = p.life;
        const max = p.lifetime;
        if (max === 0) return 1.0;

        switch (p.blender) {
            case BlendFunc.BLEND_LINEAR:
                return current / max;
            case BlendFunc.BLEND_PARABOLIC:
                return -4 * (current/max) * (current/max - 1);
            case BlendFunc.BLEND_QUARTIC:
                return -(current) * (current - 2 * max) / (max * max);
            case BlendFunc.BLEND_SINE:
                return Math.sin(Math.PI * current/max);
            case BlendFunc.BLEND_NEGGAMMA:
                return 1 - Math.exp(-(current * current) / (max * max));
            case BlendFunc.BLEND_EASING:
                return Math.exp(-500 * Math.pow(current/max - 1.0/2.0, 6));
            case BlendFunc.DEFAULT_BLEND:
            default:
                return current / max;
        }
    }

    draw(ctx, age = 0) {
        const emit = this.ctx; /* EmitContext */
        const basePoints = this.getPoints(emit);

        const nframes = Math.round(age * this.msec2frame(emit.life + emit.ulife));

        for (const baseP of basePoints) {
            if (baseP.lifetime < Math.max(1, nframes)) {
                continue;
            }

            const p = new Particle();
            Object.assign(p, baseP);

            for (let i = 0; i < nframes; ++i) {
                this.particleTick(p);
            }

            const alpha = Math.max(0, Math.min(1, this.applyBlender(p)));
            const r = Math.floor(Math.max(0, Math.min(1, p.r)) * 255);
            const g = Math.floor(Math.max(0, Math.min(1, p.g)) * 255);
            const b = Math.floor(Math.max(0, Math.min(1, p.b)) * 255);

            ctx.fillStyle = `rgba(${r}, ${g}, ${b}, ${alpha})`;
            ctx.beginPath();

            const globalPos = this.localToGlobal(p.x, p.y);

            if (p.vertices < 3) {
                ctx.arc(globalPos.x, globalPos.y, p.radius, 0, 2 * Math.PI);
            } else {
                for (let i = 0; i < p.vertices; i++) {
                    const a = p.angle + (i * 2 * Math.PI / p.vertices);
                    const vx = globalPos.x + p.radius * Math.cos(a);
                    const vy = globalPos.y + p.radius * Math.sin(a);
                    if (i === 0) {
                        ctx.moveTo(vx, vy);
                    } else {
                        ctx.lineTo(vx, vy);
                    }
                }
                ctx.closePath();
            }
            ctx.fill();
        }
    }
}
