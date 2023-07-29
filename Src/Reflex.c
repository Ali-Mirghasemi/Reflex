#include "Reflex.h"

/* Private typedef */
typedef void* (*Reflex_AlignAddressFn)(void* pValue, Reflex_TypeParams* fmt);
typedef void* (*Reflex_MoveAddressFn)(void* pValue, Reflex_TypeParams* fmt);
typedef Reflex_Result (*Reflex_ScanFn)(Reflex* reflex, void* obj, Reflex_FieldFn onField);
typedef Reflex_Result (*Reflex_SerdeFn)(Reflex* reflex, void* obj);
typedef Reflex_LenType (*Reflex_SizeFn)(const Reflex_Schema* schema);
/**
 * @brief This struct hold helper functions for manage serialize and deserialize
 */
typedef struct {
    Reflex_AlignAddressFn    alignAddress;
    Reflex_MoveAddressFn     moveAddress;
} Reflex_Type_Helper;

/* Private Functions */
static Reflex_Result Reflex_serialize_Callback(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt);
static Reflex_Result Reflex_serialize_Driver(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt);
static Reflex_Result Reflex_serialize_Compact(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt);
static Reflex_Result Reflex_deserialize_Callback(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt);
static Reflex_Result Reflex_deserialize_Driver(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt);
static Reflex_Result Reflex_deserialize_Compact(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt);

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
/* Private Macro */
#if REFLEX_SUPPORT_CUSTOM_TYPE_PARAMS
    #define __isCustom_init(schema)       const uint8_t isCustom = schema->FmtSize != 0 && schema->FmtSize != sizeof(Reflex_TypeParams);
    #define __nextFmt(schema, fmt)        fmt = isCustom ? (const Reflex_TypeParams*)((uint8_t*) fmt + schema->FmtSize) : \
                                                      fmt + 1;
#else
    #define __isCustom_init(schema)
    #define __nextFmt(schema, fmt)        fmt++
#endif

static const Reflex_ScanFn REFLEX_SCAN[] = {
#if REFLEX_FORMAT_MODE_PARAM
    Reflex_Param_scan,
#endif
#if REFLEX_FORMAT_MODE_PRIMARY
    Reflex_Primary_scan,
#endif
#if REFLEX_FORMAT_MODE_OFFSET
    Reflex_Offset_scan,
#endif
};

static const Reflex_FieldFn REFLEX_SERIALIZE[3] = {
    Reflex_serialize_Callback,
    Reflex_serialize_Driver,
    Reflex_serialize_Compact,
};

static const Reflex_FieldFn REFLEX_DESERIALIZE[3] = {
    Reflex_deserialize_Callback,
    Reflex_deserialize_Driver,
    Reflex_deserialize_Compact,
};

static const Reflex_SizeFn REFLEX_SIZE[3][2] = {
#if REFLEX_FORMAT_MODE_PARAM
    { Reflex_Param_sizeNormal, Reflex_Param_sizePacked, },
#endif
#if REFLEX_FORMAT_MODE_PRIMARY
    { Reflex_Primary_sizeNormal, Reflex_Primary_sizePacked, },
#endif
#if REFLEX_FORMAT_MODE_OFFSET
    { Reflex_Offset_sizeNormal, Reflex_Offset_sizePacked, },
#endif
};

#if REFLEX_ARCH == 8
    #define Reflex_alignAddress(pValue, objSize)        pValue
#else
static void* Reflex_alignAddress(void* pValue, Reflex_LenType objSize) {
    uint32_t pobj = (uint32_t) pValue;

#if REFLEX_ARCH != 64
    if (objSize > REFLEX_ARCH_BYTES) {
        objSize = REFLEX_ARCH_BYTES;
    }
#endif

    if (pobj % objSize != 0) {
        pobj += objSize - pobj % objSize;
    }

    return (void*) pobj;
}
#endif // REFLEX_ARCH

static void* Reflex_Primary_alignAddress(void* pValue, Reflex_TypeParams* fmt) {
    return Reflex_alignAddress(pValue, PRIMARY_TYPE_SIZE[fmt->Fields.Primary]);
}
static void* Reflex_Primary_moveAddress(void* pValue, Reflex_TypeParams* fmt) {
    uint8_t objSize = PRIMARY_TYPE_SIZE[fmt->Fields.Primary];
    return (uint8_t*) pValue + objSize;
}

