# Nomad3D
Nomad3D is a software rendered 3D graphics engine. I started this project back in 2006 and originally made it open source on SourceForge.net, [check here](https://sourceforge.net/projects/nomad3d). 

I recently decided to move it to github. The sf.net version has demo running on Win32 (e.g, Windows XP), and the current version on github has been primitively ported to run on Linux. The binary demo app is located at demo/demo, to run the demo, simply open the folder and double clicking the binary, or in Terminal, you will need to change directory to the demo/ folder and run the binary from the folder, i.e., run
```bash
cd demo
./demo
```

This demo has to load 3D model files and texture images from the same folder. Running it from other location will cause file loading error and quit.

![Demo screenshot](/screenshots/Screenshot_from_2025-12-22_04-55-38.png)

## Some quick facts
- The demo on Linux requires SDL, thus install it if you want to compile the code `sudo apt update && sudo apt install libsdl2-dev`
- The demo is running at 16bit color mode, which was popullar at the time Nomad3D was developed, SDL2 still support this mode although it is no longer natively supported by GPU. While Nomad3D is not using GPU by its design.
- To build on Linux, simply run `make` at the project folder
- Code needs some cleanning and refactoring, and I will do it later soon.
