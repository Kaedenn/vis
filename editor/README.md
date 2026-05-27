# Vis Script Editor

This directory contains a web-based script editor used for designing and generating visualizer scripts. It allows you to build complex emit sequences frame-by-frame and export them for use.

## Files Generated & Loaded

The application supports generating and loading the following file types:

*   **`export.json` (Save/Load File):** 
    Generated when you click **Save**. It stores the complete, serialized state of the editor. This includes the canvas dimensions, active toggles (axes, drag mode, native mode), frame names, and the properties of all emit layers across the entire timeline. This `.json` file can be loaded back into the application using **Load** to resume a previous session.
*   **`sequence.lua` (Export File):**
    Generated when you click **Export**. This file contains the finalized Lua script representing your timeline sequences. It generates standard Lua code leveraging `Vis`, `visutil`, and `Emit` wrappers that are ready to be used by the main visualizer application.
*   **`.csv` (Timing Import File):**
    The editor can load a custom timing CSV using the **Import Timing** button. The CSV file must contain `t(msec)` (time in milliseconds) and `dur(msec)` (duration/life) columns. It can optionally contain a `lyric` column which will be automatically loaded as the frame name. Loading this file will automatically batch-initialize frame sequences and empty emit layers at the specified timestamps.

## Controls Overview

### Top Toolbar
*   **Load (`Shift+O`) / Save (`Shift+S`):** Loads a previously saved `.json` state or saves the current session to `export.json`.
*   **Export (`Shift+E`):** Exports the timeline to `sequence.lua`.
*   **Mirror:** Duplicates the currently selected layer and mirrors it horizontally (negates the `x` coordinate and calculates the reflected rotation angle).
*   **Drag Mode:** When toggled on, you can click and drag elements directly on the canvas to visually adjust their `x` and `y` coordinates.
*   **Native Mode:** Toggles how the Lua code is serialized and displayed.
*   **Axes:** Toggles the display of the center X and Y reference axes on the canvas.
*   **Timeline Navigation:** Playhead controls to step frame-by-frame (Next/Previous), jump between populated sequences, manually set the time in milliseconds, and toggle between 30 and 60 FPS.
*   **Canvas Settings:** Inputs to change the rendering bounding box width and height.

### Left Toolbar
*   **Add / Copy / Paste Layer:** Controls to add a new emit layer or duplicate existing frame sequences.
*   **Tools:** Shape and drawing tools (Square, Circle) to modify the selected emit.
*   **Undo / Redo:** Standard history controls to undo and redo core editing actions.

### Layer Panel (Right Side)
Displays the properties for the active emit layer. It contains an expandable code view and input fields for all emit properties (`count`, `x`, `y`, `color`, `force`, `limit`, `blender`, etc.). Changes made here are instantly reflected on the canvas.

## Running the Project

To run the project, execute the `runme.py` script located in this directory. 

```bash
python3 runme.py [port] [-b,--bind HOSTNAME] [-d,--directory DIR] [-n,--no-cache] [--help]
```

*   `port`: The port to run the server on. Defaults to `8080`.
*   `-b, --bind HOSTNAME`: The hostname to bind the server to. Defaults to `localhost`.
*   `-d, --directory DIR`: The directory to serve files from. Defaults to `./`.
*   `-n, --no-cache`: Disable caching. Defaults to `False`.
*   `--help`: Show help message. Defaults to `False`.
