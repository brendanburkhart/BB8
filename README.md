# BB8 Simulation

## Building

Project configuration (only needs to be done once, or when project configuration changes):
```
meson setup build
```
To configure a release build, use
```
meson setup release -Dbuildtype=release -Db_lto=true -Doptimization=2
```

Building:
```
meson compile -C build
```

Running:
```
./build/src/bb8_simulation
```
