<p align="left">
  <img src="assets/mt3.png" width="128" height="128">
</p>

# The MT3-library (SP 1994)
MT3 (short for MoTree) is a lightweight library for serializing and deserializing any plain structure to binary and vice versa (similar to Mojang's NBT-format).
MT3 introduces an open format, the so called Binary Tree Object (BTO), of which's properties are explained overleaf.

## Motivation
The BTO-format is intended to be a successor of Mojang's NBT-format. In a nutshell: Minecraft uses NBT-files to store arbitrary data for its game intrinsics.

However, since it is unclear, whether or not NBT is currently patented by its owners, BTO is published as an open format and innovates the free use without infringement. Read this Wiki-Page for further information about NBT: https://minecraft.fandom.com/de/wiki/NBT

## BTO-Specification
The internal layout of a BTO-file uses an RB-tree (Red-Black-Tree) for fast search, insert and delete operations. For further information about RB-trees, read the following article: https://en.wikipedia.org/wiki/Red–black_tree

Each node in a BTO-file consists of some sort of data (the payload), as well as the length of the name-string, the name-string itself (should not exceed 255 characters), 
the tag and the colour-bit (a boolean value, where 0 is red and 1 is black).

When writing a tree-object to a file, the colour-bit is masked to the 7th bit of the tag. The tag of a node gives information about the type of data stored in a BTO-file. Consequently, when reading a tree, the colour-bit will be extracted and cleared from the tag.

The root-structure of a BTO-file is always a tree and can never be a list, unlike JSON, where the root-structure can either be an array or a structured-object.

To reduce disk storage of a file, BTO files use zlib's fast compression strategy.
Read more about zlib here: https://www.zlib.net

## Data Types
The BTO-format has knowledge of 17 different types of data that can be stored in a tree: numeric values, strings, trees and lists.\
Here is an overview about the individual data types:
| Data Type | Tag | Range | Size in Bytes | Description |
| :---: | :---: | :---: | :---: | :---: |
| Byte | TAG_BYTE | -128 to +127 | 1 | Signed byte |
| Short | TAG_SHORT | -32,768 to +32,767 | 2 | Signed short |
| Int | TAG_INT | -2,147,483,648 to +2,147,483,647| 4 | Signed int |
| Long | TAG_LONG | -9,223,372,036,854,775,808 to +9,223,372,036,854,775,807 | 8 | Signed long |
| Float | TAG_FLOAT | -1.2E-38 to +3.4E+38 | 4 | Floating-point type with single precision |
| Double | TAG_DOUBLE | -2.3E-308 to +1.7E+308 | 8 | Floating-point type with double precision |
| String | TAG_STRING | N/A | varies | A UTF-8 encoded string | 
| Tree | TAG_ROOT | N/A | varies | Structured object that may contain any data type explained in this section |
| Multi-list | TAG_LIST | N/A | varies | Homogenous multi-list of list-objects |
| Tree-list | TAG_ROOT_LIST | N/A | varies | List of tree-objects |
| Byte-list | TAG_BYTE_LIST | N/A | varies | List of bytes |
| Short-list | TAG_SHORT_LIST | N/A | varies | List of shorts |
| Int-list | TAG_INT_LIST | N/A | varies | List of ints |
| Long-list | TAG_LONG_LIST | N/A | varies | List of longs |
| Float-list | TAG_FLOAT_LIST | N/A | varies | List of floats |
| Double-list | TAG_LONG_LIST | N/A | varies | List of doubles |
| String-list | TAG_FLOAT_LIST | N/A | varies | List of strings |

Please note that any lists stored in a BTO-file are implemented as a doubly-linked-list. Therefore, it disables the use of random access for specific elements at runtime.
Because of this, the runtime performance exhibits the properties of a doubly-linked-list.

| Operation | Best Case | Worst Case |
| :---: | :---: | :---: |
| Search | O(1) | O(n) |
| Insert | O(1) | O(n) |
| Delete | O(1) | O(n) |

As mentioned above, any list in a BTO-file is homogenous, meaning that different data types cannot be stored together in one list. This type-safety constraint is checked by the MT3-API.

## Tree-Balancing
The nodes in a tree are lexicographically ordered by their name. Since an RB-tree is self-balancing, any insertions or deletions will automatically rearrange the internal order of the tree. This allows for fast operations with
an average performance of log2(n). The balancing-algorithm implemented by the MT3-API ensures the following properties for an RB-tree:
| Operation | Average Case | Worst Case |
| :---: | :---: | :---: |
| Search | O(log2(n)) | O(log2(n)) |
| Insert | O(log2(n)) | O(log2(n)) |
| Delete | O(log2(n)) | O(log2(n)) |

## How to use
This section demonstrates a short example of how to create and use tree-objects with the MT3-API.
```C
#include <mt3.h>

int main(int argc, char** argv)
{
  MT3_node root = NULL;
  mt3_InsertByte(&root, "byte", 1); // at first use, allocates variable "root"
  mt3_InsertShort(&root, "short", 12);
  mt3_InsertInt(&root, "int", 1234);
  mt3_InsertLong(&root, "long", 12345678);
  mt3_InsertFloat(&root, "float", 3.14159265f);
  mt3_InsertDouble(&root, "double", 13847.8374);
  mt3_InsertString(&root, "string", "Hello World");

  MT3_node subtree = NULL;
  mt3_InsertString(&subtree, "str1", "I"); // at first use, allocates variable "subtree"
  mt3_InsertString(&subtree, "str2", "am");
  mt3_InsertString(&subtree, "str3", "a");
  mt3_InsertString(&subtree, "str4", "subtree");
  mt3_Insert(&root, "subtree", subtree);
  mt3_Delete(&subtree); // deletes the subtree

  MT3_node intList = NULL;
  for(size_t i = 0; i < 10; i++)
    mt3_AppendInt(&intList, i + 1); // this will implicitly create an int-list

  mt3_Insert(&root, "intList", intList);
  mt3_Delete(&intList); // deletes the list

  mt3_Print(root);
  mt3_Delete(&root); // deletes the root-object

  return 0;
}
```

## How to build
To build this project, you will need to install the CMake CLI-or GUI-tool, have a working C/C++ compiler, as well as have GNU-Make available on your local machine (unless you want to use a different generator).

This page provides a guide on how to install CMake: https://cmake.org. \
To install GNU-Make, please consult following page: https://www.gnu.org/software/make/

Once you have CMake and GNU-Make present, use the Command-Prompt or Shell to switch to the directory, where this repository is located on your local machine (you will need to use ```cd```).

On the MT3-library folder, run following command:\
```$ cmake -Bbuild -G"Unix Makefiles"```

This will let CMake configure your project in order to create a Makefile in your build-folder. Once the configuration step has finished, switch to your build folder and run following command:
```$ make```

You can of course use any CMake-Generator of choice, as well as have any other name for your build-directory. When the project has been built, you should be able to run the tests. Run following commands to see the results:
```
$ cd MT3-library/[your-build-directory]/tests
$ ctest --output-on-failure
```

Additionally, this project provides two example programs: 
- A converter from JSON to BTO.
- A pretty-print program that displays the internal layout of a BTO-file.

Be advised that under Windows, you need to copy the ```libmt3.dll``` to your tests-and examples-folder, when specifying\
```-D BUILD_SHARED_LIBS=ON```