static void* Reflex_Pointer_alignAddress(void* pValue, Reflex_TypeParams* fmt) {
    return Reflex_alignAddress(pValue, sizeof(void*));
}
static void* Reflex_Pointer_moveAddress(void* pValue, Reflex_TypeParams* fmt) {
    uint8_t objSize = sizeof(void*);
    return (uint8_t*) pValue + objSize;
}

#define Reflex_Array_alignAddress Reflex_Primary_alignAddress

static void* Reflex_Array_moveAddress(void* pValue, Reflex_TypeParams* fmt) {
    uint8_t objSize = PRIMARY_TYPE_SIZE[fmt->Fields.Primary] * fmt->Len;
    return (uint8_t*) pValue + objSize;
}

#define Reflex_PointerArray_alignAddress Reflex_Pointer_alignAddress

static void* Reflex_PointerArray_moveAddress(void* pValue, Reflex_TypeParams* fmt) {
    uint8_t objSize = sizeof(void*) * fmt->Len;
    return (uint8_t*) pValue + objSize;
}

#define Reflex_Array2D_alignAddress Reflex_Primary_alignAddress

static void* Reflex_Array2D_moveAddress(void* pValue, Reflex_TypeParams* fmt) {
    uint8_t objSize = PRIMARY_TYPE_SIZE[fmt->Fields.Primary] * fmt->Len * fmt->MLen;
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

#if REFLEX_FORMAT_MODE_PARAM
Reflex_Result Reflex_Param_scan(Reflex* reflex, void* obj, Reflex_FieldFn onField) {
    Reflex_Result result = REFLEX_OK;
    uint8_t* pobj = (uint8_t*) obj;
    const Reflex_Type_Helper* helper;
    const Reflex_TypeParams* fmt = reflex->Schema->Fmt;
    Reflex_LenType len = reflex->Schema->Len;
    __isCustom_init(reflex->Schema);
    reflex->VariableIndex = 0;
    reflex->Obj = obj;

    while (len-- > 0 && result == REFLEX_OK) {
        helper = &REFLEX_HELPER[fmt->Fields.Category];
        // check align
        pobj = helper->alignAddress(pobj, fmt);
        // onField
        if (onField) {
            result = onField(reflex, pobj, fmt);
        }
        // move pobj
        pobj = helper->moveAddress(pobj, fmt);
        // next fmt
        __nextFmt(reflex->Schema, fmt);
        reflex->VariableIndex++;
    }

    return result;
}

Reflex_Result Reflex_Param_serialize(Reflex* reflex, void* obj) {
    return Reflex_Param_scan(reflex, obj, REFLEX_SERIALIZE[reflex->FunctionMode]);
}

Reflex_Result Reflex_Param_deserialize(Reflex* reflex, void* obj) {
    return Reflex_Param_scan(reflex, obj, REFLEX_DESERIALIZE[reflex->FunctionMode]);
}

Reflex_LenType Reflex_Param_sizeNormal(const Reflex_Schema* schema) {
    uint8_t* pobj = (uint8_t*) 0;
    const Reflex_Type_Helper* helper;
    const Reflex_TypeParams* fmt = schema->Fmt;
    Reflex_LenType len = schema->Len;
    Reflex_LenType objsize = 0;
    __isCustom_init(schema);

    while (len-- > 0) {
        helper = &REFLEX_HELPER[fmt->Fields.Category];
        if (objsize < PRIMARY_TYPE_SIZE[fmt->Fields.Primary]) {
            objsize = PRIMARY_TYPE_SIZE[fmt->Fields.Primary];
        }
        // check align
        pobj = helper->alignAddress(pobj, fmt);
        // move pobj
        pobj = helper->moveAddress(pobj, fmt);
        // next fmt
        __nextFmt(schema, fmt);
    }

    pobj = Reflex_alignAddress(pobj, objsize);

    return (Reflex_LenType) pobj;
}

Reflex_LenType Reflex_Param_sizePacked(const Reflex_Schema* schema) {
    uint8_t* pobj = (uint8_t*) 0;
    const Reflex_Type_Helper* helper;
    const Reflex_TypeParams* fmt = schema->Fmt;
    Reflex_LenType len = schema->Len;
    __isCustom_init(schema);

    while (len-- > 0) {
        helper = &REFLEX_HELPER[fmt->Fields.Category];
        // move pobj
        pobj = helper->moveAddress(pobj, fmt);
        // next fmt
        __nextFmt(schema, fmt);
    }
    return (Reflex_LenType) pobj;
}
#endif // REFLEX_FORMAT_MODE_PARAM

#if REFLEX_FORMAT_MODE_PRIMARY
Reflex_Result Reflex_Primary_scan(Reflex* reflex, void* obj, Reflex_FieldFn onField) {
    Reflex_Result result = REFLEX_OK;
    Reflex_TypeParams fmt = {0};
    uint8_t* pobj = (uint8_t*) obj;
    const Reflex_Type_Helper* helper;
    const uint8_t* pfmt = reflex->Schema->PrimaryFmt;
    reflex->VariableIndex = 0;
    reflex->Obj = obj;

    while (*pfmt != Reflex_Type_Unknown && result == REFLEX_OK) {
        fmt.Type = *pfmt++;
        helper = &REFLEX_HELPER[fmt.Fields.Category];
        // check align
        pobj = helper->alignAddress(pobj, &fmt);
        // onField
        if (onField) {
            result = onField(reflex, pobj, &fmt);
        }
        // move pobj
        pobj = helper->moveAddress(pobj, &fmt);
        reflex->VariableIndex++;
    }

    return result;
}

Reflex_Result Reflex_Primary_serialize(Reflex* reflex, void* obj) {
    return Reflex_Primary_scan(reflex, obj, REFLEX_SERIALIZE[reflex->FunctionMode]);
}

Reflex_Result Reflex_Primary_deserialize(Reflex* reflex, void* obj) {
    return Reflex_Primary_scan(reflex, obj, REFLEX_DESERIALIZE[reflex->FunctionMode]);
}

Reflex_LenType Reflex_Primary_sizeNormal(const Reflex_Schema* schema) {
    Reflex_TypeParams fmt = {0};
    uint8_t* pobj = (uint8_t*) 0;
    const Reflex_Type_Helper* helper;
    const uint8_t* pfmt = schema->PrimaryFmt;
    Reflex_LenType objsize = 0;

    while (*pfmt != Reflex_Type_Unknown) {
        fmt.Type = *pfmt++;
        helper = &REFLEX_HELPER[fmt.Fields.Category];
        if (objsize < PRIMARY_TYPE_SIZE[fmt.Type]) {
            objsize = PRIMARY_TYPE_SIZE[fmt.Type];
        }
        // check align
        pobj = helper->alignAddress(pobj, &fmt);
        // move pobj
        pobj = helper->moveAddress(pobj, &fmt);
    }

    pobj = Reflex_alignAddress(pobj, objsize);

    return (Reflex_LenType) pobj;
}

Reflex_LenType Reflex_Primary_sizePacked(const Reflex_Schema* schema) {
    Reflex_TypeParams fmt = {0};
    uint8_t* pobj = (uint8_t*) 0;
    const Reflex_Type_Helper* helper;
    const uint8_t* pfmt = schema->PrimaryFmt;

    while (*pfmt != Reflex_Type_Unknown) {
        fmt.Type = *pfmt++;
        helper = &REFLEX_HELPER[fmt.Fields.Category];
        // move pobj
        pobj = helper->moveAddress(pobj, &fmt);
    }
    return (Reflex_LenType) pobj;
}
#endif // REFLEX_FORMAT_MODE_PRIMARY

#if REFLEX_FORMAT_MODE_OFFSET
Reflex_Result Reflex_Offset_scan(Reflex* reflex, void* obj, Reflex_FieldFn onField) {
    Reflex_Result result = REFLEX_OK;
    uint8_t* pobj = (uint8_t*) obj;
    const Reflex_TypeParams* fmt = reflex->Schema->Fmt;
    Reflex_LenType len = reflex->Schema->Len;
    __isCustom_init(reflex->Schema);
    reflex->VariableIndex = 0;
    reflex->Obj = obj;

    while (len-- > 0 && result == REFLEX_OK) {
        // check align
        pobj = (uint8_t*) obj + fmt->Offset;
        // onField
        if (onField) {
            result = onField(reflex, pobj, fmt);
        }
        // next fmt
        __nextFmt(reflex->Schema, fmt);
        reflex->VariableIndex++;
    }

    return result;
}

Reflex_Result Reflex_Offset_serialize(Reflex* reflex, void* obj) {
    return Reflex_Offset_scan(reflex, obj, REFLEX_SERIALIZE[reflex->FunctionMode]);
}

Reflex_Result Reflex_Offset_deserialize(Reflex* reflex, void* obj) {
    return Reflex_Offset_scan(reflex, obj, REFLEX_DESERIALIZE[reflex->FunctionMode]);
}

Reflex_LenType Reflex_Offset_sizeNormal(const Reflex_Schema* schema) {
    uint8_t* pobj = (uint8_t*) 0;
    const Reflex_Type_Helper* helper;
    const Reflex_TypeParams* fmt = schema->Fmt;
    Reflex_LenType len = schema->Len;
    Reflex_LenType objsize = 0;
    const Reflex_TypeParams* maxOffset = fmt;
    __isCustom_init(schema);

    while (len-- > 0) {
        helper = &REFLEX_HELPER[fmt->Fields.Category];
        if (objsize < PRIMARY_TYPE_SIZE[fmt->Fields.Primary]) {
            objsize = PRIMARY_TYPE_SIZE[fmt->Fields.Primary];
        }
        // find max offset
        if (maxOffset->Offset < fmt->Offset) {
            maxOffset = fmt;
        }
        // next fmt
        __nextFmt(schema, fmt);
    }
    // max offset + size
    pobj = maxOffset->Offset + Reflex_sizeType(maxOffset);
    // align to biggest field
    pobj = Reflex_alignAddress(pobj, objsize);

    return (Reflex_LenType) pobj;
}

Reflex_LenType Reflex_Offset_sizePacked(const Reflex_Schema* schema) {
    uint8_t* pobj = (uint8_t*) 0;
    const Reflex_Type_Helper* helper;
    const Reflex_TypeParams* fmt = schema->Fmt;
    Reflex_LenType len = schema->Len;
    __isCustom_init(schema);

    while (len-- > 0) {
        helper = &REFLEX_HELPER[fmt->Fields.Category];
        // move pobj
        pobj = helper->moveAddress(pobj, fmt);
        // next fmt
        __nextFmt(schema, fmt);
    }
    return (Reflex_LenType) pobj;
}
#endif // REFLEX_FORMAT_MODE_OFFSET

Reflex_Result Reflex_scan(Reflex* reflex, void* obj, Reflex_FieldFn onField) {
    return REFLEX_SCAN[(uint8_t) reflex->Schema->FormatMode](reflex, obj, onField);
}

Reflex_Result Reflex_serialize(Reflex* reflex, void* obj) {
    return Reflex_scan(reflex, obj, REFLEX_SERIALIZE[reflex->FunctionMode]);
}

Reflex_Result Reflex_deserialize(Reflex* reflex, void* obj) {
    return Reflex_scan(reflex, obj, REFLEX_DESERIALIZE[reflex->FunctionMode]);
}

Reflex_LenType Reflex_size(const Reflex_Schema* schema, Reflex_SizeType type) {
    return REFLEX_SIZE[schema->FormatMode][type](schema);
}

Reflex_LenType Reflex_sizeType(const Reflex_TypeParams* fmt) {
    return (Reflex_LenType) REFLEX_HELPER[fmt->Fields.Category].moveAddress((void*) 0, fmt);
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

static Reflex_Result Reflex_serialize_Callback(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt) {
    return reflex->serializeCb(reflex, reflex->Buffer, obj, fmt->Type, fmt->Len, fmt->MLen);
}
static Reflex_Result Reflex_serialize_Driver(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt) {
    return reflex->Serialize->Category[fmt->Fields.Category]->fn[fmt->Fields.Primary](reflex, reflex->Buffer, obj, fmt->Fields.Type, fmt->Len, fmt->MLen);
}
static Reflex_Result Reflex_serialize_Compact(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt) {
    return reflex->SerializeFns->fn[fmt->Fields.Primary](reflex, reflex->Buffer, obj, fmt->Fields.Type, fmt->Len, fmt->MLen);
}
static Reflex_Result Reflex_deserialize_Callback(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt) {
    return reflex->deserializeCb(reflex, reflex->Buffer, obj, fmt->Type, fmt->Len, fmt->MLen);
}
static Reflex_Result Reflex_deserialize_Driver(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt) {
    return reflex->Deserialize->Category[fmt->Fields.Category]->fn[fmt->Fields.Primary](reflex, reflex->Buffer, obj, fmt->Fields.Type, fmt->Len, fmt->MLen);
}
static Reflex_Result Reflex_deserialize_Compact(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt) {
    return reflex->DeserializeFns->fn[fmt->Fields.Primary](reflex, reflex->Buffer, obj, fmt->Fields.Type, fmt->Len, fmt->MLen);
}
