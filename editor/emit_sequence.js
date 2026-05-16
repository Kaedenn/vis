import { EmitContext } from './emit_context.js';

export class EmitSequence {
    constructor() {
        this._sequence = new Map(); // Maps integer time/frame to Array of EmitContexts
    }

    add(time, emitContext) {
        if (!this._sequence.has(time)) {
            this._sequence.set(time, []);
        }
        this._sequence.get(time).push(emitContext);
    }

    getEmitsAt(frame) {
        return this._sequence.get(frame) || [];
    }

    setEmitsAt(frame, emits) {
        this._sequence.set(frame, emits);
    }

    // Iterator to loop over timestamps and emits. Keys are sorted chronologically.
    *[Symbol.iterator]() {
        const times = Array.from(this._sequence.keys()).sort((a, b) => a - b);
        for (const t of times) {
            yield [t, this._sequence.get(t)];
        }
    }
    clear() {
        this._sequence.clear();
    }
}

// vim: set ts=4 sts=4 sw=4:
