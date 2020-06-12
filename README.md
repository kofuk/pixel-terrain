# Pixel Terrain

![Sample Image](images/sample.png)

This generates map images from your Minecraft save data considering tilt and biomes.

This software contains C++ re-implementation of [matcool/anvil-parser](https://github.com/matcool/anvil-parser)
and [twoolie/NBT](https://github.com/twoolie/NBT).

## Set up the Environment

This project using functionally of C++17; you will need C++ compiler that supports
C++17.

And, this software depends on the following libraries:

- libpng
- zlib

also, this project uses CMake as build system so CMake (>= 3.15) is needed.

## Supported Environments

Though the only supported platform is GNU/Linux for now, I'm preparing to support Windows.

## Installing Artifacts

You can build this project with following shell commands:

```shell
$ mkdir build; cd $_
$ cmake ..
$ make
```

After that, you'll get executable in `build/src/`.

If you want the executable to be in your binary directory,
you can also do `make install`.
I'm sure that CMake installs the executable in `/usr/local/bin` by default, on Unix.

## Legal Information

- Minecraft is a trademark of Mojang Studios.
- Neither this software nor its author is NOT affiliated with Mojang Studios.

## License

This project contains re-implementation of other software;
License for these will be subjected to authors of these.

Other components are written by Koki Fukuda and licensed under MIT License.
