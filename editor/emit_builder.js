import { EmitContext } from './emit_context.js';

export function randrange(min, max) {
    return min + Math.random() * (max - min);
}

// Tool modes enum
export const ToolMode = Object.freeze({
    TRAPEZOID: 'trapezoid',
    CIRCLE: 'circle',
    PENCIL: 'pencil'
});

export class EmitBuilder {
    constructor(canvas, canvasW, canvasH) {
        this._ctx = new EmitContext();
        this._mode = ToolMode.TRAPEZOID;
        this._dragging = false;
        this._canvas = canvas;
        this._canvasW = canvasW;
        this._canvasH = canvasH;
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

    resize(canvasW, canvasH) {
        this._canvasW = canvasW;
        this._canvasH = canvasH;
    }

    globalToLocal(offsetX, offsetY) {
        const scale = Math.min(this._canvas.clientWidth / (2 * this._canvasW), this._canvas.clientHeight / (2 * this._canvasH));
        const padX = (this._canvas.clientWidth - 2 * this._canvasW * scale) / 2;
        const padY = (this._canvas.clientHeight - 2 * this._canvasH * scale) / 2;
        return {
            x: Math.round((offsetX - padX) / scale - this._canvasW),
            y: Math.round((offsetY - padY) / scale - this._canvasH)
        }
    }

    localToGlobal(mapX, mapY) {
        const scale = Math.min(this._canvas.clientWidth / (2 * this._canvasW), this._canvas.clientHeight / (2 * this._canvasH));
        const padX = (this._canvas.clientWidth - 2 * this._canvasW * scale) / 2;
        const padY = (this._canvas.clientHeight - 2 * this._canvasH * scale) / 2;
        return {
            x: Math.round((mapX + this._canvasW) * scale + padX),
            y: Math.round((mapY + this._canvasH) * scale + padY)
        }
    }

    mouseMove(e) {
        const pos = this.globalToLocal(e.offsetX, e.offsetY);
        if (!this._dragging) return;
        switch (this._mode) {
            case ToolMode.TRAPEZOID:
                this._ctx.ux += (pos.x - this._ctx.x) / 2;
                this._ctx.uy += (pos.y - this._ctx.y) / 2;
                break;
            case ToolMode.CIRCLE:
                this._ctx.us = Math.sqrt(Math.pow(pos.x - this._ctx.x, 2) + Math.pow(pos.y - this._ctx.y, 2));
                break;
            case ToolMode.PENCIL:
                // TODO
                break;
        }
    }

    mouseDown(e) {
        this._dragging = true;
        const pos = this.globalToLocal(e.offsetX, e.offsetY);
        this._ctx.x = pos.x;
        this._ctx.y = pos.y;
    }

    mouseUp(e) {
        this._dragging = false;
    }

    getPoints(emit) {
        let points = [];
        for (let i = 0; i < emit.count; ++i) {
            const px = randrange(emit.x - emit.ux, emit.x + emit.ux);
            const py = randrange(emit.y - emit.uy, emit.y + emit.uy);
            const ps = randrange(emit.s - emit.us, emit.s + emit.us);
            const ptheta = randrange(emit.theta - emit.utheta, emit.theta + emit.utheta);
            const p = this.localToGlobal(px + Math.cos(ptheta) * ps, py + Math.sin(ptheta) * ps);
            const q = this.localToGlobal(px + Math.cos(ptheta) * (ps + emit.ds), py + Math.sin(ptheta) * (ps + emit.ds));
            points.push(p);
            points.push(q);
        }
        return points;
    }

    draw(ctx) {
        const emit = this.ctx; /* EmitContext */
        const points = this.getPoints(emit);
        for (const p of points) {
            const r = randrange(emit.r - emit.ur, emit.r + emit.ur) * 255;
            const g = randrange(emit.g - emit.ug, emit.g + emit.ug) * 255;
            const b = randrange(emit.b - emit.ub, emit.b + emit.ub) * 255;
            ctx.fillStyle = `rgb(${Math.floor(r)}, ${Math.floor(g)}, ${Math.floor(b)})`;
            ctx.beginPath();
            ctx.arc(p.x, p.y, 2, 0, 2 * Math.PI);
            ctx.fill();
        }
    }
}
