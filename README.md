# NavGrid
An Unreal Engine 4 plugin for turn based navigation on a grid

[![Video of two pawns moving on a NavGrtid](http://img.youtube.com/vi/FoqGXE3b7FE/0.jpg)](http://www.youtube.com/watch?v=FoqGXE3b7FE)

The video above shows two pawns from the "Mixamo Anmination Pack" moving on a NavGrid.

## Quickstart
1. Save/clone into the `Plugins/` directory at the project root
2. Compile
3. Enable via `Edit->Plugins`
4. Place a `NavGrid` and a few `ExampleGridPawn`s in you level
5. Set the PlayerController class to `ANavGridExamplePC`
6. PIE!

Optional: Add `NavGrid` to `PublicDependencyModuleNames` in your `.Build.cs` to access or extend the classes from C++

## Class Overview
Examining the headers for `ANavgrid`, `AGridPawn` and `ANavGridExamplePC` are probably good starting points for figuring out how this plugin works. I've tried to make the code as readable as possible.

A few of the central classes are summarized below.

### ANavGrid
Represents the grid. It owns several `ATiles` and is responsible for pathfinding.

Useful properties:
* `XSize` and `YSize`: The dimensions of the grid in tiles.
* `TileWidth` and `TileHeight`: The size of the individual tiles in UU.
* `Default*`: The default appearance of the tiles.
* `Tiles`: An TArray with pointers to the actual tiles.

Useful functions:
* `Neighbours`: Get adjacent tiles. Optionally do collision testing and exclude tiles with obstructions.
* `TilesInRange`: Get tiles within the specified distance. Optionally do collision testing and exclude tiles with obstructions.
* `GetTile`: Get a tile from either world-space or grid coordinates.

Useful events:
* `OnTileClicked`
* `OnTileCursorOver`
* `OnTileEndCursorOver`

### ATile
A single tile in a navigation grid. Automatically created when placing a NavGrid in the level.

Useful properties:
* `Cost`: The amount of movement expended when moving into this tile.
* `Mesh`: Static mesh used for rendering this tile. Initial value is taken from `DefaultTileMesh` in the NavGrid.
* `SelectCursor` and `HoverCursor`: Mesh that can be shown just above the tile as part of the UI. Initial value is taken from `DefaultSelectCursor` and `DefaultHoverCursor` in the NavGrid.
* Various `*Highlight`: Mesh that can be shown just above the tile in order to highlight it in some way. Initial values are taken from the NavGrid.

### AGridPawn
Base class for pawns that move on a NavGrid.

Useful properties:
* `CapsuleComponent`: The size and relative location of this is used in pathfinding when determening if a tile is obstructed or not. 
* `SelectedHighlight`: Mesh shown when the pawn is selected.
* `SnapToGrid`: Snaps the pawn to grid at game start if set.

Useful functions:
* `OnTurnStart` and `OnTurnEnd`: Called when this pawn's turn begins or ends. Override to add your own code.

## Notes

### Temporal Antialiasing
The path preview is thin and changes form and posision from one instance to another. If the antialiasing method is set to `TemporalAA`, Unreal will attempt to make this motion appear smooth. This might not look good if there is some distance between the camera and the path.

There are two possible solutions to this: Either ensure that the camera is close when drawing a path or change the antialiasing method in `Project Settings->Rendering`.
