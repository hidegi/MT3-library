#include "types.h"
#include "debug.h"
#include "unit.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <string.h>
#define SP_DEFAULT_LIST_CAP 16

typedef struct
{
	struct SP_action_entry
	{
		SPchar* name;
		SPaction action;
	}* actions;
	SPsize length;
	SPsize capacity;
	SPbool initialized;
	SPint argLength;
	SPchar** argList;
} SP_action_list_head;

static SP_action_list_head SP_TEST_GLOBAL = {0};
int sp_test_status = SP_TEST_PASS;

SPbool spTestInit(SPint argc, SPchar** argv)
{
	if(SP_TEST_GLOBAL.initialized)
	{	
		return SP_TRUE;
	}
	
	SP_TEST_GLOBAL.argLength = argc;
	SP_TEST_GLOBAL.argList = argv;
	
	SP_TEST_GLOBAL.initialized = SP_TRUE;
	SP_TEST_GLOBAL.capacity = SP_DEFAULT_LIST_CAP;
	SP_TEST_GLOBAL.length = 0;
	SP_TEST_GLOBAL.actions = calloc(SP_DEFAULT_LIST_CAP, sizeof(struct SP_action_entry));
	SP_ASSERT(SP_TEST_GLOBAL.actions, "Failed to allocate action list head");
	return SP_TRUE;
}

SPbool spTestAdd(const SPchar* name, SPaction action)
{
	SP_ASSERT(SP_TEST_GLOBAL.initialized, "Cannot add test without initializing SP-test");
	for(SPsize i = 0; i < SP_TEST_GLOBAL.length; i++)
	{
		if(spTestStringEqual(SP_TEST_GLOBAL.actions[i].name, name))
		{
			SP_INFO("Action \"%s\" already added", name);
			return SP_TRUE;
		}
	}
	
	if(SP_TEST_GLOBAL.capacity < SP_TEST_GLOBAL.length + 1)
	{
		SP_TEST_GLOBAL.capacity *= 2;
		struct SP_action_entry* ptr = realloc(SP_TEST_GLOBAL.actions, SP_TEST_GLOBAL.capacity * sizeof(struct SP_action_entry));
		if(!ptr)
		{
			free(SP_TEST_GLOBAL.actions);
			return SP_FALSE;
		}
		SP_TEST_GLOBAL.actions = ptr;
	}
	SPsize length = strlen(name);
	SP_TEST_GLOBAL.actions[SP_TEST_GLOBAL.length].action = action;
	SP_TEST_GLOBAL.actions[SP_TEST_GLOBAL.length].name = calloc(length + 1, sizeof(SPchar));
	memcpy(SP_TEST_GLOBAL.actions[SP_TEST_GLOBAL.length].name, name, length);
	SP_TEST_GLOBAL.actions[SP_TEST_GLOBAL.length].name[length] = 0;
	++SP_TEST_GLOBAL.length;
	return SP_TRUE;
}

SPbool spTestDecimalEqual(SPdouble a, SPdouble b)
{
	return spTestDecimalAlmostEqual(a, b, SP_DEFAULT_PRECISION);
}

SPbool spTestDecimalAlmostEqual(SPdouble a, SPdouble b, SPsize places)
{
	return (fabs(a - b) < pow(10, (SPdouble) places));
}

SPbool spTestStringEqual(const SPchar* a, const SPchar* b)
{
	return strcmp(a, b) == 0;
}

SPbool spTestStringAlmostEqual(const SPchar* a, const SPchar* b, SPsize n)
{
	return strncmp(a, b, n) == 0;
}


SPbool spTestRunAll()
{
	SP_ASSERT(SP_TEST_GLOBAL.actions, "Cannot run tests without initializing SP-test");
	
	SPsize failCount = 0;
	sp_test_status = SP_TEST_PASS;
	printf("------------------------------------------------------------\n");
	printf(" Running unit tests\n");
	printf("------------------------------------------------------------\n");
	for(SPsize i = 0; i < SP_TEST_GLOBAL.length; i++)
	{
		SP_TEST_GLOBAL.actions[i].action();
		SP_PRINT("[SP-test]:", "Running %s ... %s", SP_TEST_GLOBAL.actions[i].name, (sp_test_status == SP_TEST_PASS) ? "PASS" : "FAIL");
		if(sp_test_status == SP_TEST_FAIL)
		{
		   sp_test_status = SP_TEST_PASS;
		   ++failCount;
		}
	}
	
	SP_PRINT("[SP-test]:", "%llu fail(s), %llu test(s)", failCount, SP_TEST_GLOBAL.length);
	return SP_TRUE;
}

void spTestTerminate()
{
	SP_ASSERT(SP_TEST_GLOBAL.actions, "Terminate invoked without initializing SP-test");
	for(SPsize i = 0; i < SP_TEST_GLOBAL.length; i++)
	{
		free(SP_TEST_GLOBAL.actions[i].name);
		SP_TEST_GLOBAL.actions[i].action = NULL;
	}
	free(SP_TEST_GLOBAL.actions);
	SP_TEST_GLOBAL.initialized = SP_FALSE;
}