#include "Reflex.h"

/* Private typedef */
typedef void* (*Reflex_AlignAddressFn)(void* pValue, const Reflex_TypeParams* fmt);
typedef void* (*Reflex_MoveAddressFn)(void* pValue, const Reflex_TypeParams* fmt);
typedef Reflex_LenType (*Reflex_ItemSizeFn)(const Reflex_TypeParams* fmt);
typedef Reflex_Result (*Reflex_ScanFn)(Reflex* reflex, void* obj, Reflex_OnFieldFn onField);
typedef Reflex_Result (*Reflex_ComplexScanFn)(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, Reflex_OnFieldFn onField);
typedef Reflex_LenType (*Reflex_SizeFn)(const Reflex_Schema* schema);
/**
 * @brief This struct hold helper functions for manage scan
 */
typedef struct {
    Reflex_AlignAddressFn    alignAddress;
    Reflex_MoveAddressFn     moveAddress;
    Reflex_ItemSizeFn        itemSize;
} Reflex_Type_Helper;

/* Private Functions */
static Reflex_Result Reflex_scan_Callback(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt);
static Reflex_Result Reflex_scan_Driver(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt);
static Reflex_Result Reflex_scan_Compact(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt);

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
#if REFLEX_SUPPORT_64BIT_INT
    sizeof(uint64_t),
    sizeof(int64_t),
#endif
    sizeof(float),
#if REFLEX_SUPPORT_DOUBLE
    sizeof(double),
#endif
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

#if REFLEX_SUPPORT_COMPLEX_TYPE
    #define __biggestField_init()               Reflex_LenType biggestField = 0, tmpSize
    #define __updateBiggestField(SIZE)          if (biggestField < (SIZE)) { \
                                                    biggestField = (SIZE); \
                                                } 
    #define __alignObject()                     reflex->PObj = Reflex_alignAddress(pobj, biggestField); \
                                                if (reflex->AlignSize < biggestField) { \
                                                    reflex->AlignSize = biggestField; \
                                                }


    #define __startCheckComplexType(fmt)        if (fmt->Fields.Primary == Reflex_PrimaryType_Complex) { \
                                                    result = Reflex_Complex_scanRaw(reflex, pobj, fmt, onField); \
                                                    __updateBiggestField(reflex->AlignSize); \
                                                    pobj = reflex->PObj; \
                                                } \
                                                else {
    #define __endCheckComplexType(fmt, helper)      tmpSize = (helper)->itemSize(fmt); \
                                                    __updateBiggestField(tmpSize); \
                                                }
#else
    #define __biggestField_init()
    #define __updateBiggestField(SIZE)
    #define __alignObject()
    #define __startCheckComplexType(fmt)
    #define __endCheckComplexType(fmt, helper)
#endif

static const Reflex_ScanFn REFLEX_SCAN[] = {
#if REFLEX_FORMAT_MODE_PARAM
    Reflex_Param_scanRaw,
#endif
#if REFLEX_FORMAT_MODE_PRIMARY
    Reflex_Primary_scanRaw,
#endif
#if REFLEX_FORMAT_MODE_OFFSET
    Reflex_Offset_scanRaw,
#endif
};

