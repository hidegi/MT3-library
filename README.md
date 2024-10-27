<p align="left">
  <img src="assets/motex.png" width="128" height="128">
</p>

# the MOT-library (SP 1994)
This is a framework to serialize a JSON-file to a MOT-file.. MOT is an open\
format that encodes an arbitrary structure to binary (similar to Mojang's NBT format)..\
However, the MOT-format uses an actual "binary tree", unlike NBT, alluding to use a binary\
tree, when merely just the representation is binary..

## How to build
To build this project, you will need to install the CMake CLI-tool,\
since this is a CMake-project, which you can find under https://cmake.org/..

Once you have CMake installed, use the Command-Prompt or Shell to switch to\
the directory, where this repository is located on your local machine..\
(You will need to use ```cd```)..

On the Pac-Man folder, run following command:\
```$ cmake -Bbuild -G"Unix Makefiles" -D SP_LOCATION=[wherever you have the SP-library located]```

You can of course use any CMake-Generator of choice, as well as have any other name for your build-directory..\
When the project has been built, you should be able to run the game, located under\
```PlaygroundPM/[your-build-directory]/src/pacman.exe```