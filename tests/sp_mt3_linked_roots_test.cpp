#include "SP/test/unit.h"
#include "SP/utils/io.h"
#include "SP/sparse/mt3.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>

static void sp_test_roots()
{
	MT3_tree tree1 = NULL;
	MT3_tree tree2 = NULL;
	MT3_tree tree3 = NULL;
	MT3_tree tree4 = NULL;
	MT3_tree tree5 = NULL;
	MT3_tree tree6 = NULL;
	
	
	
}

int main(int argc, char** argv)
{
	SP_TEST_INIT(argc, argv);
	SP_TEST_ADD(sp_test_roots);

	spTestRunAll();
	spTestTerminate();
	return 0;
}