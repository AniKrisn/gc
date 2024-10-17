#include <stdio.h>

typedef enum {
    OBJ_INT,
    OBJ_PAIR
} ObjectType;

typedef struct sObject {
    unsigned char marked;
    ObjectType type;
    struct sObject* next;

    union {
        /* OBJ_INT */
        int value;

        /* OBJ_PAIR */
        struct {
            struct sObject* head;
            struct sObject* tail;
        };
    };
} Object;

/* Minimal VM */
#define STACK_MAX 256

typedef struct {
    Object* firstObject;
    Object* stack[STACK_MAX];
    int stackSize;
} VM;

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
            object = &(*object)->next; // point to next *object
        }
    } 
}
