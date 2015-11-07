# NavGrid
An Unreal Engine 4 plugin for navigation on a agrid for turn based games

[![Video of two pawns moving on a NavGrtid](http://img.youtube.com/vi/w8L-K8WI2do/0.jpg)](http://www.youtube.com/watch?v=w8L-K8WI2do)

The video above shows two pawns from the "Mixamo Anmination Pack" moving on a NavGrid.

## Quickstart
1. Save/clone into the `Plugins/` directory at the project root
2. Compile
3. Enable via `Edit->Plugins`
4. Place a `NavGrid` and a few `ExampleGridPawn`s in you level
5. Set the PlayerController class to `ANavGridExamplePC`
6. PIE!

Optional: Add `NavGrid` to `PublicDependencyModuleNames` in your `.Build.cs` to access or extend the classes from C++
 
## Notes

### Temporal Antialiasing
The path preview is thin and changes form and posision from one instance to another. If the antialiasing method is set to `TemporalAA`, Unreal will attempt to make this motion appear smooth. This might not look good if there is some distance between the camera and the path.

There are two possible solutions to this: Either ensure that the camera is close when drawing a path or change the antialiasing method in `Project Settings->Rendering`.
