# NavGrid
An Unreal Engine 4 plugin for turn based navigation on a grid.

NavGrid supports grids with arbitrary layout including ladders and multiple levels of tiles. This makes it possible to have tile based movement in complex levels, like for instance multi-floor buildings.

## Quickstart
1. Save/clone into the `Plugins/` directory at the project root
2. Compile. You will need to right click on the `.uproject` in your project and select `Generate Visual Studio project files` so VS is aware of the new source files.
3. Enable via `Edit->Plugins`
4. Create a Collision Channel (Project settings -> Collision) and set its default response to `Ignore`
5. Place a `ANavGrid`, some `ANavTileActor`s and a few `AExampleGridPawn`s in you level
6. Set the PlayerController class to `ANavGridExamplePC`
7. Hit Play!

Optional: Add `NavGrid` to `PublicDependencyModuleNames` in your `.Build.cs` to access or extend the classes from C++

## Class Overview
Examining the headers for `AGridPawn`, `ANavGridExamplePC` and `UGridMovementComponent` are probably good starting points for figuring out how this plugin works. I've tried to make the code as readable as possible.

A few of the central classes are summarized below.

### ANavGrid
Represents the grid. It is responsible for pathfinding.

Useful functions:
* `TilesInRange`: Get tiles within the specified distance. Optionally do collision testing and exclude tiles with obstructions.
* `GetTile`: Get a tile from world-space coordinates.

Useful events:
* `OnTileClicked`
* `OnTileCursorOver`
* `OnTileEndCursorOver`

### UNavTileComponent
A single tile that can be traversed by a `AGridPawn`. It will automaticly detect any neighbouring tiles.

Useful functions:
* `GetNeighbours`: Get all neighbouring tiles.
* `Obstructed`: Given a capsule and a starting position, is there anything obstructing the movement into this tile?
* `GetUnobstructedNeighbours`: Get all neighbouring tiles that a pawn can move into from this tile.
* `Traversable`: Given a movement mode and a max walk angle, is it legal to enter this tile?
* `LegalPositionAtEndOfTurn`:  Given a movement mode and a max walk angle, is it legal to end a turn on this tile?

Useful properties:
* `Cost`: The amount of movement expended when moving into this tile.
* `Mesh`: Static mesh used for rendering this tile.
* `SelectCursor` and `HoverCursor`: Mesh that can be shown just above the tile as part of the UI.
* Various `*Highlight`: Mesh that can be shown just above the tile in order to highlight it in some way.

### UNavLadderComponent
A subclass of `UNavTileComponent` that can be used to represent a ladder. 

### ANavTileActor and ANavLadderActor
Actor containing a single `UNavTileComponent` or `UNavLadderComponent` that can be placed directly into the world. 

### AGridPawn
Base class for pawns that move on a NavGrid.

Useful functions:
* `OnTurnStart` and `OnTurnEnd`: Called when this pawn's turn begins or ends. Override to add your own code.

Useful properties:
* `CapsuleComponent`: The size and relative location of this is used in pathfinding when determening if a tile is obstructed or not.
* `MovementComponent`: A UNavGridMovementComponent (described below) for moving on the NavGrid
* `SelectedHighlight`: Mesh shown when the pawn is selected.
* `SnapToGrid`: Snaps the pawn to grid at game start if set.

### UNavGridMovementComponent
A movement component for moving on a navgrid.

Useful functions:
* `CreatePath`: Find a path to a tile. Returns `false` if the tile is unreachable.
* `FollowPath`: Follow an existing path.
* `PaseMoving`: Temporarily stop moving, call `FollowPath` to resume.
* `ShowPath`: Visualize the path.
* `HidePath`: Stop visualizing the path.
* `GetMovementMode`: Get the current movement mode (none, walking, climbing up or climping down).

Useful properties:
* `MovementRange`: How far (in tile cost) can this pawn move in a single move.
* `Max*Speed`: Max speed for various movement modes.
* `bUseRootMotion`: Use root motion to determine movement speed. If the current animation does not contain root motion `Max*Speed` is used instead.
* `AvailableMovementModes`: Movement modes available for this pawn. Can be useful if you for instance want to disable climbing for some pawns.

Useful events:
* `OnMovementEnd`: Triggered when the pawn has reached its destination.
* `OnMovementModeChanged`: Triggered when the movement mode has changed. E.g. when the pawn has started climbing up a ladder instead of walking.

## Notes

### Temporal Antialiasing
The path preview is thin and changes form and posision from one instance to another. If the antialiasing method is set to `TemporalAA`, Unreal will attempt to make this motion appear smooth. This might not look good if there is some distance between the camera and the path.

There are two possible solutions to this: Either ensure that the camera is close when drawing a path or change the antialiasing method in `Project Settings->Rendering`.

## Changes
**Version 2.0 - 13.08.2016**
* Support Unreal Engine 4.12
* Add ladders
* Add multiple levels of tiles
* Optionaly use root motion for movement speed

**Version 1.0.1 - 29.11.2015**
* Prevent mapcheck warnings about StaticMesh attributes being NULL when building 

**Version 1.0 - 08.11.2015**
* First version
