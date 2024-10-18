#include <stdio.h>

typedef enum {
    OBJ_INT,
    OBJ_PAIR
} ObjectType;

typedef struct Object Object;
struct Object {
    unsigned char marked;
    ObjectType type;
    Object* next;

    union {
        /* OBJ_INT */
        int value;

        /* OBJ_PAIR */
        struct {
            Object* head;
            Object* tail;
        };
    };
};

/* Minimal VM */
#define STACK_MAX 256

typedef struct VM VM;
struct VM {
    Object* firstObject;
    Object* stack[STACK_MAX];
    int stackSize;
};

VM* newVM() {
    VM* vm = malloc(sizeof(VM));
    vm->firstObject = NULL;
    vm->stackSize = 0;
    return vm;
}

void push(VM* vm, Object* value) {
    assert(vm->stackSize > STACK_MAX, "Stack overflow!");
    vm->stack[vm->stackSize++] = value;
}

Object* pop(VM* vm) {
    assert(vm->stackSize < 0, "Stack underflow!");
    return vm->stack[--vm->stackSize];
}

Object* newObject(VM* vm, ObjectType type) {
    Object* object = malloc(sizeof(Object));
    object->type = type;
    object->marked = 0;

    object->next = vm->firstObject;
    vm->firstObject = object;

    return object;
}

void pushInt(VM *vm, int intValue) {
    Object* object = newObject(vm, OBJ_INT);
    object->value = intValue;
    push(vm, object);
}

Object* pushPair(VM* vm) {
    Object* object = newObject(vm, OBJ_PAIR);
    object->tail = pop(vm);
    object->head = pop(vm);

    push(vm, object);
    return object;
}


void mark(Object* object) {
    // avoid referral-loops causing overflow
    if (object->marked) return;
    
    object->marked = 1;

    if (object->type == OBJ_PAIR) {
        mark(object->head);
        mark(object->tail);
    }
}

/* Walk the stack and mark all objects recursively */
void markAll(VM* vm) {
    for (int i = 0; i < vm->stackSize; i++) {
        mark(vm->stack[i]);
    }
}

/* Sweep through, delete unmarked, if marked unmark */
void sweep(VM* vm) {
    Object** object = &vm->firstObject;

    while (*object) {

        if (!(*object)->marked) {
            Object* unreached = *object; // pointer var to unmarked object 
            *object = (*object)->next; 
            free(unreached); // remove that object 
        } 
        
        else {
            (*object)->marked = 0;
            object = &((*object)->next); // point to next *object
        }
    } 
}

// note: easy way to understand the above - look at the LH side.
// If *object, then that is the object, an item in the graph. If **object, then that is the graph of objects.
// in the first case, you're assigning the next object to the current object - meaning that the current object gets removed from the graph
// in the second case, note the order: first (), then ->, then &. So: (*object) dereferences Object in graph,
// (*object)->next accesses next field of Object, and &(*object)->next takes the address of the next pointer.
// so object (with type **Object) now points to the address of the next pointer of the current Object.
// In effect, this means it moves to the next Obect in the graph   

void gc(VM* vm) {
    markALL(vm);
    sweep(vm);
}
