/**
    \file
    \brief Stack with its full implementation
*/

#include <stdlib.h>
#include "general.h"

//[DEFINES]===================================================================

#ifndef STACK_VALUE_TYPE
#error STACK_VALUE_TYPE must be defined to define the stack properly
#endif

#ifndef STACK_VALUE_PRINTF_SPEC
#error STACK_VALUE_PRINTF_SPEC must be defined to dump the stack properly
#endif

//[SECURITY_SETTINGS]==========================================================

#ifndef STACK_SECURITY_LEVEL
#define STACK_SECURITY_LEVEL 10
#endif // STACK_SECURITY_LEVEL

#if STACK_SECURITY_LEVEL > 0
#define SEC_CANARY
#endif

#if STACK_SECURITY_LEVEL > 2
#define SEC_HASH
#endif

#ifndef STACK_DUMP_DEPTH
#define STACK_DUMP_DEPTH 10
#endif

//=============================================================================
//[ONCE_INCLUDING_CONSTANTS]===================================================

#ifndef KCTF_STACK_CONSTANTS
#define KCTF_STACK_CONSTANTS

#define STACK_GENERIC(func) OVERLOAD(Stack_##func, STACK_VALUE_TYPE)
#define STACK_GENERIC_TYPE OVERLOAD(Stack, STACK_VALUE_TYPE)
#define STACK_OK(stack) do {RETURNING_VERIFY_OK(STACK_GENERIC(valid)(stack));} while(0)
#define STACK_HASH(stack) STACK_GENERIC(hash)(stack)

typedef enum stack_code {
    ERR_STACK_NOT_CREATED = -99,
    ERR_STACK_NOT_EXIST,
    ERR_BUFFER_NOT_EXIST,
    ERR_OVERFLOW,
    ERR_REALLOC_FAILED,
} stack_code;

const long long STACK_CANARY = 0xDED0C;

const double STACK_REALLOC_UP_COEF = 1.5;
const double STACK_REALLOC_DOWN_COEF = 2;
#endif // KCTF_STACK_CONSTANTS

//=============================================================================
//<KCTF>[STACK_H]==============================================================

struct STACK_GENERIC(t) {
//[SOME_AWFUL_SECURITY]========================================================
#ifdef SEC_HASH
    long long hash_left;
#else
#ifdef SEC_CANARY
    long long canary_left;
#endif
#endif
//=============================================================================
//[STACK]======================================================================
    size_t capacity;
    size_t size;
    STACK_VALUE_TYPE *buffer;
//=============================================================================
//[SOME_AWFUL_SECURITY]========================================================
#ifdef SEC_HASH
    long long hash_right;
#else
#ifdef SEC_CANARY
    long long canary_right;
#endif
#endif
//=============================================================================
};

//[DOCUMENTARY]================================================================

typedef struct STACK_GENERIC(t) STACK_GENERIC_TYPE;

long long STACK_GENERIC(hash)(const STACK_GENERIC_TYPE *cake); ///< Calculates hash(cake) the right way

void      STACK_GENERIC(recalculate_hash)   (STACK_GENERIC_TYPE *cake); ///< Calls hash(cake) the right way and sets hash_lr
int       STACK_GENERIC(recalcute_security) (STACK_GENERIC_TYPE *cake); ///< Calls recalculate_security(cake) and sets security vars

int       STACK_GENERIC(construct)      (STACK_GENERIC_TYPE *cake); ///< Constructs stack the right way. Must be called before any operations with stack
int       STACK_GENERIC(destruct)       (STACK_GENERIC_TYPE *cake); ///< Destructs stack the right way. Must be called after all operations with stack

int       STACK_GENERIC(dump)     (const STACK_GENERIC_TYPE *cake); ///< Dumps stack in a pretty way
int       STACK_GENERIC(valid)    (const STACK_GENERIC_TYPE *cake); ///< Checks, if stack is valid, returns 0 if is, err_code otherwise

size_t    STACK_GENERIC(size)     (const STACK_GENERIC_TYPE *cake); ///< Returs current number of elements in stack
size_t    STACK_GENERIC(capacity) (const STACK_GENERIC_TYPE *cake); ///< Return current max_number of elements in stack

STACK_VALUE_TYPE STACK_GENERIC(top)      (const STACK_GENERIC_TYPE *cake);

char      STACK_GENERIC(is_empty) (const STACK_GENERIC_TYPE *cake); ///< Returns 1 if stack is empty, 0 otherwise
char      STACK_GENERIC(is_full)  (const STACK_GENERIC_TYPE *cake); ///< Returns 1 if cake->size == cake->capacity, 0 otherwise

int STACK_GENERIC(push) (STACK_GENERIC_TYPE *cake, const STACK_VALUE_TYPE val); ///< Pushes val on the top of stack
int STACK_GENERIC(pop)  (STACK_GENERIC_TYPE *cake); ///< Pop top val from stack. DOES NOT return value
int STACK_GENERIC(clear)(STACK_GENERIC_TYPE *cake); ///< Pops all elements from stack

