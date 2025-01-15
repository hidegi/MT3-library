<p align="left">
  <img src="assets/mt3.png" width="128" height="128">
</p>

# The MT3-library (SP 1994)
MT3 (short for MoTree) is a lightweight library for serializing and deserializing any plain structure to\
binary and vice versa (similar to Mojang's NBT-format).

MT3 introduces an open format, the so called Binary Tree Object (BTO), of which's properties\
are explained overleaf.

## Motivation
The BTO-format is intended to be a successor of Mojang's NBT-format. In a nutshell explained:\
Minecraft uses NBT-files to store arbitrary data for its game intrinsics.

However, since it is unclear, whether or not NBT is currently patented by its owners, BTO is 
published as an open format and innovates free use without infringement.
Read this Wiki-page for further information about NBT: https://minecraft.fandom.com/de/wiki/NBT

## BTO-Specification
The internal layout of a BTO-file uses an RB-tree (Red-Black-Tree) for fast search, insert and delete operations.\
For further information about RB-trees, read following article: https://en.wikipedia.org/wiki/Red–black_tree

Each node in a BTO-file consists of some sort of data (the payload), as well as
the length of the name-string, the name-string itself (should not exceed 255 characters), 
the tag and the colour-bit (a boolean value, where 0 is red and 1 is black).

When writing a tree-object to a file, the colour-bit is masked to the 7th bit of the tag.\
The tag of a node gives information about the type of data stored in a BTO-file.\
Consequently, when reading a tree, the colour-bit will be extracted and cleared from the tag.

The root-structure of a BTO-file is always a tree and can never be a list,\
unlike JSON, where the root-structure can either be an array or a structured-object.

The compression strategy used is the Fast-ZLIB-Compression.

## Data Types
The BTO-format has knowledge of 17 different types of data that can be written to disk:\
numeric values, strings, trees and lists.

Here an overview about the individual data types:
| Data Type | Tag | Range | Size in Bytes | Description |
| :---: | :---: | :---: | :---: | :---: |
| Byte | TAG_BYTE | -128 to +127 | 1 | Signed byte |
| Short | TAG_SHORT | -32,768 to +32,767 | 2 | Signed short |
| Int | TAG_INT | -2,147,483,648 to +2,147,483,647| 4 | Signed int |
| Long | TAG_LONG | -9,223,372,036,854,775,808 to +9,223,372,036,854,775,807 | 8 | Signed long |
| Float | TAG_FLOAT | -1.2E-38 to +3.4E+38 | 4 | Floating-point type with single precision |
| Double | TAG_DOUBLE | -2.3E-308 to +1.7E+308 | 8 | Floating-point type with double precision |
| String | TAG_STRING | N/A | varies | A UTF-8 encoded string | 
| Tree | TAG_TREE | N/A | varies | Structured object that may contain any data type explained in this section |
| Multi-list | TAG_LIST | N/A | varies | Homogenous multi-list of list-objects |
| Tree-list | TAG_TREE_LIST | N/A | varies | List of tree-objects |
| Byte-list | TAG_BYTE_LIST | N/A | varies | List of byte |
| Short-list | TAG_SHORT_LIST | N/A | varies | List of short |
| Int-list | TAG_INT_LIST | N/A | varies | List of int |
| Long-list | TAG_LONG_LIST | N/A | varies | List of long |
| Float-list | TAG_FLOAT_LIST | N/A | varies | List of bytes |
| Double-list | TAG_LONG_LIST | N/A | varies | List of bytes |
| String-list | TAG_FLOAT_LIST | N/A | varies | List of bytes |

Further, any lists stored in a BTO-file are implemented as a double-linked-list.\
Therefore, it disables the use of random access of specific elements at runtime.\
Because of this, the runtime performance exhibits the same properties as a double-linked-list.

| Operation | Average Case | Worst Case |
| :---: | :---: | :---: |
| Search | O(1) | O(n) |
| Insert | O(1) | O(n) |
| Delete | O(1) | O(n) |

As mentioned above, any list in a BTO-file is homogenous, meaning that different data types
cannot be stored together in one list. This type-safety constraint is checked by the MT3-API.

## Tree-Balancing
The nodes in a tree are lexicographically ordered by their name. Since an RB-tree is self-balancing, any insertions or\
deletions will automatically rearrange the internal order of the tree. This allows for fast operations with
an average performance of log2(n).

The balancing-algorithm implemented by the MT3 API ensures the following properties for an RB-tree:
| Operation | Average Case | Worst Case |
| :---: | :---: | :---: |
| Search | O(log2(n)) | O(log2(n)) |
| Insert | O(log2(n)) | O(log2(n)) |
| Delete | O(log2(n)) | O(log2(n)) |

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
