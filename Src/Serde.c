#include "Serde.h"

static const uint8_t PRIMARY_TYPE_SIZE[Serde_PrimaryType_Length] = {
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

static void* Serde_Primary_alignAddress(void* pValue, Serde_Type type, Serde_LenType len, Serde_LenType len2) {
    uint8_t objSize = PRIMARY_TYPE_SIZE[type & SERDE_TYPE_PRIMARY_MASK];
    uint32_t pobj = (uint8_t*) pValue;

    if (pobj % objSize != 0) {
        pobj += objSize - pobj % objSize;
    }

    return (void*) pobj;
}
static void* Serde_Primary_moveAddress(void* pValue, Serde_Type type, Serde_LenType len, Serde_LenType len2) {
    uint8_t objSize = PRIMARY_TYPE_SIZE[type & SERDE_TYPE_PRIMARY_MASK];
    return (uint8_t*) pValue + objSize;
}

static void* Serde_Pointer_alignAddress(void* pValue, Serde_Type type, Serde_LenType len, Serde_LenType len2) {
    uint8_t objSize = sizeof(void*);
    uint32_t pobj = (uint8_t*) pValue;

    if (pobj % objSize != 0) {
        pobj += objSize - pobj % objSize;
    }

    return (void*) pobj;
}
static void* Serde_Pointer_moveAddress(void* pValue, Serde_Type type, Serde_LenType len, Serde_LenType len2) {
    uint8_t objSize = sizeof(void*);
    return (uint8_t*) pValue + objSize;
}

#define SERDE_HELPER(CAT)           { Serde_ ##CAT ##_alignAddress, Serde_ ##CAT ##_moveAddress }

static const Serde_Type_Helper SERDE_HELPER[Serde_Category_Length] = {
    SERDE_HELPER(Primary),
    SERDE_HELPER(Pointer),
};

void Serde_serializePrimary(void* out, const uint8_t* fmt, void* obj, Serde_SerializeFn* serialize) {
    Serde_Type_BitFields ptype;
    uint8_t* pobj = (uint8_t*) obj;
    Serde_Type_Helper* helper;

    while (*fmt != Serde_Type_Unknown) {
        ptype.Type = *fmt++;
        helper = &SERDE_HELPER[ptype.Category];
        // check align
        pobj = helper->alignAddress(pobj, ptype.Type, 0, 0);
        // serialize
        if (*serialize != NULL) {
            (*serialize)(out, pobj, ptype.Type, 0, 0);
        }
        serialize++;
        // move pobj
        pobj = helper->moveAddress(pobj, ptype.Type, 0, 0);
    }
}

