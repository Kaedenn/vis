# Vis Script Editor

This directory contains a web-based script editor used for designing and generating visualizer scripts. It allows you to build complex emit sequences frame-by-frame and export them for use.

## Files Generated & Loaded

The application supports generating and loading the following file types:

*   **`export.json` (Save/Load) (`Ctrl+s` / `Shift+s` / `Ctrl+o` / `Shift+o`):** 
    Generated when you click **Save**. It stores the complete, serialized state of the editor. This includes the canvas dimensions, active toggles (axes, drag mode, native mode), frame names, and the properties of all emit layers across the entire timeline. This `.json` file can be loaded back into the editor using **Load** to resume a previous session.
*   **`sequence.lua` (Export) (`Ctrl+e` / `Shift+e`):**
    Generated when you click **Export**. This file contains the finalized Lua script representing your timeline sequences. It generates standard Lua code leveraging `Vis`, `visutil`, and `Emit` wrappers that are ready to be used by the main visualizer application.
*   **`.csv` (Timing Import) (`Ctrl+i` / `Shift+i`):**
    The editor can load a custom timing CSV using the **Import Timing** button. The CSV file must contain `t(msec)` (time in milliseconds) and `dur(msec)` (duration/life) columns. It can optionally contain a `lyric` column which will be automatically loaded as the frame name. Loading this file will automatically batch-initialize frame sequences and empty emit layers at the specified timestamps.

    **Important:** This function will *not* overwrite or modify existing frames. If a frame at a specific time already exists (e.g., from a previous import or manual creation), it will be skipped, and only the non-existent frames will be added. This prevents accidental data loss when re-importing or combining datasets.

## Controls Overview

### Top Toolbar
*   **Load (`Ctrl+o` / `Shift+o`) / Save (`Ctrl+s` / `Shift+s`):** Loads a previously saved `.json` state into the editor, or saves the current session to `export.json`.
*   **Export (`Ctrl+e` / `Shift+e`):** Exports the timeline to `sequence.lua`.
*   **Mirror:** Duplicates the currently selected layer and mirrors it horizontally (negates the `x` coordinate and calculates the reflected rotation angle).
*   **Drag Mode:** When toggled on, you can click and drag elements directly on the canvas to visually adjust their `x` and `y` coordinates.
*   **Native Mode:** Toggles how the Lua code is serialized and displayed.
*   **Axes:** Toggles the display of the center X and Y reference axes on the canvas.
*   **Timeline Navigation:** Playhead controls to step frame-by-frame (Next/Previous), jump between populated sequences, manually set the time in milliseconds, and toggle between 30 and 60 FPS.
*   **Canvas Settings:** Inputs to change the rendering bounding box width and height.
*   **Custom Scripting:** Opens a text editor modal to write custom Lua code that will be prepended to the exported `sequence.lua` file.

### Left Toolbar
*   **Add Layer:** Adds a new emit layer to the current frame.
*   **Copy Layer:** Copies all emit layers for the current frame to an application clipboard.
*   **Paste Layer:** Pastes all copied emit layers from the application clipboard to the current frame.
*   **Tools:** Shape and drawing tools (Square, Circle) to modify the selected emit.
*   **Undo / Redo:** Standard history controls to undo and redo core editing actions.
*   **Auto-Populate Particle Lifetime:** Iterates through the timeline and sets the `life` attribute of all particles in each frame to the time difference between that frame and the next frame.
*   **Auto-Populate Particle Tags:** Iterates through the timeline and sets the `tag` attribute of all particles in each frame to that frame's offset time in milliseconds.

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
