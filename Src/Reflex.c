#include "Reflex.h"

/* Private typedef */
typedef Reflex_LenType (*Reflex_SizeFn)(Reflex* Reflex);

/* Private Functions */

/* Private variables */
static const uint8_t PRIMARY_TYPE_SIZE[Reflex_PrimaryType_Length] = {
    1, // sizeof(void),
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

static const Reflex_SizeFn REFLEX_SIZE[2][2] = {
    { Reflex_sizeNormal, Reflex_sizePacked,},
    { Reflex_sizeNormalPrimary, Reflex_sizePackedPrimary,},
};

static void* Reflex_alignAddress(void* pValue, Reflex_LenType objSize) {
    uint32_t pobj = (uint32_t) pValue;
    
    if (pobj % objSize != 0) {
        pobj += objSize - pobj % objSize;
    }

    return (void*) pobj;
}

static void* Reflex_Primary_alignAddress(void* pValue, Reflex_Type type, Reflex_LenType len, Reflex_LenType len2) {
    return Reflex_alignAddress(pValue, PRIMARY_TYPE_SIZE[type & REFLEX_TYPE_PRIMARY_MASK]);
}
static void* Reflex_Primary_moveAddress(void* pValue, Reflex_Type type, Reflex_LenType len, Reflex_LenType len2) {
    uint8_t objSize = PRIMARY_TYPE_SIZE[type & REFLEX_TYPE_PRIMARY_MASK];
    return (uint8_t*) pValue + objSize;
}

static void* Reflex_Pointer_alignAddress(void* pValue, Reflex_Type type, Reflex_LenType len, Reflex_LenType len2) {
    return Reflex_alignAddress(pValue, sizeof(void*));
}
static void* Reflex_Pointer_moveAddress(void* pValue, Reflex_Type type, Reflex_LenType len, Reflex_LenType len2) {
    uint8_t objSize = sizeof(void*);
    return (uint8_t*) pValue + objSize;
}

#define Reflex_Array_alignAddress Reflex_Primary_alignAddress

static void* Reflex_Array_moveAddress(void* pValue, Reflex_Type type, Reflex_LenType len, Reflex_LenType len2) {
    uint8_t objSize = PRIMARY_TYPE_SIZE[type & REFLEX_TYPE_PRIMARY_MASK] * len;
    return (uint8_t*) pValue + objSize;
}

#define Reflex_PointerArray_alignAddress Reflex_Pointer_alignAddress

static void* Reflex_PointerArray_moveAddress(void* pValue, Reflex_Type type, Reflex_LenType len, Reflex_LenType len2) {
    uint8_t objSize = sizeof(void*) * len;
    return (uint8_t*) pValue + objSize;
}

#define Reflex_2DArray_alignAddress Reflex_Primary_alignAddress

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
    const Reflex_Type_Helper* helper;
    const uint8_t* fmt = reflex->PrimaryFmt;
    const Reflex_SerializeFn* serialize = reflex->FunctionMode != Reflex_FunctionMode_Single ? reflex->SerializeFunctions : &reflex->Serialize;
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
        if (reflex->FunctionMode != Reflex_FunctionMode_Single) {
            serialize++;
        }
        // move pobj
        pobj = helper->moveAddress(pobj, ptype.Type, 0, 0);
    }
}

void Reflex_scan(Reflex* reflex, void* obj) {
    uint8_t* pobj = (uint8_t*) obj;
    const Reflex_Type_Helper* helper;
    const Reflex_TypeParams* fmt = reflex->Fmt;
    const Reflex_SerializeFn* serialize = reflex->FunctionMode ? reflex->SerializeFunctions : &reflex->Serialize;
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
        if (reflex->FunctionMode != Reflex_FunctionMode_Single) {
            serialize++;
        }
        // move pobj
        pobj = helper->moveAddress(pobj, fmt->Fields.Type, fmt->Len, fmt->Len2);
        // next fmt
        fmt++;
    }
}

Reflex_LenType Reflex_size(Reflex* reflex, Reflex_SizeType type) {
    return REFLEX_SIZE[reflex->FormatMode][type](reflex);
}

Reflex_LenType Reflex_sizeNormal(Reflex* reflex) {
    uint8_t* pobj = (uint8_t*) 0;
    const Reflex_Type_Helper* helper;
    const Reflex_TypeParams* fmt = reflex->Fmt;
    Reflex_LenType len = reflex->VariablesLength;
    Reflex_LenType objsize = 0;

    while (len-- > 0) {
        helper = &REFLEX_HELPER[fmt->Fields.Category];
        if (objsize < PRIMARY_TYPE_SIZE[fmt->Fields.Primary]) {
            objsize = PRIMARY_TYPE_SIZE[fmt->Fields.Primary];
        }
        // check align
        pobj = helper->alignAddress(pobj, fmt->Fields.Type, fmt->Len, fmt->Len2);
        // move pobj
        pobj = helper->moveAddress(pobj, fmt->Fields.Type, fmt->Len, fmt->Len2);
        // next fmt
        fmt++;
    }

    pobj = Reflex_alignAddress(pobj, objsize);

    return (Reflex_LenType) pobj;
}
Reflex_LenType Reflex_sizePacked(Reflex* reflex) {
    uint8_t* pobj = (uint8_t*) 0;
    const Reflex_Type_Helper* helper;
    const Reflex_TypeParams* fmt = reflex->Fmt;
    Reflex_LenType len = reflex->VariablesLength;

    while (len-- > 0) {
        helper = &REFLEX_HELPER[fmt->Fields.Category];
        // move pobj
        pobj = helper->moveAddress(pobj, fmt->Fields.Type, fmt->Len, fmt->Len2);
        // next fmt
        fmt++;
    }
    return (Reflex_LenType) pobj;
}
Reflex_LenType Reflex_sizeNormalPrimary(Reflex* reflex) {
    Reflex_Type_BitFields ptype;
    uint8_t* pobj = (uint8_t*) 0;
    const Reflex_Type_Helper* helper;
    const uint8_t* fmt = reflex->PrimaryFmt;
    Reflex_LenType objsize = 0;

    while (*fmt != Reflex_Type_Unknown) {
        ptype.Type = *fmt++;
        helper = &REFLEX_HELPER[ptype.Category];
        if (objsize < PRIMARY_TYPE_SIZE[ptype.Type]) {
            objsize = PRIMARY_TYPE_SIZE[ptype.Type];
        }
        // check align
        pobj = helper->alignAddress(pobj, ptype.Type, 0, 0);
        // move pobj
        pobj = helper->moveAddress(pobj, ptype.Type, 0, 0);
    }

    pobj = Reflex_alignAddress(pobj, objsize);

    return (Reflex_LenType) pobj;
}
Reflex_LenType Reflex_sizePackedPrimary(Reflex* reflex) {
    Reflex_Type_BitFields ptype;
    uint8_t* pobj = (uint8_t*) 0;
    const Reflex_Type_Helper* helper;
    const uint8_t* fmt = reflex->PrimaryFmt;

    while (*fmt != Reflex_Type_Unknown) {
        ptype.Type = *fmt++;
        helper = &REFLEX_HELPER[ptype.Category];
        // move pobj
        pobj = helper->moveAddress(pobj, ptype.Type, 0, 0);
    }
    return (Reflex_LenType) pobj;
}

