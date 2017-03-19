#include <stdio.h>
#include <unistd.h>
#include "mymalloc.h"


typedef struct node {
        struct node *prev;
        int size;
        char free;
        struct node *next;
} Node;

Node *first;
Node *last;

void initializeStruct(Node *a, Node *prev, int size, int free, Node *next);


void *my_bestfit_malloc(int size) {
        void *ret;
        char *alignment;
        int allocate;
        int best;
        Node *curr = NULL;
        Node *best = NULL;
        Node *temp = NULL;

        // If allocation size required <= 0, then there's nothing to be allocated.
        if(size == 0 || size < 0) {
                return NULL;
        }

        allocate = size + sizeof(Node); // total allocation size is the size of the Node + the request

        if(first == NULL) {
                last = first = (Node *)sbrk(allocate);

                initializeStruct(first, NULL, allocate, 1, NULL);

                alignment = (char *)first;
                ret = alignment + sizeof(Node);
        } else {
                curr = first;
                best = 0;

                // Start best fit algorithm
                while(curr != NULL) {
                        if(curr->free == 0) {
                                if(curr->size >= allocate && (curr->size < best || best == 0)) {
                                        best = curr->size;
                                        best = curr;
                                        if(curr->size == allocate) {
                                                break;
                                        }
                                }
                        }
                        curr = curr->next;
                }
                // End best fit algorithm

                // No space fitting the allocation request was found
                if(best == NULL) {
                        best = (Node *)sbrk(allocate);
                        initializeStruct(best, NULL, allocate, 1, NULL);

                        alignment = (char *)best;
                        ret = alignment + sizeof(Node);
                        temp = last;
                        last->next = best;
                        last = best;
                        last->prev = temp;
                } else {
                        if(best > allocate) {
                                curr = (Node *)((char*)best + allocate);

                                initializeStruct(curr, best, (best->size - allocate), 0, best->next);

                                temp = curr->next;
                                temp->prev = curr;

                                initializeStruct(best, best->prev, allocate, 1, curr);

                                alignment = (char *)best;
                                ret = alignment + sizeof(Node);
                        } else {
                                best->free = 1;

                                alignment = (char *)best;
                                ret = alignment + sizeof(Node);
                        }
                }
        }
        return ret;
}

void my_free(void *ptr) {
        Node *pointer;
        Node *prev, *next;
        int tempSize;

        if(ptr == NULL) {
                return;
        }

        pointer = (Node *)((char *)ptr - sizeof(Node)); //offset the node
        prev = pointer->prev;
        next = pointer->next;

        if(next == NULL) {
                last = pointer;
        }

        if(next == NULL) {
                if(prev == NULL) {
                        first = last = NULL;
                        sbrk(0 - pointer->size);
                        return;
                } else {
                        if(prev->free == 1) {
                                last = prev;
                                last->next = NULL;
                                sbrk(0 - pointer->size);
                        } else {
                          last = prev->prev;
                          last->next = NULL;
                          sbrk(0 - (pointer->size + prev->size));
                        }
                }
        }
        // Coalesce nodes
        if (!(prev != NULL && next == NULL)) {
                pointer->free = 0;
                if(prev != NULL && prev->free == 0) {
                        pointer->prev->next = pointer->next;
                        pointer->next->prev = pointer->prev;
                        pointer->prev->size += pointer->size;
                        pointer = pointer->prev;
                        next = pointer->next;
                        prev = pointer->prev;
                }
                if(next != NULL && next->free == 0) {
                        tempSize = pointer->next->size;
                        pointer->next = pointer->next->next;
                        pointer->next->prev = pointer;
                        pointer->size += tempSize;
                }
        }
}

void initializeStruct(Node *a, Node *prev, int size, int free, Node *next) {

        a->prev = prev;
        a->size = size;
        a->free = free;
        a->next = next;
}
