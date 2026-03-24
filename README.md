# test-MagmaComputer

### Format of file
Program uses .obj format to read/write model.
Example file:

```

```

### Using program
Pass program path of the config.
Config example:

```
./cube.obj
0.0 5.0 0.0
10.0 5.0 0.0
0.0 5.0 10.0
```

First line - source file
next 3 vectors representing cutting plane.

Output file names will be **source file** + **left/right**