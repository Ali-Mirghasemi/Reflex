#include "Reflex.h"

static const uint8_t PRIMARY_TYPE_SIZE[Reflex_PrimaryType_Length] = {
    sizeof(void),
    sizeof(char),
    sizeof(uint8_t),
    sizeof(int8_t),
    sizeof(uint16_t),
    sizeof(int16_t),
    sizeof(uint32_t),
    sizeof(int32_t),
    sizeof(uint64_t),
    sizeof(int64_t),
    sizeof(float),
    sizeof(double),
};

static void* Reflex_Primary_alignAddress(void* pValue, Reflex_Type type, Reflex_LenType len, Reflex_LenType len2) {
    uint8_t objSize = PRIMARY_TYPE_SIZE[type & REFLEX_TYPE_PRIMARY_MASK];
    uint32_t pobj = (uint8_t*) pValue;

    if (pobj % objSize != 0) {
        pobj += objSize - pobj % objSize;
    }

    return (void*) pobj;
}
static void* Reflex_Primary_moveAddress(void* pValue, Reflex_Type type, Reflex_LenType len, Reflex_LenType len2) {
    uint8_t objSize = PRIMARY_TYPE_SIZE[type & REFLEX_TYPE_PRIMARY_MASK];
    return (uint8_t*) pValue + objSize;
}

static void* Reflex_Pointer_alignAddress(void* pValue, Reflex_Type type, Reflex_LenType len, Reflex_LenType len2) {
    uint8_t objSize = sizeof(void*);
    uint32_t pobj = (uint8_t*) pValue;

    if (pobj % objSize != 0) {
        pobj += objSize - pobj % objSize;
    }

    return (void*) pobj;
}
static void* Reflex_Pointer_moveAddress(void* pValue, Reflex_Type type, Reflex_LenType len, Reflex_LenType len2) {
    uint8_t objSize = sizeof(void*);
    return (uint8_t*) pValue + objSize;
}

static void* Reflex_Array_alignAddress(void* pValue, Reflex_Type type, Reflex_LenType len, Reflex_LenType len2) {
    uint8_t objSize = PRIMARY_TYPE_SIZE[type & REFLEX_TYPE_PRIMARY_MASK];
    uint32_t pobj = (uint8_t*) pValue;

    if (pobj % objSize != 0) {
        pobj += objSize - pobj % objSize;
    }

    return (void*) pobj;
}
static void* Reflex_Array_moveAddress(void* pValue, Reflex_Type type, Reflex_LenType len, Reflex_LenType len2) {
    uint8_t objSize = PRIMARY_TYPE_SIZE[type & REFLEX_TYPE_PRIMARY_MASK] * len;
    return (uint8_t*) pValue + objSize;
}

static void* Reflex_PointerArray_alignAddress(void* pValue, Reflex_Type type, Reflex_LenType len, Reflex_LenType len2) {
    uint8_t objSize = sizeof(void*);
    uint32_t pobj = (uint8_t*) pValue;

    if (pobj % objSize != 0) {
        pobj += objSize - pobj % objSize;
    }

    return (void*) pobj;
}
static void* Reflex_PointerArray_moveAddress(void* pValue, Reflex_Type type, Reflex_LenType len, Reflex_LenType len2) {
    uint8_t objSize = sizeof(void*) * len;
    return (uint8_t*) pValue + objSize;
}

static void* Reflex_2DArray_alignAddress(void* pValue, Reflex_Type type, Reflex_LenType len, Reflex_LenType len2) {
    uint8_t objSize = PRIMARY_TYPE_SIZE[type & REFLEX_TYPE_PRIMARY_MASK];
    uint32_t pobj = (uint8_t*) pValue;

    if (pobj % objSize != 0) {
        pobj += objSize - pobj % objSize;
    }

    return (void*) pobj;
}
static void* Reflex_2DArray_moveAddress(void* pValue, Reflex_Type type, Reflex_LenType len, Reflex_LenType len2) {
    uint8_t objSize = PRIMARY_TYPE_SIZE[type & REFLEX_TYPE_PRIMARY_MASK] * len * len2;
    return (uint8_t*) pValue + objSize;
}

#define REFLEX_HELPER(CAT)           { Reflex_ ##CAT ##_alignAddress, Reflex_ ##CAT ##_moveAddress }

static const Reflex_Type_Helper REFLEX_HELPER[Reflex_Category_Length] = {
    REFLEX_HELPER(Primary),
    REFLEX_HELPER(Pointer),
    REFLEX_HELPER(Array),
    REFLEX_HELPER(PointerArray),
    REFLEX_HELPER(2DArray),
};

void Reflex_scanPrimary(Reflex* reflex, void* obj) {
    Reflex_Type_BitFields ptype;
    uint8_t* pobj = (uint8_t*) obj;
    Reflex_Type_Helper* helper;
    const uint8_t* fmt = reflex->Fmt;
    const Reflex_SerializeFn* serialize = reflex->RepeatFn ? reflex->SerializeFunctions : &reflex->Serialize;
    void* out = reflex->Buffer;

    while (*fmt != Reflex_Type_Unknown) {
        ptype.Type = *fmt++;
        helper = &REFLEX_HELPER[ptype.Category];
        // check align
        pobj = helper->alignAddress(pobj, ptype.Type, 0, 0);
        // serialize
        if (*serialize != NULL) {
            (*serialize)(reflex, out, pobj, ptype.Type, 0, 0);
        }
        if (!reflex->RepeatFn) {
            serialize++;
        }
        // move pobj
        pobj = helper->moveAddress(pobj, ptype.Type, 0, 0);
    }
}

void Reflex_scan(Reflex* reflex, void* obj) {
    uint8_t* pobj = (uint8_t*) obj;
    Reflex_Type_Helper* helper;
    const Reflex_TypeParams* fmt = reflex->Fmt;
    const Reflex_SerializeFn* serialize = reflex->SerializeFunctions;
    void* out = reflex->Buffer;
    Reflex_LenType len = reflex->VariablesLength;

    while (len-- > 0) {
        helper = &REFLEX_HELPER[fmt->Fields.Category];
        // check align
        pobj = helper->alignAddress(pobj, fmt->Fields.Type, fmt->Len, fmt->Len2);
        // serialize
        if (*serialize != NULL) {
            (*serialize)(reflex, out, pobj, fmt->Fields.Type, fmt->Len, fmt->Len2);
        }
        if (!reflex->RepeatFn) {
            serialize++;
        }
        // move pobj
        pobj = helper->moveAddress(pobj, fmt->Fields.Type, fmt->Len, fmt->Len2);
        // next fmt
        fmt++;
    }
}

