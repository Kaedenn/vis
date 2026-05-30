import { EmitContext } from './emit_context.js';

export class EmitSequence {
    constructor() {
        this._sequence = new Map(); // Maps integer time/frame to Array of EmitContexts
        this._names = new Map(); // Maps integer time/frame to string name
    }

    add(time, emitContext) {
        if (!this._sequence.has(time)) {
            this._sequence.set(time, []);
        }
        this._sequence.get(time).push(emitContext);
    }

    exists(frame) {
        return this._sequence.has(frame);
    }

    getEmitsAt(frame) {
        return this._sequence.get(frame) || [];
    }

    setEmitsAt(frame, emits) {
        this._sequence.set(frame, emits);
    }

    deleteAt(frame) {
        this._sequence.delete(frame);
        this._names.delete(frame);
    }

    getNameAt(frame) {
        return this._names.get(frame) || "";
    }

    setNameAt(frame, name) {
        if (name) {
            this._names.set(frame, name);
        } else {
            this._names.delete(frame);
        }
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
        this._names.clear();
    }
}

// vim: set ts=4 sts=4 sw=4:
