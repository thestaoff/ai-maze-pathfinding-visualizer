# AI Maze Solver and Pathfinding Visualizer

An interactive C++/SFML application that visualizes how graph-search algorithms explore and solve a 20 x 20 maze.

## Algorithms

- Dijkstra's algorithm
- Breadth-First Search (BFS)
- Depth-First Search (DFS)
- A* search with Manhattan distance

## Features

- Step-by-step, non-blocking visualization
- Clickable wall editing before a search begins
- Distinct colors for walls, visited cells, endpoints, and the final path
- Four included maze layouts
- Keyboard controls for selecting, starting, and resetting an algorithm

## Controls

| Key or action | Function |
|---|---|
| `1` | Select Dijkstra |
| `2` | Select BFS |
| `3` | Select DFS |
| `4` | Select A* |
| `Enter` | Start the selected search |
| `R` | Reset the search state |
| Left click | Toggle a wall while idle |

## Requirements

- A C++17-compatible compiler
- SFML 2.5 or newer (`graphics`, `window`, and `system` components)
- CMake 3.16 or newer, or an equivalent manual build setup

## Build

```bash
cmake -S . -B build
cmake --build build
```

Run the generated `maze_visualizer` executable from the repository root so the default `input4.txt` maze can be loaded.

## Maze format

Each maze file contains a 20 x 20 grid. A `0` is a wall and a `1` is an open cell. Additional examples are available in `mazes/`.

## Color guide

- Black: wall
- White: unexplored cell
- Cyan: visited cell
- Green: start
- Red: destination
- Yellow: final path
