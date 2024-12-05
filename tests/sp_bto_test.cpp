#include "SP/test/unit.h"
#include "SP/sparse/tree.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>

void test_bto_insertion()
{
	sp::BTO tree;
	tree.insertByte("byte", 58);
	tree.insertTree("subTree");
	tree.insertTree("subTree.subTree1");
	tree.insertByte("subTree.subTree1.x", 5);
	tree.insertByte("subTree.subTree1.y", 53);
	tree.insertInt("subTree.z", 1667);
	tree.insertInt("subTree.z", 38924);
	
	tree.debugPrint();
}

int main(int argc, char** argv)
{
	SP_TEST_INIT(argc, argv);
	SP_TEST_ADD(test_bto_insertion);
	spTestRunAll();
	spTestTerminate();
	return 0;
}