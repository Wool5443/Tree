# Tree

This repo is an implementation of a binary tree. It can build and draw ones. You can also print or read a tree in prefix form.

## Installation

```bash
git clone https://github.com/Wool5443/Tree
make dirs
make release
```
## Usage

In [TreeSettings.ini](headers/TreeSettings.ini) you can choose data type of the tree, max size, token separator in tree files.

```c++
Tree tree = {};
ErrorCode treeInitError = tree.Init();
```

This initializes our tree with an empty root.
We can set its value and add children.

```c++
tree.root->value = 10;

TreeNodeResult leftChild = TreeNode::New(5, nullptr, nullptr);
RETURN_ERROR(leftChild.error);
TreeNodeResult rightChild = TreeNode::New(15, nullptr, nullptr);
RETURN_ERROR(rightChild.error);

RETURN_ERROR(tree.root->SetLeft(leftChild.value));
RETURN_ERROR(tree.root->SetRight(rightChild.value));

TreeNodeResult rightLeftChild = TreeNode::New(12, nullptr, nullptr);
RETURN_ERROR(rightLeftChild.error);

RETURN_ERROR(tree.root->right->SetLeft(rightLeftChild.value));
```

Now we can draw our tree. And find it in [img](log/img).

```c++
tree->Dump();
```

The tree is constantly checked for mistakes by counting number of nodes. Each node contains the amount of nodes in the subtree. This verification can be disabled in [TreeSettings.ini](headers/TreeSettings.ini).