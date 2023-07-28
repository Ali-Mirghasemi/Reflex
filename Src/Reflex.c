#include "Reflex.h"

/* Private typedef */
typedef void* (*Reflex_AlignAddressFn)(void* pValue, Reflex_Type type, Reflex_LenType len, Reflex_LenType len2);
typedef void* (*Reflex_MoveAddressFn)(void* pValue, Reflex_Type type, Reflex_LenType len, Reflex_LenType len2);
typedef Reflex_Result (*Reflex_SerdeFn)(Reflex* reflex, void* obj);
/**
 * @brief This struct hold helper functions for manage serialize and deserialize
 */
typedef struct {
    Reflex_AlignAddressFn    alignAddress;
    Reflex_MoveAddressFn     moveAddress;
} Reflex_Type_Helper;
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

static const Reflex_SerdeFn REFLEX_SERIALIZE[] = {
    Reflex_serializeParam,
    Reflex_serializePrimary,
};

static const Reflex_SerdeFn REFLEX_DESERIALIZE[] = {
    Reflex_deserializeParam,
    Reflex_deserializePrimary,
};

static const Reflex_SizeFn REFLEX_SIZE[2][2] = {
    { Reflex_sizeNormal, Reflex_sizePacked,},
    { Reflex_sizeNormalPrimary, Reflex_sizePackedPrimary,},
};

#if REFLEX_ARCH == 8
    #define Reflex_alignAddress(pValue, objSize)        pValue
#else 
static void* Reflex_alignAddress(void* pValue, Reflex_LenType objSize) {
    uint32_t pobj = (uint32_t) pValue;
    
    if (pobj % objSize != 0) {
        pobj += objSize - pobj % objSize;
    }

    return (void*) pobj;
}
#endif // REFLEX_ARCH

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

#define Reflex_Array2D_alignAddress Reflex_Primary_alignAddress

static void* Reflex_Array2D_moveAddress(void* pValue, Reflex_Type type, Reflex_LenType len, Reflex_LenType len2) {
    uint8_t objSize = PRIMARY_TYPE_SIZE[type & REFLEX_TYPE_PRIMARY_MASK] * len * len2;
    return (uint8_t*) pValue + objSize;
}

#define REFLEX_HELPER(CAT)           { Reflex_ ##CAT ##_alignAddress, Reflex_ ##CAT ##_moveAddress }

static const Reflex_Type_Helper REFLEX_HELPER[Reflex_Category_Length] = {
    REFLEX_HELPER(Primary),
    REFLEX_HELPER(Pointer),
    REFLEX_HELPER(Array),
    REFLEX_HELPER(PointerArray),
    REFLEX_HELPER(Array2D),
};

Reflex_Result Reflex_serializePrimary(Reflex* reflex, void* obj) {
    Reflex_Result result = REFLEX_OK;
    Reflex_Type_BitFields ptype;
    uint8_t* pobj = (uint8_t*) obj;
    const Reflex_Type_Helper* helper;
    const uint8_t* fmt = reflex->Schema->PrimaryFmt;
    reflex->VariableIndex = 0;
    reflex->Obj = obj;

    while (*fmt != Reflex_Type_Unknown && result == REFLEX_OK) {
        ptype.Type = *fmt++;
        helper = &REFLEX_HELPER[ptype.Category];
        // check align
        pobj = helper->alignAddress(pobj, ptype.Type, 0, 0);
        // serialize
        if (reflex->FunctionMode == Reflex_FunctionMode_Callback) {
            result = reflex->serializeCb(reflex, reflex->Buffer, pobj, ptype.Type, 0, 0);
        }
        else {
            result = reflex->Serialize->Primary->fn[ptype.Primary](reflex, reflex->Buffer, pobj, ptype.Type, 0, 0);
        }
        // move pobj
        pobj = helper->moveAddress(pobj, ptype.Type, 0, 0);
        reflex->VariableIndex++;
    }

    return result;
}

Reflex_Result Reflex_deserializePrimary(Reflex* reflex, void* obj) {
    Reflex_Result result = REFLEX_OK;
    Reflex_Type_BitFields ptype;
    uint8_t* pobj = (uint8_t*) obj;
    const Reflex_Type_Helper* helper;
    const uint8_t* fmt = reflex->Schema->PrimaryFmt;
    reflex->VariableIndex = 0;
    reflex->Obj = obj;

    while (*fmt != Reflex_Type_Unknown && result == REFLEX_OK) {
        ptype.Type = *fmt++;
        helper = &REFLEX_HELPER[ptype.Category];
        // check align
        pobj = helper->alignAddress(pobj, ptype.Type, 0, 0);
        // deserialize
        if (reflex->FunctionMode == Reflex_FunctionMode_Callback) {
            result = reflex->deserializeCb(reflex, reflex->Buffer, pobj, ptype.Type, 0, 0);
        }
        else {
            result = reflex->Deserialize->Primary->fn[ptype.Primary](reflex, reflex->Buffer, pobj, ptype.Type, 0, 0);
        }
        // move pobj
        pobj = helper->moveAddress(pobj, ptype.Type, 0, 0);
        reflex->VariableIndex++;
    }

    return result;
}