static const Reflex_OnFieldFn REFLEX_ON_FIELD_FNS[3] = {
    Reflex_scan_Callback,
    Reflex_scan_Driver,
    Reflex_scan_Compact,
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

static Reflex_LenType Reflex_Primary_itemSize(const Reflex_TypeParams* fmt) {
    return PRIMARY_TYPE_SIZE[fmt->Fields.Primary];
}
static void* Reflex_Primary_alignAddress(void* pValue, const Reflex_TypeParams* fmt) {
    return Reflex_alignAddress(pValue, PRIMARY_TYPE_SIZE[fmt->Fields.Primary]);
}
static void* Reflex_Primary_moveAddress(void* pValue, const Reflex_TypeParams* fmt) {
    uint8_t objSize = PRIMARY_TYPE_SIZE[fmt->Fields.Primary];
    return (uint8_t*) pValue + objSize;
}

static Reflex_LenType Reflex_Pointer_itemSize(const Reflex_TypeParams* fmt) {
    return sizeof(void*);
}
static void* Reflex_Pointer_alignAddress(void* pValue, const Reflex_TypeParams* fmt) {
    return Reflex_alignAddress(pValue, sizeof(void*));
}
static void* Reflex_Pointer_moveAddress(void* pValue, const Reflex_TypeParams* fmt) {
    uint8_t objSize = sizeof(void*);
    return (uint8_t*) pValue + objSize;
}

#define Reflex_Array_itemSize Reflex_Primary_itemSize
#define Reflex_Array_alignAddress Reflex_Primary_alignAddress

static void* Reflex_Array_moveAddress(void* pValue, const Reflex_TypeParams* fmt) {
    uint8_t objSize = PRIMARY_TYPE_SIZE[fmt->Fields.Primary] * fmt->Len;
    return (uint8_t*) pValue + objSize;
}

#define Reflex_PointerArray_itemSize Reflex_Pointer_itemSize
#define Reflex_PointerArray_alignAddress Reflex_Pointer_alignAddress

static void* Reflex_PointerArray_moveAddress(void* pValue, const Reflex_TypeParams* fmt) {
    uint8_t objSize = sizeof(void*) * fmt->Len;
    return (uint8_t*) pValue + objSize;
}

#define Reflex_Array2D_itemSize Reflex_Primary_itemSize
#define Reflex_Array2D_alignAddress Reflex_Primary_alignAddress

static void* Reflex_Array2D_moveAddress(void* pValue, const Reflex_TypeParams* fmt) {
    uint8_t objSize = PRIMARY_TYPE_SIZE[fmt->Fields.Primary] * fmt->Len * fmt->MLen;
    return (uint8_t*) pValue + objSize;
}

#define REFLEX_HELPER(CAT)           { Reflex_ ##CAT ##_alignAddress, Reflex_ ##CAT ##_moveAddress, Reflex_ ##CAT ##_itemSize }

static const Reflex_Type_Helper REFLEX_HELPER[Reflex_Category_Length] = {
    REFLEX_HELPER(Primary),
    REFLEX_HELPER(Pointer),
    REFLEX_HELPER(Array),
    REFLEX_HELPER(PointerArray),
    REFLEX_HELPER(Array2D),
};

#if REFLEX_FORMAT_MODE_PARAM
Reflex_Result Reflex_Param_scanRaw(Reflex* reflex, void* obj, Reflex_OnFieldFn onField) {
    Reflex_Result result = REFLEX_OK;
    uint8_t* pobj = (uint8_t*) obj;
    const Reflex_Type_Helper* helper;
    const Reflex_TypeParams* fmt = reflex->Schema->Fmt;
    Reflex_LenType len = reflex->Schema->Len;
    __isCustom_init(reflex->Schema);
    __biggestField_init();
    reflex->VariableIndex = 0;
    reflex->Obj = obj;

    while (len-- > 0 && result == REFLEX_OK) {
        // start check complex type
        __startCheckComplexType(fmt);
        helper = &REFLEX_HELPER[fmt->Fields.Category];
        // check align
        pobj = helper->alignAddress(pobj, fmt);
        // onField
        if (onField) {
            result = onField(reflex, pobj, fmt);
        }
        // move pobj
        pobj = helper->moveAddress(pobj, fmt);
        reflex->VariableIndex++;
        // end check complex type
        __endCheckComplexType(fmt, helper);
        // next fmt
        __nextFmt(reflex->Schema, fmt);
    }
    __alignObject();

    return result;
}

Reflex_Result Reflex_Param_scan(Reflex* reflex, void* obj) {
    return Reflex_Param_scanRaw(reflex, obj, REFLEX_ON_FIELD_FNS[reflex->FunctionMode]);
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
Reflex_Result Reflex_Primary_scanRaw(Reflex* reflex, void* obj, Reflex_OnFieldFn onField) {
    Reflex_Result result = REFLEX_OK;
    Reflex_TypeParams fmt = {0};
    uint8_t* pobj = (uint8_t*) obj;
    const Reflex_Type_Helper* helper;
    const uint8_t* pfmt = reflex->Schema->PrimaryFmt;
    __biggestField_init();
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
        __updateBiggestField(PRIMARY_TYPE_SIZE[fmt.Fields.Primary]);
        reflex->VariableIndex++;
        // move pobj
        pobj = helper->moveAddress(pobj, &fmt);
    }
    __alignObject();

    return result;
}

Reflex_Result Reflex_Primary_scan(Reflex* reflex, void* obj) {
    return Reflex_Primary_scanRaw(reflex, obj, REFLEX_ON_FIELD_FNS[reflex->FunctionMode]);
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
Reflex_Result Reflex_Offset_scanRaw(Reflex* reflex, void* obj, Reflex_OnFieldFn onField) {
    Reflex_Result result = REFLEX_OK;
    uint8_t* pobj = (uint8_t*) obj;
    const Reflex_TypeParams* fmt = reflex->Schema->Fmt;
    Reflex_LenType len = reflex->Schema->Len;
    __isCustom_init(reflex->Schema);
    __biggestField_init();
    reflex->VariableIndex = 0;
    reflex->Obj = obj;

    while (len-- > 0 && result == REFLEX_OK) {
        // strat check complex type
        __startCheckComplexType(fmt);
        // check align
        pobj = (uint8_t*) obj + fmt->Offset;
        // onField
        if (onField) {
            result = onField(reflex, pobj, fmt);
        }
        reflex->VariableIndex++;
        // end check complex type
        __endCheckComplexType(fmt, &REFLEX_HELPER[fmt->Fields.Category]);
        // next fmt
        __nextFmt(reflex->Schema, fmt);
    }
    __alignObject();

    return result;
}

Reflex_Result Reflex_Offset_scan(Reflex* reflex, void* obj) {
    return Reflex_Offset_scanRaw(reflex, obj, REFLEX_ON_FIELD_FNS[reflex->FunctionMode]);
}

Reflex_LenType Reflex_Offset_sizeNormal(const Reflex_Schema* schema) {
    uint8_t* pobj = (uint8_t*) 0;
    const Reflex_TypeParams* fmt = schema->Fmt;
    Reflex_LenType len = schema->Len;
    Reflex_LenType objsize = 0;
    const Reflex_TypeParams* maxOffset = fmt;
    __isCustom_init(schema);

    while (len-- > 0) {
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
    pobj = (uint8_t*) (maxOffset->Offset + REFLEX_HELPER[maxOffset->Fields.Category].itemSize(maxOffset));
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

#if REFLEX_SUPPORT_COMPLEX_TYPE

static Reflex_Result Reflex_Complex_Begin_Driver(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt) {
    return reflex->Driver->Category[fmt->Fields.Category]->fnComplexBegin(reflex, obj, fmt);
}
static Reflex_Result Reflex_Complex_Begin_Compact(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt) {
    return reflex->CompactFns->fnComplexBegin(reflex, obj, fmt);
}
static Reflex_Result Reflex_Complex_End_Driver(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt) {
    return reflex->Driver->Category[fmt->Fields.Category]->fnComplexEnd(reflex, obj, fmt);
}
static Reflex_Result Reflex_Complex_End_Compact(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt) {
    return reflex->CompactFns->fnComplexEnd(reflex, obj, fmt);
}

static const Reflex_OnFieldFn REFLEX_COMPLEX_BEGIN[] = {
    Reflex_scan_Callback,
    Reflex_Complex_Begin_Driver,
    Reflex_Complex_Begin_Compact,
};

static const Reflex_OnFieldFn REFLEX_COMPLEX_END[] = {
    Reflex_scan_Callback,
    Reflex_Complex_End_Driver,
    Reflex_Complex_End_Compact,
};

static Reflex_Result Reflex_Complex_Primary_scan(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt ,Reflex_OnFieldFn onField) {
    Reflex_Result res;
    const Reflex_Schema* tmpSchema = reflex->Schema;

    REFLEX_COMPLEX_BEGIN[reflex->FunctionMode](reflex, obj, fmt);

    reflex->VariableIndexOffset += reflex->VariableIndex;
    reflex->Schema = fmt->Schema;
    res = REFLEX_SCAN[(uint8_t) reflex->Schema->FormatMode](reflex, obj, onField);
    reflex->Schema = tmpSchema;

    REFLEX_COMPLEX_END[reflex->FunctionMode](reflex, obj, fmt);

    return res;
}

static Reflex_Result Reflex_Complex_Pointer_scan(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, Reflex_OnFieldFn onField) {
    Reflex_Result res;
    const Reflex_Schema* tmpSchema = reflex->Schema;

    REFLEX_COMPLEX_BEGIN[reflex->FunctionMode](reflex, obj, fmt);

    reflex->VariableIndexOffset += reflex->VariableIndex;
    reflex->Schema = fmt->Schema;
    res = REFLEX_SCAN[(uint8_t) reflex->Schema->FormatMode](reflex, *(void**) obj, onField);
    reflex->Schema = tmpSchema;
    reflex->Obj = obj;
    // move obj
    reflex->PObj = Reflex_Pointer_moveAddress(obj, fmt);

    REFLEX_COMPLEX_END[reflex->FunctionMode](reflex, obj, fmt);

    return res;
}

static Reflex_Result Reflex_Complex_Array_scan(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, Reflex_OnFieldFn onField) {
    Reflex_Result res = REFLEX_OK;
    Reflex_LenType len = fmt->Len;

    REFLEX_COMPLEX_BEGIN[reflex->FunctionMode](reflex, obj, fmt);

    while (len-- > 0 && res == REFLEX_OK) {
        res = Reflex_Complex_Primary_scan(reflex, obj, fmt, onField);
        obj = reflex->PObj;
    }

    REFLEX_COMPLEX_END[reflex->FunctionMode](reflex, obj, fmt);

    return res;
}

static Reflex_Result Reflex_Complex_PointerArray_scan(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, Reflex_OnFieldFn onField) {
    Reflex_Result res = REFLEX_OK;
    Reflex_LenType len = fmt->Len;

    REFLEX_COMPLEX_BEGIN[reflex->FunctionMode](reflex, obj, fmt);

    while (len-- > 0 && res == REFLEX_OK) {
        res = Reflex_Complex_Pointer_scan(reflex, obj, fmt, onField);
        obj = reflex->PObj;
    }

    REFLEX_COMPLEX_END[reflex->FunctionMode](reflex, obj, fmt);

    return res;
}

static Reflex_Result Reflex_Complex_Array2D_scan(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, Reflex_OnFieldFn onField) {
    Reflex_Result res = REFLEX_OK;
    Reflex_LenType len;
    Reflex_LenType mlen = fmt->MLen;

    REFLEX_COMPLEX_BEGIN[reflex->FunctionMode](reflex, obj, fmt);

    while (mlen-- > 0 && res == REFLEX_OK) {
        len = fmt->Len;
        while (len-- > 0 && res == REFLEX_OK) {
            res = Reflex_Complex_Primary_scan(reflex, obj, fmt, onField);
            obj = reflex->PObj;
        }
    }

    REFLEX_COMPLEX_END[reflex->FunctionMode](reflex, obj, fmt);

    return res;
}

static const Reflex_ComplexScanFn REFLEX_COMPLEX[] = {
    Reflex_Complex_Primary_scan,
    Reflex_Complex_Pointer_scan,
    Reflex_Complex_Array_scan,
    Reflex_Complex_PointerArray_scan,
    Reflex_Complex_Array2D_scan,
};

Reflex_Result Reflex_Complex_scanRaw(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, Reflex_OnFieldFn onField) {
    return REFLEX_COMPLEX[fmt->Fields.Category](reflex, obj, fmt, onField);
}

Reflex_Result Reflex_Complex_scan(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, Reflex_OnFieldFn onField) {
    return Reflex_Complex_scanRaw(reflex, obj, fmt, REFLEX_ON_FIELD_FNS[reflex->FunctionMode]);
}
#endif

Reflex_Result Reflex_scanRaw(Reflex* reflex, void* obj, Reflex_OnFieldFn onField) {
#if REFLEX_SUPPORT_COMPLEX_TYPE
    reflex->PObj = obj;
    reflex->AlignSize = 0;
    reflex->VariableIndexOffset = 0;
#endif
    return REFLEX_SCAN[(uint8_t) reflex->Schema->FormatMode](reflex, obj, onField);
}

Reflex_Result Reflex_scan(Reflex* reflex, void* obj) {
    return Reflex_scanRaw(reflex, obj, REFLEX_ON_FIELD_FNS[reflex->FunctionMode]);
}

Reflex_LenType Reflex_size(const Reflex_Schema* schema, Reflex_SizeType type) {
    return REFLEX_SIZE[schema->FormatMode][type](schema);
}

Reflex_LenType Reflex_sizeType(const Reflex_TypeParams* fmt) {
    return (Reflex_LenType) REFLEX_HELPER[fmt->Fields.Category].moveAddress((void*) 0, fmt);
}

void Reflex_setCallback(Reflex* reflex, Reflex_OnFieldFn fn) {
    reflex->onField = fn;
    reflex->FunctionMode = Reflex_FunctionMode_Callback;
}
void Reflex_setDriver(Reflex* reflex, const Reflex_ScanDriver* driver) {
    reflex->Driver = driver;
    reflex->FunctionMode = Reflex_FunctionMode_Driver;
}
void Reflex_setCompact(Reflex* reflex, const Reflex_ScanFunctions* compact) {
    reflex->CompactFns = compact;
    reflex->FunctionMode = Reflex_FunctionMode_Compact;
}

Reflex_LenType Reflex_getVariableIndex(Reflex* reflex) {
#if REFLEX_SUPPORT_COMPLEX_TYPE
    return reflex->VariableIndex + reflex->VariableIndexOffset;
#else
    return reflex->VariableIndex;
#endif
}
Reflex_LenType Reflex_getVariablesLength(Reflex* reflex) {
    return reflex->Schema->Len;
}
void* Reflex_getMainVariable(Reflex* reflex) {
    return reflex->Obj;
}

#if REFLEX_SUPPORT_ARGS
void  Reflex_setArgs(Reflex* reflex, void* args) {
    reflex->Args = args;
}
void* Reflex_getArgs(Reflex* reflex) {
    return reflex->Args;
}
#endif

#if REFLEX_SUPPORT_BUFFER
void  Reflex_setBuffer(Reflex* reflex, void* buf) {
    reflex->Buffer = buf;
}
void* Reflex_getBuffer(Reflex* reflex) {
    return reflex->Buffer;
}
#endif

static Reflex_Result Reflex_scan_Callback(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt) {
    return reflex->onField(reflex, obj, fmt);
}
static Reflex_Result Reflex_scan_Driver(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt) {
    return reflex->Driver->Category[fmt->Fields.Category]->fn[fmt->Fields.Primary](reflex, obj, fmt);
}
static Reflex_Result Reflex_scan_Compact(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt) {
    return reflex->CompactFns->fn[fmt->Fields.Primary](reflex, obj, fmt);
}