int STACK_GENERIC(resize)(STACK_GENERIC_TYPE *cake, const size_t new_capacity); ///< Tries to make cake->capacity be new_capacity
int STACK_GENERIC(resize_up)(STACK_GENERIC_TYPE *cake);
int STACK_GENERIC(resize_down)(STACK_GENERIC_TYPE *cake);

//=============================================================================
//=============================================================================
//<KCTF>[STACK_C]==============================================================

long long STACK_GENERIC(hash)(const STACK_GENERIC_TYPE *cake) {
    RETURNING_VERIFY(cake != NULL);
    RETURNING_VERIFY(cake->buffer != NULL);
    return + do_hash((const char*)cake + sizeof(long long), sizeof(STACK_GENERIC_TYPE) - 2 * sizeof(long long))
           + do_hash(cake->buffer, (cake->capacity - 1) * sizeof(STACK_VALUE_TYPE));
}

#ifdef SEC_HASH
void STACK_GENERIC(recalculate_hash)(STACK_GENERIC_TYPE *cake) {
    long long int new_hash = STACK_HASH(cake);
    cake->hash_left = new_hash;
    cake->hash_right = new_hash;
}
#endif

int STACK_GENERIC(valid)(const STACK_GENERIC_TYPE *cake) {
    if (!cake) {
        RETURN_ERROR_VERIFY(ERR_STACK_NOT_EXIST);
    }

    if (!cake->buffer) {
        RETURN_ERROR_VERIFY(ERR_BUFFER_NOT_EXIST);
    }

    if (cake->size > cake->capacity) {
        RETURN_ERROR_VERIFY(ERR_OVERFLOW);
    }

#ifdef SEC_HASH
    if (cake->hash_left != STACK_HASH(cake) || cake->hash_left != cake->hash_right) {
        RETURN_ERROR_VERIFY(ERROR_BAD_HASH);
    }
#else
#ifdef SEC_CANARY
    if (cake->canary_left != STACK_CANARY || cake->canary_left != cake->canary_right) {
        RETURN_ERROR_VERIFY(ERROR_BAD_CANARY);
    }
#endif
#endif

    return OK;
}

int STACK_GENERIC(recalcute_security)(STACK_GENERIC_TYPE *cake) {
    RETURNING_VERIFY(cake != NULL);

#ifdef SEC_HASH
    STACK_GENERIC(recalculate_hash)(cake);
#else
#ifdef SEC_CANARY
    cake->canary_left = STACK_CANARY;
    cake->canary_right = STACK_CANARY;
#endif
#endif

    return OK;
}

int STACK_GENERIC(construct)(STACK_GENERIC_TYPE *cake) {
    RETURNING_VERIFY(cake != NULL);

    const size_t capacity = 32;
    cake->buffer = (STACK_VALUE_TYPE*) calloc(capacity, sizeof(STACK_VALUE_TYPE));
    RETURNING_VERIFY(cake->buffer != NULL);

    cake->capacity = capacity;
    cake->size = 0;

    RETURNING_VERIFY_OK(STACK_GENERIC(recalcute_security)(cake));
    STACK_OK(cake);
    return OK;
}

STACK_GENERIC_TYPE *STACK_GENERIC(new)() {
    STACK_GENERIC_TYPE *stack = calloc(sizeof(STACK_GENERIC_TYPE), 1);
    VERIFY_t(STACK_GENERIC(construct)(stack) == 0, STACK_GENERIC_TYPE*);
    return stack;
}

int STACK_GENERIC(destruct)(STACK_GENERIC_TYPE *cake) {
    STACK_OK(cake);

    free(cake->buffer);
    cake->capacity = SIZE_T_P;
    cake->size = SIZE_T_P;
    cake->buffer = NULL;

    return OK;
}

int STACK_GENERIC(delete)(STACK_GENERIC_TYPE *cake) {
    VERIFY_OK(STACK_GENERIC(destruct)(cake));
    free(cake);
    return 0;
}

size_t STACK_GENERIC(size)(const STACK_GENERIC_TYPE *cake) {
    // RETURNING_VERIFY_OK(STACK_GENERIC(valid)(cake));
    return cake->size;
}

size_t STACK_GENERIC(capacity)(const STACK_GENERIC_TYPE *cake) {
    // RETURNING_VERIFY_OK(STACK_GENERIC(valid)(cake));
    return cake->capacity;
}

