// main.js - Editor Logic

// Tool modes enum
const ToolMode = Object.freeze({
    TRAPEZOID: 'trapezoid',
    CIRCLE: 'circle',
    PENCIL: 'pencil'
});

class EmitBuilder {
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
        return {
            x: Math.round((offsetX / this._canvas.clientWidth) * (2 * this._canvasW) - this._canvasW),
            y: Math.round((offsetY / this._canvas.clientHeight) * (2 * this._canvasH) - this._canvasH)
        }
    }

    localToGlobal(mapX, mapY) {
        return {
            x: Math.round((mapX + this._canvasW) * this._canvas.clientWidth / (2 * this._canvasW)),
            y: Math.round((mapY + this._canvasH) * this._canvas.clientHeight / (2 * this._canvasH))
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

    draw(ctx) {
        const emit = this.ctx;
        const p0 = this.localToGlobal(emit.x, emit.y);
        // Determine bounding box: x - ux, x + ux, y - uy, y + uy
        // 
        // Draw centered on p0, width emit.ux, height emit.uy
        // 
        switch (this._mode) {
            case ToolMode.TRAPEZOID:
                // TODO: Draw current context to ctx
                break;
            case ToolMode.CIRCLE:
                // TODO: Draw current context to ctx
                break;
            case ToolMode.PENCIL:
                // TODO: Draw current context to ctx
                break;
        }
    }
}

document.addEventListener('DOMContentLoaded', () => {
    // Editor state
    let currentToolMode = ToolMode.TRAPEZOID;
    let currentTimeMs = 0; // State variable to track the currently edited time
    let fps = 30; // Frames per second
    let showAxes = true; // State for axes visibility
    let mirrorMode = false;
    let scaleMode = false;
    let dragMode = false;
    let nativeMode = false;
    let canvasW = 800; // Initial canvas width
    let canvasH = 600; // Initial canvas height

    // Sequence to store all generated emits mapped by time
    const emitSequence = new EmitSequence();

    // The current array of emit contexts being edited/generated for the active frame
    let currentEmits = [new EmitBuilder(canvasW, canvasH)];
    let currentEmitIndex = 0; // Tracks which emit in currentEmits is actively being edited

    const canvas = document.getElementById('editor-canvas');
    const ctx = canvas.getContext('2d');
    const workspace = document.getElementById('workspace');

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
    });

    inputCanvasH.addEventListener('change', (e) => {
        canvasH = parseInt(e.target.value, 10) || 600;
        console.log(`Canvas height set to ${canvasH}`);
        for (const builder of currentEmits) {
            builder.resize(canvasW, canvasH);
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
            title.appendChild(closeBtn);
            
            const code = document.createElement('code');
            code.className = 'layer-code';
            
            if (idx === currentEmitIndex) {
                code.textContent = layer.ctx.serialize(nativeMode, currentTimeMs);
            }
            
            item.appendChild(title);
            item.appendChild(code);
            
            item.addEventListener('click', () => {
                currentEmitIndex = idx;
                updateLayerDisplay();
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
    }

    // Start rendering
    requestAnimationFrame(draw);

    // Mouse Events
    canvas.addEventListener('mousemove', (e) => {
        if (statusCursor) {
            // Map coordinates: center is 0,0. Left is negative X, Up is negative Y.
            // (-canvasW, -canvasH) is upper-left, (canvasW, canvasH) is lower-right.
            const mapX = Math.round((e.offsetX / canvas.clientWidth) * (2 * canvasW) - canvasW);
            const mapY = Math.round((e.offsetY / canvas.clientHeight) * (2 * canvasH) - canvasH);

            statusCursor.textContent = `${mapX} , ${mapY}`;
        }
    });

    canvas.addEventListener('mouseleave', () => {
        if (statusCursor) {
            statusCursor.textContent = `- , -`;
        }
    });

    canvas.addEventListener('mousedown', (e) => {
        console.log(`Mouse down (Button: ${e.button}) at ${e.offsetX}, ${e.offsetY}`);
    });

    canvas.addEventListener('mouseup', (e) => {
        console.log(`Mouse up (Button: ${e.button}) at ${e.offsetX}, ${e.offsetY}`);
    });

    canvas.addEventListener('click', (e) => {
        console.log(`Click at ${e.offsetX}, ${e.offsetY}`);
    });

    // Keyboard Events
    // Note: the canvas has tabindex="0" so it can receive focus for key events
    canvas.addEventListener('keydown', (e) => {
        console.log(`Key down: ${e.key} (Code: ${e.code})`);

        // Prevent default behavior for specific keys if necessary (like scrolling with arrows)
        // if (['ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight'].includes(e.key)) {
        //     e.preventDefault();
        // }
    });

    canvas.addEventListener('keyup', (e) => {
        console.log(`Key up: ${e.key} (Code: ${e.code})`);
    });

    function onLoad(file) {
        const reader = new FileReader();
        reader.onload = (e) => {
            try {
                const data = JSON.parse(e.target.result);
                console.log('Loaded JSON:', data);
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
                        requestAnimationFrame(draw);
                        break;
                    case 'btn-drag':
                        dragMode = btn.classList.contains('active');
                        requestAnimationFrame(draw);
                        break;
                    case 'btn-scale':
                        scaleMode = btn.classList.contains('active');
                        requestAnimationFrame(draw);
                        break;
                    case 'btn-mirror':
                        mirrorMode = btn.classList.contains('active');
                        requestAnimationFrame(draw);
                        break;
                }
            }

            // Handle specific button actions
            if (btn.id === 'btn-add-layer') {
                currentEmits.push(new EmitBuilder(canvasW, canvasH));
                currentEmitIndex = currentEmits.length - 1;
                console.log(`Added new emit layer. Now editing layer ${currentEmitIndex}`);
                updateLayerDisplay();
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
                //TODO: Stub for save logic
                console.log('Save button clicked');
            } else if (btn.id === 'btn-mirror') {
                //TODO: Stub for mirror logic
                console.log('Mirror button clicked');
            } else if (btn.id === 'btn-scale') {
                //TODO: Stub for scale logic
                console.log('Scale button clicked');
            } else if (btn.id === 'btn-drag') {
                //TODO: Stub for drag logic
                console.log('Drag button clicked');
            } else if (btn.id === 'btn-native') {
                //TODO: Stub for native logic
                console.log('Native button clicked');
            } else if (btn.id === 'btn-export') {
                //TODO: Stub for luagen logic
                console.log('Export button clicked');
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
