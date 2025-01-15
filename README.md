<p align="left">
  <img src="assets/mt3.png" width="128" height="128">
</p>

# The MT3-library (SP 1994)
MT3 (short for MoTree) is a lightweight library for serializing and deserializing any plain structure to\
binary and vice versa (similar to Mojang's NBT-format).

MT3 introduces an open format, the so called Binary Tree Object (BTO), of which's properties\
are explained overleaf.

## BTO-Specification
The internal layout of a BTO-file uses an RB-tree (Red-Black-Tree) for fast search, insert and delete operations.\
For further information about RB-trees, read following article: https://en.wikipedia.org/wiki/Red–black_tree

Each node in a BTO-file consists of some sort of data (the payload), as well as
the length of the name-string, the name-string itself and a tag.

## Data Types
The BTO-format has knowledge of 15 different types of data that can be written to a 
BTO-file: numeric data types, strings, lists and trees. 

All numeric values are always of a signed type.
The BTO-format has knowledge of following data types:
| Data type | Tag | Range | Size in Bytes | Description |
| :---: | :---: | :---: | :---: | :---: |
| byte | TAG_BYTE | -128 to +127 | 1 | An signed byte |
| short | TAG_SHORT | -32,768 to +32,767 | 2 | A signed short |
| int | TAG_INT | -2,147,483,648 to +2,147,483,647| 4 | A signed int |
| long | TAG_LONG | -9,223,372,036,854,775,808 to +9,223,372,036,854,775,807 | 8 | A signed long |
| float | TAG_FLOAT | -1.2E-38 to +3.4E+38 | 4 | Floating-point type with single precision |
| double | TAG_DOUBLE | -2.3E-308 to +1.7E+308 | 8 | Floating-point type with double precision |
| string | TAG_STRING | N/A | varies | A UTF-8 encoded string | 
| tree | TAG_TREE | N/A | varies | A structured object that may contain any data type listed above (including further trees and further lists)|
| multi-list | TAG_LIST | N/A | varies | Homogenous multi-list of list-objects |
| tree-list | TAG_TREE_LIST | N/A | varies | List of tree-objects |
| byte-list | TAG_BYTE_LIST | N/A | varies | List of byte |
| short-list | TAG_SHORT_LIST | N/A | varies | List of short |
| int-list | TAG_INT_LIST | N/A | varies | List of int |
| long-list | TAG_LONG_LIST | N/A | varies | List of long |
| float-list | TAG_FLOAT_LIST | N/A | varies | List of bytes |
| double-list | TAG_LONG_LIST | N/A | varies | List of bytes |
| string-list | TAG_FLOAT_LIST | N/A | varies | List of bytes |


The tag tells exactly what type of data a node in a BTO stores.

To distinguish between these types, each node (like NBT) is labeled with a tag.\
The nodes are lexicographically ordered by their name, however, the length of a name\
should not exceed 255 characters.

Consequently, all data types can be stored as a list, where the first bit in the tag\
tells if some node stores a list of some type (1 for list, 0 for plain type).

The compression strategy used is the Fast-ZLIB-Compression.\
The internal layout of a BTO is a Red-Black-Tree (the color-bit is the 7th bit in the tag).

## How to build
To build this project, you will need to install the CMake CLI-tool,\
since this is a CMake-project, which you can find under https://cmake.org/.

Once you have CMake installed, use the Command-Prompt or Shell to switch to\
the directory, where this repository is located on your local machine.\
(You will need to use ```cd```).

On the MT3-library folder, run following command:\
```$ cmake -Bbuild -G"Unix Makefiles"```

You can of course use any CMake-Generator of choice, as well as have any other name for your build-directory.\
When the project has been built, you should be able to run the tests, located under\
```MT3-library/[your-build-directory]/tests```\
\
Be advised that under Windows, you need to copy the ```libmt3.dll``` to your tests-folder, when specifying\
```-D BUILD_SHARED_LIBS=ON```.
