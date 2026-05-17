// main.js - Editor Logic

import { EmitContext, ForceFunc, LimitFunc, BlendFunc } from './emit_context.js';
import { EmitSequence } from './emit_sequence.js';

import { EmitBuilder, ToolMode } from './emit_builder.js';

window.emitSequence = null;
window.currentEmits = null;
window.currentEmitIndex = 0;

const DOWN_CARET = '&#9660;';
const UP_CARET = '&#9650;';

document.addEventListener('DOMContentLoaded', () => {
    // Editor state
    let currentToolMode = ToolMode.TRAPEZOID;
    let currentTimeMs = 0; // State variable to track the currently edited time
    let fps = 30; // Frames per second
    let showAxes = true; // State for axes visibility

    let dragMode = false;
    let nativeMode = false;
    let canvasW = 800; // Initial canvas width
    let canvasH = 600; // Initial canvas height
    let clipboardFrame = null; // Stores serialized context list for copying/pasting frames

    // Sequence to store all generated emits mapped by time
    window.emitSequence = new EmitSequence();

    const canvas = document.getElementById('editor-canvas');
    const ctx = canvas.getContext('2d');
    const workspace = document.getElementById('workspace');

    // The current array of emit contexts being edited/generated for the active frame
    window.currentEmits = [new EmitBuilder(canvas, canvasW, canvasH)];
    window.currentEmitIndex = 0; // Tracks which emit in currentEmits is actively being edited

    // Resize canvas to fit the workspace
    function resizeCanvas() {
        canvas.width = workspace.clientWidth;
        canvas.height = workspace.clientHeight;
    }

    // Initial resize
    window.addEventListener('resize', resizeCanvas);
    resizeCanvas();

    // Setup Time UI controls
    const btnTimePrev = document.getElementById('btn-time-prev');
    const btnTimeNext = document.getElementById('btn-time-next');
    const btnTimeSeekPrev = document.getElementById('btn-time-seek-prev');
    const btnTimeSeekNext = document.getElementById('btn-time-seek-next');
    const inputTime = document.getElementById('input-time');
    const selectFps = document.getElementById('select-fps');
    const labelFrame = document.getElementById('label-frame');
    const inputCanvasW = document.getElementById('input-canvas-w');
    const inputCanvasH = document.getElementById('input-canvas-h');
    const statusCursor = document.getElementById('status-cursor');

    inputCanvasW.addEventListener('change', (e) => {
        canvasW = parseInt(e.target.value, 10) || 800;
        console.log(`Canvas width set to ${canvasW}`);
        for (const builder of currentEmits) {
            builder.resize(canvasW, canvasH);
        }
        requestAnimationFrame(draw);
    });

    inputCanvasH.addEventListener('change', (e) => {
        canvasH = parseInt(e.target.value, 10) || 600;
        console.log(`Canvas height set to ${canvasH}`);
        for (const builder of currentEmits) {
            builder.resize(canvasW, canvasH);
        }
        requestAnimationFrame(draw);
    });

    // Keyboard Events
    // Note: the canvas has tabindex="0" so it can receive focus for key events
    document.addEventListener('keydown', (e) => {
        console.log(`Key down: ${e.key} (Code: ${e.code})`);

        if ((e.ctrlKey || e.shiftKey) && e.key.toLowerCase() === 's') {
            e.preventDefault();
            document.getElementById('btn-save').click();
        } else if ((e.ctrlKey || e.shiftKey) && e.key.toLowerCase() === 'o') {
            e.preventDefault();
            document.getElementById('btn-load').click();
        } else if ((e.ctrlKey || e.shiftKey) && e.key.toLowerCase() === 'e') {
            e.preventDefault();
            document.getElementById('btn-export').click();
        }
    });

    function updateLayerDisplay() {
        const layerList = document.getElementById('layer-list');
        if (!layerList) return;
        layerList.innerHTML = '';

        if (currentEmits.length === 0) {
            const emptyMsg = document.createElement('div');
            emptyMsg.style.padding = '8px 16px';
            emptyMsg.style.color = 'var(--text-color)';
            emptyMsg.style.opacity = '0.5';
            emptyMsg.style.fontSize = '14px';
            emptyMsg.style.fontStyle = 'italic';
            emptyMsg.textContent = 'No layers';
            layerList.appendChild(emptyMsg);
            return;
        }

        // Draw each layer window
        currentEmits.forEach((layer, idx) => {
            const item = document.createElement('div');
            item.className = 'layer-item';
            if (idx === currentEmitIndex) {
                item.classList.add('selected');
            }

            const title = document.createElement('div');
            title.className = 'layer-title';

            const titleText = document.createElement('span');
            titleText.textContent = `${idx === currentEmitIndex ? '>' : ' '} Layer ${idx}`;
            titleText.style.flex = '1';

            const copyCodeBtn = document.createElement('button');
            copyCodeBtn.innerHTML = '&#x2398;'; // ⎘ copy symbol
            copyCodeBtn.title = 'Copy Code to Clipboard';
            copyCodeBtn.style.background = 'none';
            copyCodeBtn.style.border = 'none';
            copyCodeBtn.style.color = 'inherit';
            copyCodeBtn.style.cursor = 'pointer';
            copyCodeBtn.style.padding = '0 4px';
            copyCodeBtn.style.opacity = '0.6';

            copyCodeBtn.addEventListener('mouseenter', () => copyCodeBtn.style.opacity = '1');
            copyCodeBtn.addEventListener('mouseleave', () => copyCodeBtn.style.opacity = '0.6');

            copyCodeBtn.addEventListener('click', (e) => {
                e.stopPropagation();
                const codeString = layer.ctx.serialize(nativeMode, currentTimeMs, true);
                navigator.clipboard.writeText(codeString).then(() => {
                    console.log('Code copied to clipboard.');
                }).catch(err => {
                    console.error('Failed to copy code: ', err);
                });
            });

            const duplicateBtn = document.createElement('button');
            duplicateBtn.innerHTML = '&#128203;'; // 📋 clipboard
            duplicateBtn.title = 'Duplicate Layer';
            duplicateBtn.style.background = 'none';
            duplicateBtn.style.border = 'none';
            duplicateBtn.style.color = 'inherit';
            duplicateBtn.style.cursor = 'pointer';
            duplicateBtn.style.padding = '0 4px';
            duplicateBtn.style.opacity = '0.6';

            duplicateBtn.addEventListener('mouseenter', () => duplicateBtn.style.opacity = '1');
            duplicateBtn.addEventListener('mouseleave', () => duplicateBtn.style.opacity = '0.6');

            duplicateBtn.addEventListener('click', (e) => {
                e.stopPropagation();
                const newBuilder = new EmitBuilder(canvas, canvasW, canvasH);
                newBuilder.ctx = EmitContext.fromJSON(layer.ctx.toJSON());
                currentEmits.splice(idx + 1, 0, newBuilder);
                currentEmitIndex = idx + 1;
                updateLayerDisplay();
                requestAnimationFrame(draw);
            });

            const closeBtn = document.createElement('button');
            closeBtn.innerHTML = '&#x2715;'; // X symbol
            closeBtn.style.background = 'none';
            closeBtn.style.border = 'none';
            closeBtn.style.color = 'inherit';
            closeBtn.style.cursor = 'pointer';
            closeBtn.style.padding = '0 4px';
            closeBtn.style.opacity = '0.6';

            closeBtn.addEventListener('mouseenter', () => closeBtn.style.opacity = '1');
            closeBtn.addEventListener('mouseleave', () => closeBtn.style.opacity = '0.6');

            closeBtn.addEventListener('click', (e) => {
                e.stopPropagation();
                currentEmits.splice(idx, 1);
                if (currentEmits.length === 0) {
                    currentEmitIndex = null;
                } else if (currentEmitIndex === idx) {
                    currentEmitIndex = Math.min(currentEmitIndex, currentEmits.length - 1);
                } else if (currentEmitIndex > idx) {
                    currentEmitIndex--;
                }
                updateLayerDisplay();
            });

            title.appendChild(titleText);
            title.appendChild(copyCodeBtn);
            title.appendChild(duplicateBtn);
            title.appendChild(closeBtn);

            const codeContainer = document.createElement('div');
            codeContainer.className = 'layer-code-container';

            const code = document.createElement('code');
            code.className = 'layer-code';
            code.classList.toggle('collapsed');

            const toggleBtn = document.createElement('button');
            toggleBtn.className = 'layer-code-toggle';
            toggleBtn.innerHTML = DOWN_CARET;

            toggleBtn.addEventListener('click', (e) => {
                e.stopPropagation();
                code.classList.toggle('collapsed');
                toggleBtn.innerHTML = code.classList.contains('collapsed') ? DOWN_CARET : UP_CARET;
                if (idx === currentEmitIndex) {
                    code.textContent = layer.ctx.serialize(nativeMode, currentTimeMs, code.classList.contains('collapsed'));
                }
            });
            code.addEventListener('click', (e) => {
                toggleBtn.click();   
            })

            if (idx === currentEmitIndex) {
                code.textContent = layer.ctx.serialize(nativeMode, currentTimeMs, code.classList.contains('collapsed'));
            }

            codeContainer.appendChild(code);
            codeContainer.appendChild(toggleBtn);

            item.appendChild(title);
            item.appendChild(codeContainer);

            if (idx === currentEmitIndex) {
                const controls = document.createElement('div');
                controls.style.display = 'flex';
                controls.style.flexDirection = 'column';
                controls.style.marginTop = '8px';
                controls.style.gap = '4px';

                const config = [
                    { name: 'count', min: 0, step: 1 },
                    { name: 'x', min: -canvasW/2, max: canvasW/2, step: 1 },
                    { name: 'y', min: -canvasH/2, max: canvasH/2, step: 1 },
                    { name: 'ux', min: 0, max: canvasW, step: 1 },
                    { name: 'uy', min: 0, max: canvasH, step: 1 },
                    { name: 's', min: 0, max: Math.max(canvasW, canvasH), step: 1 },
                    { name: 'us', min: 0, max: Math.max(canvasW, canvasH), step: 1 },
                    { name: 'theta', min: 0, max: 2 * Math.PI, step: 0.01 },
                    { name: 'utheta', min: 0, max: Math.PI, step: 0.01 },
                    { name: 'ds', step: 0.1 },
                    { name: 'uds', step: 0.1 },
                    { name: 'life', min: 0, step: 1 },
                    { name: 'ulife', min: 0, step: 1 },
                    { name: 'rgb', type: 'color' },
                    { name: 'ur', min: 0, max: 1, step: 0.01 },
                    { name: 'ug', min: 0, max: 1, step: 0.01 },
                    { name: 'ub', min: 0, max: 1, step: 0.01 },
                    { name: 'depth', step: 1 },
                    { name: 'force', type: 'select', options: Object.keys(ForceFunc).map(k => ({text: k, value: ForceFunc[k]})) },
                    { name: 'limit', type: 'select', options: Object.keys(LimitFunc).map(k => ({text: k, value: LimitFunc[k]})) },
                    { name: 'blender', type: 'select', options: Object.keys(BlendFunc).map(k => ({text: k, value: BlendFunc[k]})) },
                    { name: 'tag', min: 0, step: 1 }
                ];

                for (const c of config) {
                    const row = document.createElement('div');
                    row.style.display = 'flex';
                    row.style.justifyContent = 'space-between';

                    const lbl = document.createElement('label');
                    lbl.textContent = c.name;
                    lbl.style.fontSize = '12px';
                    lbl.style.minWidth = '60px';

                    let inp;

                    if (c.type === 'color') {
                        inp = document.createElement('input');
                        inp.type = 'color';
                        const toHex = val => Math.round(Math.max(0, Math.min(1, val)) * 255).toString(16).padStart(2, '0');
                        inp.value = `#${toHex(layer.ctx.r)}${toHex(layer.ctx.g)}${toHex(layer.ctx.b)}`;

                        inp.addEventListener('input', (e) => {
                            const hex = e.target.value;
                            layer.ctx.r = parseInt(hex.slice(1,3), 16) / 255.0;
                            layer.ctx.g = parseInt(hex.slice(3,5), 16) / 255.0;
                            layer.ctx.b = parseInt(hex.slice(5,7), 16) / 255.0;
                            code.textContent = layer.ctx.serialize(nativeMode, currentTimeMs, code.classList.contains('collapsed'));
                            requestAnimationFrame(draw);
                        });
                    } else if (c.type === 'select') {
                        inp = document.createElement('select');
                        for (const opt of c.options) {
                            const o = document.createElement('option');
                            o.value = opt.value;
                            o.textContent = opt.text;
                            inp.appendChild(o);
                        }
                        inp.value = layer.ctx[c.name];

                        inp.addEventListener('change', (e) => {
                            layer.ctx[c.name] = e.target.value;
                            code.textContent = layer.ctx.serialize(nativeMode, currentTimeMs, code.classList.contains('collapsed'));
                            requestAnimationFrame(draw);
                        });
                    } else {
                        inp = document.createElement('input');
                        if (c.min !== undefined) inp.min = c.min;
                        if (c.max !== undefined) {
                            if (inp.min === undefined) inp.min = 0;
                            inp.max = c.max;
                            inp.type = 'range';
                        } else {
                            inp.type = 'number';
                        }
                        if (c.step !== undefined) inp.step = c.step;

                        inp.placeholder = c.name;
                        inp.value = layer.ctx[c.name] !== undefined ? layer.ctx[c.name] : 0;

                        inp.addEventListener('input', (e) => {
                            let text = e.target.value;
                            if (c.type === 'range') {
                                text = text.replace(/[^0-9.-]/g, '');
                            } else if (c.name === 'count') {
                                text = text.replace(/[^0-9]/g, '');
                            } else {
                                text = text.replace(/[^0-9.-]/g, '');
                            }
                            e.target.value = text;

                            let val = parseFloat(text);
                            if (!isNaN(val)) {
                                layer.ctx[c.name] = val;
                                code.textContent = layer.ctx.serialize(nativeMode, currentTimeMs, code.classList.contains('collapsed'));
                                requestAnimationFrame(draw);
                            }
                        });
                    }

                    inp.style.width = '80px';
                    inp.style.background = 'var(--bg-panel)';
                    inp.style.color = 'var(--text-color)';
                    inp.style.border = '1px solid var(--border-color)';
                    if (c.type !== 'color') {
                        inp.style.padding = '2px 4px';
                    }

                    inp.addEventListener('click', e => e.stopPropagation());

                    row.appendChild(lbl);
                    row.appendChild(inp);
                    controls.appendChild(row);
                }
                item.appendChild(controls);
            }

            item.addEventListener('click', () => {
                if (currentEmitIndex !== idx) {
                    currentEmitIndex = idx;
                    updateLayerDisplay();
                }
            });

            layerList.appendChild(item);
        });
    }

    function updateTimeUI() {
        inputTime.value = Math.round(currentTimeMs);
        btnTimePrev.disabled = currentTimeMs <= 0;

        // Calculate and update frame number based on current time and FPS
        const frameNum = Math.round(currentTimeMs * fps / 1000);
        labelFrame.textContent = `Frame: ${frameNum}`;
        updateLayerDisplay();
        requestAnimationFrame(draw);
    }

    selectFps.addEventListener('change', (e) => {
        fps = parseInt(e.target.value, 10);
        updateTimeUI();
    });

    function saveCurrentFrame() {
        emitSequence.setEmitsAt(currentTimeMs, [...currentEmits]);
    }

    function loadCurrentFrame() {
        const loaded = emitSequence.getEmitsAt(currentTimeMs);
        if (loaded.length > 0) {
            currentEmits = [...loaded];
            currentEmitIndex = 0;
        } else {
            currentEmits = [];
            currentEmitIndex = null;
        }
    }

    function exportData() {
        saveCurrentFrame();

        const data = {
            canvasW,
            canvasH,
            toggles: { showAxes, nativeMode, dragMode },
            frames: {}
        };

        for (const [time, builders] of emitSequence) {
            const ms = Math.floor(time);
            data.frames[ms] = builders.map(b => b.ctx.toJSON());
        }
        const ms = Math.floor(currentTimeMs);
        if (!data.frames[ms]) {
            console.log(`Missed entry for ${ms}: ${currentTimeMs}`);
            data.frames[ms] = currentEmits.map(b => b.ctx.toJSON());
        }

        const json = JSON.stringify(data, null, 2);
        const blob = new Blob([json], { type: 'application/json' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = 'export.json';
        a.click();
        URL.revokeObjectURL(url);
    }

    btnTimePrev.addEventListener('click', () => {
        saveCurrentFrame();
        const step = 1000 / fps;
        currentTimeMs -= step;
        if (currentTimeMs < 0) currentTimeMs = 0;
        loadCurrentFrame();
        updateTimeUI();
    });

    btnTimeNext.addEventListener('click', () => {
        saveCurrentFrame();
        const step = 1000 / fps;
        currentTimeMs += step;
        loadCurrentFrame();
        updateTimeUI();
    });

    btnTimeSeekPrev.addEventListener('click', () => {
        saveCurrentFrame();
        const times = Array.from(window.emitSequence._sequence.keys())
            .filter(t => window.emitSequence.getEmitsAt(t).length > 0)
            .sort((a, b) => a - b);
        
        if (times.length === 0) return;
        if (times.length === 1 && times[0] === currentTimeMs) return;

        let target = times.slice().reverse().find(t => t < currentTimeMs);
        if (target === undefined) {
            target = times[times.length - 1]; // wrap around to largest
        }

        currentTimeMs = target;
        loadCurrentFrame();
        updateTimeUI();
    });

    btnTimeSeekNext.addEventListener('click', () => {
        saveCurrentFrame();
        const times = Array.from(window.emitSequence._sequence.keys())
            .filter(t => window.emitSequence.getEmitsAt(t).length > 0)
            .sort((a, b) => a - b);
        
        if (times.length === 0) return;
        if (times.length === 1 && times[0] === currentTimeMs) return;

        let target = times.find(t => t > currentTimeMs);
        if (target === undefined) {
            target = times[0]; // wrap around to smallest
        }

        currentTimeMs = target;
        loadCurrentFrame();
        updateTimeUI();
    });

    inputTime.addEventListener('change', (e) => {
        const val = parseFloat(e.target.value);
        if (!isNaN(val) && val >= 0) {
            saveCurrentFrame();
            currentTimeMs = val;
            loadCurrentFrame();
        }
        updateTimeUI();
    });

    // Initial UI update
    updateTimeUI();

    // Rendering loop
    function draw() {
        // Clear the canvas for the new frame
        ctx.clearRect(0, 0, canvas.width, canvas.height);

        // Draw axes if enabled
        if (showAxes) {
            // Center coordinates
            const cx = Math.floor(canvas.width / 2) + 0.5; // +0.5 for crisp 1px lines
            const cy = Math.floor(canvas.height / 2) + 0.5;

            // Draw a black box representing canvasW x canvasH
            ctx.fillStyle = 'black';
            ctx.fillRect(Math.floor(canvas.width / 2) - canvasW / 2, Math.floor(canvas.height / 2) - canvasH / 2, canvasW, canvasH);

            ctx.beginPath();
            ctx.strokeStyle = 'rgba(255, 255, 255, 0.15)'; // Faint, visible white
            ctx.lineWidth = 1;

            // X-axis (horizontal)
            ctx.moveTo(0, cy);
            ctx.lineTo(canvas.width, cy);

            // Y-axis (vertical)
            ctx.moveTo(cx, 0);
            ctx.lineTo(cx, canvas.height);

            ctx.stroke();
        }

        for (const emit of currentEmits) {
            emit.draw(ctx);
        }
    }

    // Start rendering
    requestAnimationFrame(draw);

    let isDraggingCanvas = false;
    let lastDragPos = null;

    // Mouse Events
    canvas.addEventListener('mousedown', (e) => {
        if (dragMode) {
            isDraggingCanvas = true;
            const builder = currentEmits[currentEmitIndex];
            if (builder) {
                lastDragPos = builder.globalToLocal(e.offsetX, e.offsetY);
            }
        } else {
            console.log(`Mouse down (Button: ${e.button}) at ${e.offsetX}, ${e.offsetY}`);
        }
    });

    canvas.addEventListener('mousemove', (e) => {
        const builder = currentEmits[currentEmitIndex] || (currentEmits.length > 0 ? currentEmits[0] : null);
        let mapPos = { x: 0, y: 0 };
        
        if (builder) {
            mapPos = builder.globalToLocal(e.offsetX, e.offsetY);
        } else {
            mapPos.x = Math.round((e.offsetX / canvas.clientWidth) * (2 * canvasW) - canvasW);
            mapPos.y = Math.round((e.offsetY / canvas.clientHeight) * (2 * canvasH) - canvasH);
        }

        if (statusCursor) {
            statusCursor.textContent = `${mapPos.x} , ${mapPos.y}`;
        }

        if (dragMode && isDraggingCanvas && lastDragPos && builder) {
            const dx = mapPos.x - lastDragPos.x;
            const dy = mapPos.y - lastDragPos.y;
            builder.ctx.x += dx;
            builder.ctx.y += dy;
            lastDragPos = mapPos;
            
            requestAnimationFrame(draw);
            
            // Update code display in real time without tearing down DOM
            const codeElements = document.querySelectorAll('.layer-code');
            if (codeElements[currentEmitIndex]) {
                codeElements[currentEmitIndex].textContent = builder.ctx.serialize(nativeMode, currentTimeMs, codeElements[currentEmitIndex].classList.contains('collapsed'));
            }
        }
    });

    canvas.addEventListener('mouseup', (e) => {
        if (dragMode && isDraggingCanvas) {
            isDraggingCanvas = false;
            lastDragPos = null;
            updateLayerDisplay(); // Refresh UI inputs on drop
        } else {
            console.log(`Mouse up (Button: ${e.button}) at ${e.offsetX}, ${e.offsetY}`);
        }
    });

    canvas.addEventListener('mouseleave', () => {
        if (dragMode && isDraggingCanvas) {
            isDraggingCanvas = false;
            lastDragPos = null;
            updateLayerDisplay();
        }
        if (statusCursor) {
            statusCursor.textContent = `- , -`;
        }
    });

    canvas.addEventListener('click', (e) => {
        if (!dragMode) {
            console.log(`Click at ${e.offsetX}, ${e.offsetY}`);
        }
    });

    canvas.addEventListener('keyup', (e) => {
        console.log(`Key up: ${e.key} (Code: ${e.code})`);
    });

    function onLoad(file) {
        const reader = new FileReader();
        reader.onload = (e) => {
            try {
                const data = JSON.parse(e.target.result);

                if (data.canvasW) canvasW = data.canvasW;
                if (data.canvasH) canvasH = data.canvasH;
                inputCanvasW.value = canvasW;
                inputCanvasH.value = canvasH;

                if (data.toggles) {
                    showAxes = !!data.toggles.showAxes;
                    nativeMode = !!data.toggles.nativeMode;
                    dragMode = !!data.toggles.dragMode;

                    document.getElementById('btn-axes').classList.toggle('active', showAxes);
                    document.getElementById('btn-native').classList.toggle('active', nativeMode);
                    document.getElementById('btn-drag').classList.toggle('active', dragMode);
                }

                emitSequence.clear();
                if (data.frames) {
                    for (const [timeStr, contexts] of Object.entries(data.frames)) {
                        const time = parseInt(timeStr, 10);
                        const builders = contexts.map(ctxData => {
                            const builder = new EmitBuilder(canvas, canvasW, canvasH);
                            builder.ctx = EmitContext.fromJSON(ctxData);
                            return builder;
                        });
                        emitSequence.setEmitsAt(time, builders);
                    }
                }

                loadCurrentFrame();
                updateTimeUI();
                requestAnimationFrame(draw);

                console.log('Loaded JSON successfully.');
            } catch (error) {
                console.error('Failed to parse JSON:', error);
                alert('Failed to parse JSON: ' + error.message);
            }
        };
        reader.readAsText(file);
    }

    // Toolbar Button Click Events
    const buttons = document.querySelectorAll('.btn');
    buttons.forEach(btn => {
        btn.addEventListener('click', (e) => {
            const btnName = btn.title || btn.textContent.trim();
            console.log(`Button clicked: ${btnName}`);

            // Handle tool buttons (exclusive toggle)
            if (btn.classList.contains('tool-btn')) {
                // Disable all other buttons first
                document.querySelectorAll('.tool-btn').forEach(b => b.classList.remove('active'));
                btn.classList.add('active');

                if (btn.id === 'tool-trapezoid') {
                    currentToolMode = ToolMode.TRAPEZOID;
                } else if (btn.id === 'tool-circle') {
                    currentToolMode = ToolMode.CIRCLE;
                } else if (btn.id === 'tool-pencil') {
                    currentToolMode = ToolMode.PENCIL;
                }

                console.log(`Tool mode changed to: ${currentToolMode}`);
            }
            // Handle regular toggle buttons
            else if (btn.classList.contains('toggle-btn')) {
                btn.classList.toggle('active');

                switch (btn.id) {
                    case 'btn-axes':
                        showAxes = btn.classList.contains('active');
                        requestAnimationFrame(draw);
                        break;
                    case 'btn-native':
                        nativeMode = btn.classList.contains('active');
                        document.querySelectorAll('.layer-code').forEach((codeEl, i) => {
                            if (currentEmits[i]) {
                                codeEl.textContent = currentEmits[i].ctx.serialize(nativeMode, currentTimeMs, codeEl.classList.contains('collapsed'));
                            }
                        });
                        requestAnimationFrame(draw);
                        break;
                    case 'btn-drag':
                        dragMode = btn.classList.contains('active');
                        requestAnimationFrame(draw);
                        break;

                }
            }

            if (btn.id === 'btn-mirror') {
                // TODO: Copy current emits across y=0
                requestAnimationFrame(draw);
            } else if (btn.id === 'btn-add-layer') {
                currentEmits.push(new EmitBuilder(canvas, canvasW, canvasH));
                currentEmitIndex = currentEmits.length - 1;
                console.log(`Added new emit layer. Now editing layer ${currentEmitIndex}`);
                updateLayerDisplay();
            } else if (btn.id === 'btn-copy-frame') {
                if (currentEmits.length > 0) {
                    clipboardFrame = currentEmits.map(b => b.ctx.toJSON());
                    console.log(`Copied ${clipboardFrame.length} emits to clipboard.`);
                } else {
                    console.log('No emits to copy.');
                    clipboardFrame = null;
                }
            } else if (btn.id === 'btn-paste-frame') {
                if (clipboardFrame) {
                    for (const data of clipboardFrame) {
                        const builder = new EmitBuilder(canvas, canvasW, canvasH);
                        // EmitContext uses fromJSON correctly? Wait, let's verify EmitContext import
                        // We do have `import { EmitContext }` at top of main.js
                        builder.ctx = EmitContext.fromJSON(data);
                        currentEmits.push(builder);
                    }
                    if (currentEmitIndex === null || currentEmits.length > 0) {
                        currentEmitIndex = currentEmits.length - 1;
                    }
                    updateLayerDisplay();
                    requestAnimationFrame(draw);
                    console.log(`Pasted ${clipboardFrame.length} emits from clipboard.`);
                } else {
                    console.log('Clipboard is empty.');
                }
            } else if (btn.id === 'btn-load') {
                const input = document.createElement('input');
                input.type = 'file';
                input.accept = '.json';
                input.multiple = false;
                input.onchange = (e) => {
                    const file = e.target.files[0];
                    if (file) {
                        onLoad(file);
                    }
                };
                input.click();
            } else if (btn.id === 'btn-save') {
                console.log('Save button clicked');
                exportData();
            } else if (btn.id === 'btn-native') {
                console.log('Native mode toggled');
            } else if (btn.id === 'btn-export') {
                console.log('Export button clicked');
                saveCurrentFrame();
                let luaContent = `
-- Generated by Vis editor
Vis = require('Vis')
VisUtil = require('visutil')
Emit = require('emit')
`;
                for (const [time, builders] of window.emitSequence) {
                    for (const builder of builders) {
                        luaContent += builder.ctx.serialize(nativeMode, time) + "\n";
                    }
                }
                const blob = new Blob([luaContent], { type: 'text/plain' });
                const url = URL.createObjectURL(blob);
                const a = document.createElement('a');
                a.href = url;
                a.download = 'sequence.lua';
                a.click();
                URL.revokeObjectURL(url);
            } else if (btn.id === 'btn-undo') {
                //TODO: Stub for undo logic
                console.log('Undo button clicked');
            } else if (btn.id === 'btn-redo') {
                //TODO: Stub for redo logic
                console.log('Redo button clicked');
            }
        });
    });
});

// vim: set ts=4 sts=4 sw=4:
