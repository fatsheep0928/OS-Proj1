#ifndef _RED_BLACK_TREE_H
	#define _RED_BLACK_TREE_H
	#include <stdio.h>
	#include <stdlib.h>
	#ifndef _ULL_
		#define _ULL_
		#define ull unsigned long long int
	#endif

	typedef enum COLOR{ RED=0, BLACK=1 }COLOR;	

	typedef struct item{
		ull ready_time;
		ull time_left;
		ull id;
	}Item;

	typedef struct RBTreeNode{
		COLOR color;
		Item key;
		struct RBTreeNode *left, *right, *parent;
	}Node, *RBTree;

	typedef struct rb_root{
		Node *node;
	}RBRoot;

	#define rb_parent(r)   ((r)->parent)
	#define rb_color(r) ((r)->color)
	#define rb_is_red(r)   ((r)->color==RED)
	#define rb_is_black(r)  ((r)->color==BLACK)
	#define rb_set_black(r)  do { (r)->color = BLACK; } while (0)
	#define rb_set_red(r)  do { (r)->color = RED; } while (0)
	#define rb_set_parent(r,p)  do { (r)->parent = (p); } while (0)
	#define rb_set_color(r,c)  do { (r)->color = (c); } while (0)

	void print_item(Item item){
		printf("(%llu %llu %llu)  ", item.ready_time, item.time_left, item.id);
	}

	int item_compare(Item a, Item b){ //equals to a_is_smaller()
	    if(a.time_left < b.time_left) return 1;
	    else if(a.time_left > b.time_left) return 0;
	    return (a.ready_time <= b.ready_time) ? 1:0;
	}

	RBRoot* create_rbtree(){  //create rbtree and return pointer to root
		RBRoot *root;
		if((root = (RBRoot *)malloc(sizeof(RBRoot))) == NULL){ printf("Failed to malloc RBRoot\n"); exit(1);}
		root->node = NULL;
		return root;
	}

	static void preorder(RBTree tree){
		if(tree != NULL){
			print_item(tree->key);
			preorder(tree->left);
			preorder(tree->right);
		}
	}

	void preorder_rbtree(RBRoot *root){ //print tree in preorder
		if (root) preorder(root->node);
	}

	static void inorder(RBTree tree){
		if(tree != NULL){
			inorder(tree->left);
			print_item(tree->key);
			inorder(tree->right);
		}
	}

	void inorder_rbtree(RBRoot *root){ //print tree in inorder
		if (root) inorder(root->node);
	}

	static void postorder(RBTree tree){
		if(tree != NULL){
			postorder(tree->left);
			postorder(tree->right);
			print_item(tree->key);
		}
	}

	void postorder_rbtree(RBRoot *root){ //print tree in postorder
		if (root) postorder(root->node);
	}

	static Node* minimum(RBTree tree){
		if (tree == NULL) return NULL;
		while(tree->left != NULL) tree = tree->left;
		return tree;
	}

	static void rbtree_left_rotate(RBRoot *root, Node *x){
		Node *y = x->right;                                 /*        left rotate Node* (x)                                */
		x->right = y->left;                                 /*        	        px                              px         */
		if(y->left != NULL) y->left->parent = x;            /*      	       /                               /           */
		y->parent = x->parent;                              /*      	      x                               y            */
                                                            /*        	     /  \      --(left rotate)-->    / \           */
		if(x->parent == NULL) root->node = y;               /*	            lx   y                          x  ry          */
		else{                                               /*	                / \                        / \             */
			if (x->parent->left == x) x->parent->left = y;  /*	               ly  ry                     lx ly            */
			else x->parent->right = y;
		}
		y->left = x;
		x->parent = y;
	}

	static void rbtree_right_rotate(RBRoot *root, Node *y){
		Node *x = y->left;                                    /*       right rotate Node* (y)                              */
		y->left = x->right;                                   /*	            py                               py        */
		if(x->right != NULL) x->right->parent = y;            /*	           /                                /          */
		x->parent = y->parent;                                /*	          y                                x           */
		if(y->parent == NULL) root->node = x;                 /*	         /  \      --(right rotate)-->    /  \         */
		else{                                                 /*	        x   ry                           lx   y        */
			if (y == y->parent->right) y->parent->right = x;  /*	       / \                                   / \       */
			else y->parent->left = x;                         /*	      lx  rx                                rx  ry     */
		}
		x->right = y;
		y->parent = x;
	}

	static void rbtree_insert_fixup(RBRoot *root, Node *node){
		Node *parent, *gparent;
		while ((parent = rb_parent(node)) && rb_is_red(parent)){
			gparent = rb_parent(parent);
			if (parent == gparent->left){
				// Case 1: uncle is red
				{
					Node *uncle = gparent->right;
					if (uncle && rb_is_red(uncle)){
						rb_set_black(uncle);
						rb_set_black(parent);
						rb_set_red(gparent);
						node = gparent;
						continue;
					}
 				}
				// Case 2: uncle is black && I'm right child
				if (parent->right == node){
					Node *tmp;
					rbtree_left_rotate(root, parent);
					tmp = parent;
					parent = node;
					node = tmp;
				}
				// Case 3: uncle is black && I'm left child
				rb_set_black(parent);
				rb_set_red(gparent);
				rbtree_right_rotate(root, gparent);
			}else
				{
				// Case 1: uncle is red
				{
					Node *uncle = gparent->left;
					if (uncle && rb_is_red(uncle)){
						rb_set_black(uncle);
						rb_set_black(parent);
						rb_set_red(gparent);
						node = gparent;
						continue;
					}
				}
				// Case 2: uncle is black && I'm left child
				if (parent->left == node){
					Node *tmp;
					rbtree_right_rotate(root, parent);
					tmp = parent;
					parent = node;
					node = tmp;
				}
				// Case 3: uncle is black && I'm right child
				rb_set_black(parent);
				rb_set_red(gparent);
				rbtree_left_rotate(root, gparent);
			}
		}
		rb_set_black(root->node);
	}

	static void rbtree_insert(RBRoot *root, Node *node){
		Node *y = NULL;
		Node *x = root->node;
		// 1. 将红黑树当作一颗二叉查找树，将节点添加到二叉查找树中。
		while (x != NULL){
			y = x;
			x = item_compare(node->key, x->key)? x->left:x->right;
		}
		rb_parent(node) = y;
		if(y != NULL)
			if(item_compare(node->key, y->key)) y->left = node;
			else y->right = node;
		else root->node = node;

		node->color = RED;
		rbtree_insert_fixup(root, node);
	}

	static Node* create_rbtree_node(Item key, Node *parent, Node *left, Node* right){
		Node* p;
		if ((p = (Node *)malloc(sizeof(Node))) == NULL){ printf("Failed to malloc rbtree node\n"); exit(1);}
		p->key = key;     p->left = left;
		p->right = right; p->parent = parent;
		p->color = BLACK; return p;
	}

	void insert_rbtree(RBRoot *root, Item key){
		Node *node = create_rbtree_node(key, NULL, NULL, NULL);
		rbtree_insert(root, node);
	}

	static void rbtree_delete_fixup(RBRoot *root, Node *node, Node *parent){
		Node *other;
		while ((!node || rb_is_black(node)) && node != root->node){
			if (parent->left == node){
				other = parent->right;
				if (rb_is_red(other)){
					// Case 1: x's sibling w is red
					rb_set_black(other);
					rb_set_red(parent);
					rbtree_left_rotate(root, parent);
					other = parent->right;
				}
				if ((!other->left || rb_is_black(other->left)) &&
					(!other->right || rb_is_black(other->right))){
					// Case 2: x's sibling w is black && w's childs are all black
					rb_set_red(other);
					node = parent;
					parent = rb_parent(node);
				}else{
					if (!other->right || rb_is_black(other->right)){
						// Case 3: x's sibling w is black && w->left is red  w->right is black
						rb_set_black(other->left);
						rb_set_red(other);
						rbtree_right_rotate(root, other);
						other = parent->right;
					}
					// Case 4: x's sibling w is black && w->right is red  w->left is arbitrary
					rb_set_color(other, rb_color(parent));
					rb_set_black(parent);
					rb_set_black(other->right);
					rbtree_left_rotate(root, parent);
					node = root->node;
					break;
				}
			}else{
				other = parent->left;
				if (rb_is_red(other)){
					// Case 1: x's sibling w is red
					rb_set_black(other);
					rb_set_red(parent);
					rbtree_right_rotate(root, parent);
					other = parent->left;
				}
				if ((!other->left || rb_is_black(other->left)) &&
					(!other->right || rb_is_black(other->right))){
					// Case 2: x's sibling w is black && w's childs are both black
					rb_set_red(other);
					node = parent;
					parent = rb_parent(node);
				}else{
					if (!other->left || rb_is_black(other->left)){
						// Case 3: x's sibling w is black && w->left is red  w->right is black
						rb_set_black(other->right);
						rb_set_red(other);
						rbtree_left_rotate(root, other);
						other = parent->left;
					}
					// Case 4: x's sibling w is black && w->right is red  w->left is arbitrary
					rb_set_color(other, rb_color(parent));
					rb_set_black(parent);
					rb_set_black(other->left);
					rbtree_right_rotate(root, parent);
					node = root->node;
					break;
				}
			}
		}
		if (node) rb_set_black(node);
	}

	void rbtree_delete(RBRoot *root, Node *node){
		Node *child, *parent;
		COLOR color;
		if ( (node->left!=NULL) && (node->right!=NULL) ){
			Node *replace = node;
			replace = replace->right;
			while (replace->left != NULL) replace = replace->left;			
			if (rb_parent(node))
				if (rb_parent(node)->left == node) rb_parent(node)->left = replace;
				else rb_parent(node)->right = replace;
			else root->node = replace;

			child = replace->right;
			parent = rb_parent(replace);
			color = rb_color(replace);
			if (parent == node) parent = replace;
			else{
				if (child) rb_set_parent(child, parent);
				parent->left = child;
				replace->right = node->right;
				rb_set_parent(node->right, replace);
			}
			replace->parent = node->parent;
			replace->color = node->color;
			replace->left = node->left;
			node->left->parent = replace;
			if (color == BLACK) rbtree_delete_fixup(root, child, parent);
			free(node);
			return ;
		}
		child = (node->left !=NULL)? node->left:node->right;
		parent = node->parent;
		color = node->color;
		if (child) child->parent = parent;
		if (parent)
			if (parent->left == node) parent->left = child;
        	else parent->right = child;
		else root->node = child;
		if (color == BLACK)
			rbtree_delete_fixup(root, child, parent);
		free(node);
	}

	static void rbtree_destroy(RBTree tree){
		if (tree==NULL) return ;
		if (tree->left != NULL)
        	rbtree_destroy(tree->left);
		if (tree->right != NULL)
			rbtree_destroy(tree->right);
		free(tree);
	}

	void destroy_rbtree(RBRoot *root){
		if (root != NULL)
			rbtree_destroy(root->node);
			free(root);
	}
#endif
