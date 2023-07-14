#include <stdlib.h>
#include <stdio.h>

#include "binary_tree.h"
#include "deque.h"
#include "vector.h"

KeyValPair *key_val_pair_construct(void *key, void *val) {
    KeyValPair *kvp = calloc(1, sizeof(KeyValPair));
    kvp->value = val;
    kvp->key = key;

    return kvp;
}

void _kvp_info_destroy(KeyValPair *kvp, KeyDestroyFn key_destroy_fn, ValDestroyFn val_destroy_fn) {
    key_destroy_fn(kvp->key);
    val_destroy_fn(kvp->value);
}

void key_val_pair_destroy(KeyValPair *kvp, KeyDestroyFn key_destroy_fn, ValDestroyFn val_destroy_fn) {
    _kvp_info_destroy(kvp, key_destroy_fn, val_destroy_fn);
    free(kvp);
}

Node *node_construct(void *key, void *value, Node *left, Node *right) {
    Node *node = calloc(1, sizeof(Node));
    node->kvp = key_val_pair_construct(key, value);
    node->left = left;
    node->right = right;

    return node;
}

void node_destroy(Node *node) {
    free(node);
}


BinaryTree *binary_tree_construct(CmpFn cmp_fn, KeyDestroyFn key_destroy_fn, ValDestroyFn val_destroy_fn) {
    BinaryTree *bt = calloc(1, sizeof(BinaryTree));
    bt->root = NULL;
    bt->cmp_fn = cmp_fn;
    bt->key_destroy_fn = key_destroy_fn;
    bt->val_destroy_fn = val_destroy_fn;

    return bt;
}

Node *_recursive_bt_add(BinaryTree *bt, Node *node, void *key, void *value) {
    if (!node) {
        return node_construct(key, value, NULL, NULL);
    }

    if (bt->cmp_fn(node->kvp->key, key) < 0) {
        node->right = _recursive_bt_add(bt, node->right, key, value);
    }
    else if (bt->cmp_fn(node->kvp->key, key) > 0) {
        node->left = _recursive_bt_add(bt, node->left, key, value);
    }
    else {
        _kvp_info_destroy(node->kvp, bt->key_destroy_fn, bt->val_destroy_fn);
        node->kvp->value = value;
        node->kvp->key = key;
    }
    return node;
}

void binary_tree_add(BinaryTree *bt, void *key, void *value) {
    bt->root = _recursive_bt_add(bt, bt->root, key, value);
}

int binary_tree_empty(BinaryTree *bt) {
    return bt->root == NULL;
}

void binary_tree_remove(BinaryTree *bt, void *key) {
    Node *curr = bt->root;
    Node *prev_curr = NULL;
    Node *sub = NULL;
    Node *prev_sub = NULL;

    // Finding the node to remove
    while (curr) {
        if (bt->cmp_fn(curr->kvp->key, key) < 0) {
            prev_curr = curr;
            curr = curr->right;
        }
        else if (bt->cmp_fn(curr->kvp->key, key) > 0) {
            prev_curr = curr;
            curr = curr->left;
        }
        else {
            break;
        }
    }
    
    if (!curr) return;

    // Determining what to replace the removed node with
    if (curr->right) {
        prev_sub = curr;
        sub = curr->right;
        while (sub->left) {
            prev_sub = sub;
            sub = sub->left;
        }

        key_val_pair_destroy(curr->kvp, bt->key_destroy_fn, bt->val_destroy_fn);
        curr->kvp = sub->kvp;

        if (prev_sub != curr) prev_sub->left = sub->right;
        else prev_sub->right = sub->right;

        free(sub);
    }
    else if (curr->left) {
        prev_sub = curr;
        sub = curr->left;
        while (sub->right) {
            prev_sub = sub;
            sub = sub->right;
        }

        key_val_pair_destroy(curr->kvp, bt->key_destroy_fn, bt->val_destroy_fn);
        curr->kvp = sub->kvp;

        if (prev_sub != curr) prev_sub->right = sub->left;
        else prev_sub->left = sub->left;

        node_destroy(sub);
    }
    else {
        if (!prev_curr) {
            bt->root = NULL;
        }
        else if (bt->cmp_fn(prev_curr->kvp->key, key) < 0) {
            prev_curr->right = NULL;
        }
        else if (bt->cmp_fn(prev_curr->kvp->key, key) > 0) {
            prev_curr->left = NULL;
        }
        key_val_pair_destroy(curr->kvp, bt->key_destroy_fn, bt->val_destroy_fn);
        node_destroy(curr);
    }
}

KeyValPair *binary_tree_min(BinaryTree *bt) {
    Node *curr = bt->root;

    while (curr->left) { 
        curr = curr->left;
    }

    return curr->kvp;
}

KeyValPair *binary_tree_max(BinaryTree *bt) {
    Node *curr = bt->root;

    while (curr->right) {
        curr = curr->right;
    }

    return curr->kvp;
}

KeyValPair *binary_tree_pop_min(BinaryTree *bt) {
    Node *curr = bt->root;
    Node *prev = NULL;

    while (curr->left) {
        prev = curr;    
        curr = curr->left;
    }

    if (prev) prev->left = curr->right;
    else bt->root = curr->right;

    KeyValPair *kvp = curr->kvp;
    node_destroy(curr);

    return kvp;
}

KeyValPair *binary_tree_pop_max(BinaryTree *bt) {
    {
    Node *curr = bt->root;
    Node *prev = NULL;

    while (curr->right) {
        prev = curr;    
        curr = curr->right;
    }

    if (prev) prev->right = curr->left;
    else bt->root = curr->left;

    KeyValPair *kvp = curr->kvp;
    node_destroy(curr);

    return kvp;
}
}

