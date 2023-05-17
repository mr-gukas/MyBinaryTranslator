#pragma once
#include "../../tree/include/tree.h"
#include "../../filework/include/filework.h"
#include "../../utils/dsl.h"
#include "../../standart/include/standart.h"
#include <math.h>


enum AkinatorStatus
{
	STATUS_OK         = 0 << 0,
	NULL_EXPRESSION   = 1 << 0,
	NULL_TREE         = 1 << 1,
	BAD_NODE          = 1 << 2,
	WRONG_DATA        = 1 << 3, 
	NULL_NODE         = 1 << 4,
	NON_EXISTABLE_OBJ = 1 << 5,
};

TreeNode_t* TrNodeCopy         (TreeNode_t* node);
int         TreeSimplify       (Tree_t*     tree);
int         TreeSimplifyConst  (Tree_t*     tree, TreeNode_t* node);
int         SimplifyConst      (TreeNode_t* node, Tree_t*     tree);
int         TreeSimplifyNeutral(Tree_t*     tree, TreeNode_t* node);
int         SimplifyNeutral    (TreeNode_t* node, Tree_t*     tree);

