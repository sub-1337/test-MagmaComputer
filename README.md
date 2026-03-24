# test-MagmaComputer

### Build
Build process is quite strightforward - just build with Cmake.

```
cd splitter
mkdir out
cmake -B out -S .
cd out
make
```

### Format of file
Program uses .obj format to read/write model.
Example file:

```
# Created by FreeCAD <https://www.freecad.org>
v 10.000000 10.000000 0.000000
v 0.000000 10.000000 0.000000
v 0.000000 0.000000 0.000000
v 10.000000 0.000000 0.000000
vn 0.000000 0.000000 1.000000
vn 0.000000 0.000000 1.000000
f 1//1 2//1 3//1
f 1//2 3//2 4//2
```

`v` is vertex
`vn` is normal
`f` is face

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

Copy `splitter/tests/files` to your binary folder.