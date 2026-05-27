import { EmitContext } from './emit_context.js';

export function randrange(min, max) {
    return min + Math.random() * (max - min);
}

// Tool modes enum
export const ToolMode = Object.freeze({
    SQUARE: 'square',
    CIRCLE: 'circle'
});

export class EmitBuilder {
    constructor(canvas, canvasW, canvasH) {
        this._ctx = new EmitContext();
        this._mode = ToolMode.SQUARE;
        this._dragging = false;
        this._canvas = canvas;
        this._canvasW = canvasW;
        this._canvasH = canvasH;

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

    get dragging() { return this._dragging; }
    set dragging(value) { this._dragging = value; }

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

    getPoints(emit) {
        let points = [];
        for (let i = 0; i < emit.count; ++i) {
            const px = randrange(emit.x - emit.ux, emit.x + emit.ux);
            const py = randrange(emit.y - emit.uy, emit.y + emit.uy);
            const ps = randrange(emit.s - emit.us, emit.s + emit.us);
            const ptheta = randrange(emit.theta - emit.utheta, emit.theta + emit.utheta);
            points.push(this.localToGlobal(
                px + Math.cos(ptheta) * ps,
                py + Math.sin(ptheta) * ps));
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
            ctx.arc(p.x, p.y, emit.radius, 0, 2 * Math.PI);
            ctx.fill();
        }
    }
}
