<p align="left">
  <img src="assets/motex.ico" width="128" height="128">
</p>

# The MT3-library (SP 1994)
MT3 (short for MoTree) is an open format that encodes an arbitrary structure into 
binary (similar to Mojang's NBT format)..

## Specification
The MT3-format only knows two fundamental types of data: binary-trees and byte-sequences..\
To distinguish between the type of a byte-sequence, each byte-sequence comes with attributes, namely\
the length and the payload..\
\
Like NBT, each member is stored as a node with a tag; however, the huge difference between the two is\
that MT3 uses an 8-Byte number to store the name (unlike a string), using an SDBM-hash..\
The default compression strategy is the Fast-ZLIB-Compression, although, GZIP is also possible..\
\
The internal layout of MT3 is a Red-Black-Tree (the color-bit is the 7th bit in the tag)..

## How to build
To build this project, you will need to install the CMake CLI-tool,\
since this is a CMake-project, which you can find under https://cmake.org/..

Once you have CMake installed, use the Command-Prompt or Shell to switch to\
the directory, where this repository is located on your local machine..\
(You will need to use ```cd```)..

On the MT3-library folder, run following command:\
```$ cmake -Bbuild -G"Unix Makefiles"```

You can of course use any CMake-Generator of choice, as well as have any other name for your build-directory..\
When the project has been built, you should be able to run the tests, located under\
```MT3-library/[your-build-directory]/tests```\
\
Be advised that under Windows, you need to copy the ```libmot.dll``` to your tests-folder, when specifying\
```-D BUILD_SHARED_LIBS=ON```..