int STACK_GENERIC(dump)(const STACK_GENERIC_TYPE *cake) {
    if (cake == NULL) {
        printf("[DMP]<stack>: [ptr](0) [valid](FALSE) /try to guess why/\n");
    }
    const int validity = STACK_GENERIC(valid)(cake);

    if (!validity) {
        printf("[DMP]<stack>: [ptr](%p) [valid](true)\n", (const void*) cake);
    } else {
        printf("[DMP]<stack>: [ptr](%p) [valid](FALSE) ^- check last error -^\n", (const void*) cake);
    }

#ifdef SEC_HASH
    printf("[   ]<     >: [hash_l](%lld)\n", cake->hash_left);
    printf("[   ]<     >: [hash_r](%lld)\n", cake->hash_right);
#else
#ifdef SEC_CANARY
    printf("[   ]<     >: [canary_l](%X)\n", cake->canary_left);
    printf("[   ]<     >: [canary_r](%X)\n", cake->canary_right);
#endif
#endif

    printf("[   ]<     >: [size](%zu)\n", cake->size);
    printf("[   ]<     >: [capacity](%zu)\n", cake->capacity);
    printf("[   ]<.buf.>: [buffer](%p)\n", (void*)(cake->buffer));

    const size_t print_depth = (size_t) min((long long) cake->size, STACK_DUMP_DEPTH);
    for (size_t i = 0; i < print_depth; ++i) {
        printf("[   ]<%3zu  >: [](" STACK_VALUE_PRINTF_SPEC ")\n", i, cake->buffer[i]);
    }

    return validity;
}

int STACK_GENERIC(resize)(STACK_GENERIC_TYPE *cake, const size_t new_capacity) {
    STACK_OK(cake);

    if (new_capacity < cake->size) {
         RETURN_ERROR_VERIFY(ERR_REALLOC_FAILED);
    }

    STACK_VALUE_TYPE *new_buffer = (STACK_VALUE_TYPE*) realloc(cake->buffer, sizeof(STACK_VALUE_TYPE) * new_capacity);
    if (!new_buffer) {
        RETURN_ERROR_VERIFY(ERR_REALLOC_FAILED);
    }

    cake->buffer   = new_buffer;
    cake->capacity = new_capacity;
    STACK_GENERIC(recalcute_security)(cake);

    STACK_OK(cake);
    return OK;
}

int STACK_GENERIC(resize_up)(STACK_GENERIC_TYPE *cake) {
    STACK_OK(cake);

    RETURNING_VERIFY_OK(STACK_GENERIC(resize)(cake, (size_t)((double) STACK_GENERIC(capacity)(cake) * STACK_REALLOC_UP_COEF)));

    STACK_OK(cake);
    return OK;
}

int STACK_GENERIC(resize_down)(STACK_GENERIC_TYPE *cake) {
    STACK_OK(cake);

    RETURNING_VERIFY(STACK_GENERIC(resize)(cake, (size_t)((double) STACK_GENERIC(capacity)(cake) / STACK_REALLOC_DOWN_COEF * 1.5)) == 0);

    STACK_OK(cake);
    return OK;
}

int STACK_GENERIC(push)(STACK_GENERIC_TYPE *cake, const STACK_VALUE_TYPE val) {
    STACK_OK(cake);

    if (STACK_GENERIC(is_full)(cake)) {
        STACK_GENERIC(resize_up)(cake);
    }

    cake->buffer[cake->size++] = val;
    STACK_GENERIC(recalcute_security)(cake);

    STACK_OK(cake);
    return OK;
}

int STACK_GENERIC(pop)(STACK_GENERIC_TYPE *cake) {
    STACK_OK(cake);
    RETURNING_VERIFY(cake->size > 0);

    --cake->size;
    STACK_GENERIC(recalcute_security)(cake);

    if ((double) cake->capacity / (double) (cake->size + 1) > STACK_REALLOC_DOWN_COEF) {
        STACK_GENERIC(resize_down)(cake);
    }

    STACK_GENERIC(recalcute_security)(cake);

    STACK_OK(cake);
    return OK;
}

int STACK_GENERIC(clear)(STACK_GENERIC_TYPE *cake) {
    STACK_OK(cake);

    size_t init_size = STACK_GENERIC(size)(cake);
    for (size_t i = 0; i < init_size; ++i) {
        RETURNING_VERIFY_OK(STACK_GENERIC(pop)(cake));
    }

    STACK_OK(cake);
    return OK;
}

char STACK_GENERIC(is_empty)(const STACK_GENERIC_TYPE *cake) {
    STACK_OK(cake);
    return STACK_GENERIC(size)(cake) == 0;
}

char STACK_GENERIC(is_full)(const STACK_GENERIC_TYPE *cake) {
    STACK_OK(cake);
    return cake->size == cake->capacity;
}

STACK_VALUE_TYPE STACK_GENERIC(top)(const STACK_GENERIC_TYPE *cake) {
    STACK_OK(cake);
    VERIFY_t(cake->size > 0, STACK_VALUE_TYPE);

    return cake->buffer[cake->size - 1];
}
//=============================================================================