Reflex_Result Reflex_serializeParam(Reflex* reflex, void* obj) {
    Reflex_Result result = REFLEX_OK;
    uint8_t* pobj = (uint8_t*) obj;
    const Reflex_Type_Helper* helper;
    const Reflex_TypeParams* fmt = reflex->Schema->Fmt;
    Reflex_LenType len = reflex->Schema->Len;
    reflex->VariableIndex = 0;
    reflex->Obj = obj;

    while (len-- > 0 && result == REFLEX_OK) {
        helper = &REFLEX_HELPER[fmt->Fields.Category];
        // check align
        pobj = helper->alignAddress(pobj, fmt->Fields.Type, fmt->Len, fmt->MLen);
        // serialize
        if (reflex->FunctionMode == Reflex_FunctionMode_Callback) {
            result = reflex->serializeCb(reflex, reflex->Buffer, pobj, fmt->Fields.Type, fmt->Len, fmt->MLen);
        }
        else {
            result = reflex->Serialize->Category[fmt->Fields.Category]->fn[fmt->Fields.Primary](reflex, reflex->Buffer, pobj, fmt->Fields.Type, fmt->Len, fmt->MLen);
        }
        // move pobj
        pobj = helper->moveAddress(pobj, fmt->Fields.Type, fmt->Len, fmt->MLen);
        // next fmt
        fmt++;
        reflex->VariableIndex++;
    }

    return result;
}

Reflex_Result Reflex_deserializeParam(Reflex* reflex, void* obj) {
    Reflex_Result result = REFLEX_OK;
    uint8_t* pobj = (uint8_t*) obj;
    const Reflex_Type_Helper* helper;
    const Reflex_TypeParams* fmt = reflex->Schema->Fmt;
    Reflex_LenType len = reflex->Schema->Len;
    reflex->VariableIndex = 0;
    reflex->Obj = obj;

    while (len-- > 0 && result == REFLEX_OK) {
        helper = &REFLEX_HELPER[fmt->Fields.Category];
        // check align
        pobj = helper->alignAddress(pobj, fmt->Fields.Type, fmt->Len, fmt->MLen);
        // deserialize
        if (reflex->FunctionMode == Reflex_FunctionMode_Callback) {
            result = reflex->deserializeCb(reflex, reflex->Buffer, pobj, fmt->Fields.Type, fmt->Len, fmt->MLen);
        }
        else {
            result = reflex->Deserialize->Category[fmt->Fields.Category]->fn[fmt->Fields.Primary](reflex, reflex->Buffer, pobj, fmt->Fields.Type, fmt->Len, fmt->MLen);
        }
        // move pobj
        pobj = helper->moveAddress(pobj, fmt->Fields.Type, fmt->Len, fmt->MLen);
        // next fmt
        fmt++;
        reflex->VariableIndex++;
    }

    return result;
}

Reflex_Result Reflex_serialize(Reflex* reflex, void* obj) {
    return REFLEX_SERIALIZE[(uint8_t) reflex->Schema->FormatMode](reflex, obj);
}

Reflex_Result Reflex_deserialize(Reflex* reflex, void* obj) {
    return REFLEX_DESERIALIZE[(uint8_t) reflex->Schema->FormatMode](reflex, obj);
}

Reflex_LenType Reflex_size(Reflex* reflex, Reflex_SizeType type) {
    return REFLEX_SIZE[reflex->Schema->FormatMode][type](reflex);
}

Reflex_LenType Reflex_sizeNormal(Reflex* reflex) {
    uint8_t* pobj = (uint8_t*) 0;
    const Reflex_Type_Helper* helper;
    const Reflex_TypeParams* fmt = reflex->Schema->Fmt;
    Reflex_LenType len = reflex->Schema->Len;
    Reflex_LenType objsize = 0;

    while (len-- > 0) {
        helper = &REFLEX_HELPER[fmt->Fields.Category];
        if (objsize < PRIMARY_TYPE_SIZE[fmt->Fields.Primary]) {
            objsize = PRIMARY_TYPE_SIZE[fmt->Fields.Primary];
        }
        // check align
        pobj = helper->alignAddress(pobj, fmt->Fields.Type, fmt->Len, fmt->MLen);
        // move pobj
        pobj = helper->moveAddress(pobj, fmt->Fields.Type, fmt->Len, fmt->MLen);
        // next fmt
        fmt++;
    }

    pobj = Reflex_alignAddress(pobj, objsize);

    return (Reflex_LenType) pobj;
}
Reflex_LenType Reflex_sizePacked(Reflex* reflex) {
    uint8_t* pobj = (uint8_t*) 0;
    const Reflex_Type_Helper* helper;
    const Reflex_TypeParams* fmt = reflex->Schema->Fmt;
    Reflex_LenType len = reflex->Schema->Len;

    while (len-- > 0) {
        helper = &REFLEX_HELPER[fmt->Fields.Category];
        // move pobj
        pobj = helper->moveAddress(pobj, fmt->Fields.Type, fmt->Len, fmt->MLen);
        // next fmt
        fmt++;
    }
    return (Reflex_LenType) pobj;
}
Reflex_LenType Reflex_sizeNormalPrimary(Reflex* reflex) {
    Reflex_Type_BitFields ptype;
    uint8_t* pobj = (uint8_t*) 0;
    const Reflex_Type_Helper* helper;
    const uint8_t* fmt = reflex->Schema->PrimaryFmt;
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
    const uint8_t* fmt = reflex->Schema->PrimaryFmt;

    while (*fmt != Reflex_Type_Unknown) {
        ptype.Type = *fmt++;
        helper = &REFLEX_HELPER[ptype.Category];
        // move pobj
        pobj = helper->moveAddress(pobj, ptype.Type, 0, 0);
    }
    return (Reflex_LenType) pobj;
}

Reflex_LenType Reflex_getVariableIndex(Reflex* reflex) {
    return reflex->VariableIndex;
}
Reflex_LenType Reflex_getVariablesLength(Reflex* reflex) {
    return reflex->Schema->Len;
}
void* Reflex_getMainVariable(Reflex* reflex) {
    return reflex->Obj;
}
