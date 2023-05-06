#include "diff.h"

#ifdef LOG_MODE
    FILE* LogFile = startLog(LogFile);
#endif

int main(void)
{
	FILE* standart = fopen("obj/standart.txt", "r");
	assert(standart);

	Tree_t  stTree = {};
	stTree.root = ReadStandart(standart);

	fclose(standart);
	
	TreeSimplify(&stTree);
	TreeUpdate(&stTree, stTree.root);

	standart = fopen("obj/standart.txt", "w");
	PrintTreeToFile(stTree.root, standart);
	fclose(standart);

	TreeDtor(&stTree);	

#ifdef LOG_MODE
    endLog(LogFile);
#endif
	return 0;
}

TreeNode_t* TrNodeCopy(TreeNode_t* node)
{
	if (node == NULL) return NULL;

	TreeNode_t* newNode = (TreeNode_t*) calloc(1, sizeof(TreeNode_t));
	if (newNode == NULL)
		return NULL;

	newNode->type   = node->type;
	newNode->opVal  = node->opVal;
	newNode->numVal = node->numVal;
	strcpy(newNode->name, node->name);
	
	if (node->left != nullptr)
		newNode->left = TrNodeCopy(node->left);
	
	if (node->right != nullptr)
		newNode->right = TrNodeCopy(node->right);

	return newNode;
}

int TreeSimplify(Tree_t* tree)
{
	if (tree == NULL) return NULL_TREE;
		
	int     isSimpled = 0;
	size_t	oldSize   = tree->size;
	do 
	{
		isSimpled = 0;
		
		TreeSimplifyConst(tree, tree->root);
		if (tree->size < oldSize)
		{
			isSimpled += 1;
			oldSize   = tree->size;
		}

		TreeSimplifyNeutral(tree, tree->root);
		if (tree->size < oldSize)
		{
			isSimpled += 1;
			oldSize    = tree->size;
		}
	} while (isSimpled);
	
	return STATUS_OK;
}

int TreeSimplifyConst(Tree_t* tree, TreeNode_t* node)
{
	if (node == NULL || tree == NULL) return WRONG_DATA;
		
	int oldSize = 0;
	
	if (node->left == NULL && node->right == NULL) return 0;
	
	int isSimpled = 0;

	if (node->left != NULL && node->type == Type_OP && node->opVal != Op_IsBt)
		isSimpled = SimplifyConst(node, tree);
	TreeUpdate(tree, tree->root);

	if (!isSimpled && node && node->left)
		TreeSimplifyConst(tree, node->left);

	if (!isSimpled && node && node->right)
		TreeSimplifyConst(tree, node->right);

	TreeUpdate(tree, tree->root);
	return 0;
}

int SimplifyConst(TreeNode_t* node, Tree_t* tree)
{
	if (node == NULL) return WRONG_DATA;
	
	int isSimpled = 0;
	
	if (node->left->type == Type_NUM) 
	{                                              
		double newNum = 0;

		if (node->right != NULL && node->right->type == Type_NUM)				
		{
			switch (node->opVal)
			{
				case Op_Add: newNum = node->left->numVal + node->right->numVal;		break;
				case Op_Sub: newNum = node->left->numVal - node->right->numVal;		break;
				case Op_Mul: newNum = node->left->numVal * node->right->numVal;		break;
				case Op_Div: newNum = node->left->numVal / node->right->numVal;		break;
				case Op_Pow: newNum = pow(node->left->numVal, node->right->numVal); break;

				default: break;
			}
		}
		else if (node->right == NULL)	
		{
			switch (node->opVal)
			{
				case Op_Sin: newNum = sin(node->left->numVal);	break;
				case Op_Cos: newNum = cos(node->left->numVal);	break;
				case Op_Ln:	 newNum = log(node->left->numVal);	break;

				default: break;
			}

		}
		
		else 
			return isSimpled;

		TreeNode_t* newNode = MakeNode(Type_NUM, Op_Null, newNum, NULL, NULL, NULL);  

		if (node == tree->root)
			tree->root = newNode;
		if (node->parent && node->parent->left && node->parent->left == node)                                               
				node->parent->left = newNode;                                            
		else if (node->parent && node->parent->right && node->parent->right == node)       
				node->parent->right = newNode;                                        
		
		TrNodeRemove(tree, node);
		isSimpled = 1;
	}
	
	return isSimpled;
}

int TreeSimplifyNeutral(Tree_t* tree, TreeNode_t* node)
{
	if (node == NULL || tree == NULL) return WRONG_DATA;
	
	if (node->left == NULL && node->right == NULL) return 0;
	
	int isSimpled = 0;

	if (node->type == Type_OP && node->opVal != Op_IsBt)
		isSimpled = SimplifyNeutral(node, tree);

	TreeUpdate(tree, tree->root);

	if (!isSimpled && node && node->left)
		TreeSimplifyNeutral(tree, node->left);

	if (!isSimpled && node && node->right)
		TreeSimplifyNeutral(tree, node->right);

	TreeUpdate(tree, tree->root);

	return 0;
}

int SimplifyNeutral(TreeNode_t* node, Tree_t* tree)
{
	if (node == NULL) return WRONG_DATA;
	
	int isSimpled = 0;
	
	int isRight = -1;
	int isLeft  = -1;

	if (node->right && node->right->type == Type_NUM)
	{
		if (node->right->numVal == 0)
			isRight = 0;
		else if (node->right ->numVal == 1)
			isRight = 1;
	}
	
	if (node->left && node->left->type == Type_NUM)
	{
		if (node->left->numVal == 0)
			isLeft = 0;
		else if (node->left->numVal == 1)
			isLeft = 1;
	}

	if (isRight == -1 && isLeft == -1)
		return isSimpled;
	
	TreeNode_t* newNode = NULL;
	
	switch (node->opVal)
	{
		case Op_Add:
		{
			if (isLeft == 0)
				newNode = TrNodeCopy(node->right);
			break;
		}
		case Op_Sub:
		{
			if (isRight == 0)
				newNode = TrNodeCopy(node->left);
			break;
		}
		case Op_Mul:
		{
			if (isLeft == 0)
				newNode = TrNodeCopy(node->left);
			else if (isLeft == 1)
				newNode = TrNodeCopy(node->right);
			break;
		}
		case Op_Div:
		{
			if (isLeft == 0 || isRight == 1)
				newNode = TrNodeCopy(node->left);
			break;
		}
		case Op_Pow:
		{
			if (isLeft == 1 || isRight == 1)
				newNode = TrNodeCopy(node->left);
			else if (isRight == 0)
				newNode = MAKE_NUM(1);

			break;
		}
		case Op_Ln:
		{
			if (isRight == 1)
				newNode = MAKE_NUM(0);
			break;
		}

		default: break;
	}
	
	if (newNode == NULL)
		return isSimpled;
	
	if (node == tree->root)
		tree->root = newNode;
	if (node->parent && node->parent->left && node->parent->left == node)                                               
			node->parent->left = newNode;                                            
	else if (node->parent && node->parent->right && node->parent->right == node)       
			node->parent->right = newNode;                                        
					
	TrNodeRemove(tree, node);
		
	isSimpled = 1;
	
	return isSimpled;
}

