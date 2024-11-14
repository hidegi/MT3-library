<p align="left">
  <img src="assets/motex.ico" width="128" height="128">
</p>

# The MT3-library (SP 1994)
MT3 (short for MoTree) is a library for serializing and deserializing any plain structure to\
binary and vice-versa (similar to Mojang's NBT format)..

MT3 introduces an open format, the so called Binary Tree Object (BTO), of which's properties\
are explained overleaf..

## BTO Specification
All numeric values are always of a signed type..\
The BTO-format has knowledge of following data types:
| Data type | Tag | Range | Size in Bytes |
| :---: | :---: | :---: | :---: |
| null | 0 | N/A | 0 |
| root | 1 | N/A | n |
| byte | 2 | -128 to +127 | 1 |
| short | 3 | -32,768 to +32,767 | 2|
| int | 4 | -2,147,483,648 to +2,147,483,647| 4 |
| long | 5 | -9,223,372,036,854,775,808 to +9,223,372,036,854,775,807 | 8 |
| float | 6 | -1.2E-38 to +3.4E+38 | 4 | 
| double | 7 | -2.3E-308 to +1.7E+308 | 8 |
| string | 8 | N/A | n + 1 |

The tag-byte tells exactly what type of data a node in a BTO stores..

Consequently, all data types can be comprised as an array, where the first bit in the tag-byte (1 for array, 0 for plain type)\
tells whether or not some node stores an array of some type..\
An array of root-objects will store a list of trees as a double-linked list of binary-trees, rather than a binary-tree of binary-trees..

To distinguish between these types, each node (like NBT) is labeled with a tag..\
However, the huge difference between NBT and BTO is that BTO uses an SDBM-hash to calculate the weight for a node..\
The input type for a weighted value could be of any type, however MT3 uses a string to calculate a weight..

The default compression strategy is the Fast-ZLIB-Compression, although, GZIP is also possible..\
The internal layout of a BTO is a Red-Black-Tree (the color-bit is the 7th bit in the tag)..

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