void *_recursive_bt_get(BinaryTree *bt, Node *node, void *key) {
    if (!node) return NULL;

    if (bt->cmp_fn(node->kvp->key, key) < 0) {
        return _recursive_bt_get(bt, node->right, key);
    }
    else if (bt->cmp_fn(node->kvp->key, key) > 0) {
        return _recursive_bt_get(bt, node->left, key);
    }
    
    return node->kvp->value;
}

void *binary_tree_get(BinaryTree *bt, void *key) {
    return _recursive_bt_get(bt, bt->root, key);
}

void _recursive_bt_node_destroy(BinaryTree *bt, Node *node) {
    if (!node) return;

    //printf("Destroying: %s\n", node->kvp->key);
    _recursive_bt_node_destroy(bt, node->left);
    _recursive_bt_node_destroy(bt, node->right);
    key_val_pair_destroy(node->kvp, bt->key_destroy_fn, bt->val_destroy_fn);
    node_destroy(node);
}

void binary_tree_destroy(BinaryTree *bt) {
    _recursive_bt_node_destroy(bt, bt->root);
    free(bt);
}

// a funcao abaixo pode ser util para debug, mas nao eh obrigatoria.
// void binary_tree_print(BinaryTree *bt);

void _deque_destroy_node(void *n) {
    node_destroy((Node *)n);
}

Vector *binary_tree_inorder_traversal(BinaryTree *bt) {
    Node *curr = bt->root;
    Vector *vec = vector_construct();
    // Using deque as a stack
    Deque *stack = deque_construct(_deque_destroy_node);
    
    while (1) {
        while (curr) {
            deque_push_back(stack, curr);
            curr = curr->left;
        }
        if (deque_size(stack) == 0) break;
        else {
            Node *aux = deque_pop_back(stack);
            vector_push_back(vec, aux->kvp);
            curr = aux->right;
        }
    }
    deque_destroy(stack);
    return vec;
}

Vector *binary_tree_preorder_traversal(BinaryTree *bt) {
    Vector *vec = vector_construct();
    
    // Using deque as a stack
    Deque *stack = deque_construct(_deque_destroy_node);
    if (bt->root) deque_push_back(stack, bt->root);

    while (deque_size(stack) > 0) {
        Node *aux = deque_pop_back(stack);
        vector_push_back(vec, aux->kvp);

        if (aux->right) deque_push_back(stack, aux->right);
        if (aux->left) deque_push_back(stack, aux->left);
    }

    deque_destroy(stack);

    return vec;
}

Vector *binary_tree_postorder_traversal(BinaryTree *bt) {
    Vector *vec = vector_construct();

    // Using deques as stacks
    Deque *q1 = deque_construct(_deque_destroy_node);
    Deque *q2 = deque_construct(_deque_destroy_node);

    if (bt->root) deque_push_back(q1, bt->root);

    while (deque_size(q1) > 0) {
        Node *aux = deque_pop_back(q1);
        
        if (aux->left) deque_push_back(q1, aux->left);
        if (aux->right) deque_push_back(q1, aux->right);

        deque_push_back(q2, aux);
    }

    while (deque_size(q2) > 0) {
        Node *aux = deque_pop_back(q2);
        vector_push_back(vec, aux->kvp);
    }

    deque_destroy(q1);
    deque_destroy(q2);

    return vec;
}

Vector *binary_tree_levelorder_traversal(BinaryTree *bt) {
    Vector *vec = vector_construct();
    // Using deque as a queue
    Deque *queue = deque_construct(_deque_destroy_node);
    
    if (bt->root) deque_push_back(queue, bt->root);

    while (deque_size(queue) > 0) {
        Node *aux = deque_pop_front(queue);

        if (aux) {
            vector_push_back(vec, aux->kvp);
            deque_push_back(queue, aux->left);
            deque_push_back(queue, aux->right);
        }
    }

    deque_destroy(queue);

    return vec;
}

void _recursive_bt_inorder_traversal(Node *node, Vector *vec) {
    if (!node) return;

    _recursive_bt_inorder_traversal(node->left, vec);
    vector_push_back(vec, node->kvp);
    _recursive_bt_inorder_traversal(node->right, vec);
}

Vector *binary_tree_inorder_traversal_recursive(BinaryTree *bt) {
    Vector *vec = vector_construct();

    _recursive_bt_inorder_traversal(bt->root, vec);

    return vec;
}

void _recursive_bt_preorder_traversal(Node *node, Vector *vec) {
    if (!node) return;

    vector_push_back(vec, node->kvp);
    _recursive_bt_preorder_traversal(node->left, vec);
    _recursive_bt_preorder_traversal(node->right, vec);
}

Vector *binary_tree_preorder_traversal_recursive(BinaryTree *bt) {
    Vector *vec = vector_construct();

    _recursive_bt_preorder_traversal(bt->root, vec);

    return vec;
}

void _recursive_bt_postorder_traversal(Node *node, Vector *vec) {
    if (!node) return;

    _recursive_bt_postorder_traversal(node->left, vec);
    _recursive_bt_postorder_traversal(node->right, vec);
    vector_push_back(vec, node->kvp);
}

Vector *binary_tree_postorder_traversal_recursive(BinaryTree *bt) {
    Vector *vec = vector_construct();

    _recursive_bt_postorder_traversal(bt->root, vec);

    return vec;
}
