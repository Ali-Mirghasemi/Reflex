#include "Reflex.h"

/* Private typedef */
typedef void* (*Reflex_AlignAddressFn)(void* pValue, const Reflex_TypeParams* fmt);
typedef void* (*Reflex_MoveAddressFn)(void* pValue, const Reflex_TypeParams* fmt);
typedef Reflex_LenType (*Reflex_ItemSizeFn)(const Reflex_TypeParams* fmt);
typedef Reflex_Result (*Reflex_ScanFn)(Reflex* reflex, void* obj, Reflex_OnFieldFn onField);
typedef Reflex_GetResult (*Reflex_GetFieldFn)(Reflex* reflex, void* obj, const Reflex_TypeParams* filedFmt, Reflex_Field* field);
typedef Reflex_Result (*Reflex_ComplexScanFn)(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, Reflex_OnFieldFn onField);
typedef Reflex_GetResult (*Reflex_ComplexGetFieldFn)(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, const void* filedFmt, Reflex_Field* field);
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

#if REFLEX_SUPPORT_TYPE_COMPLEX
    #define __biggestField_init()               Reflex_LenType biggestField = 1, tmpSize
    #define __biggestField_init_NoTemp()        Reflex_LenType biggestField = 1
    #define __updateBiggestField(SIZE)          if (biggestField < (SIZE)) { \
                                                    biggestField = (SIZE); \
                                                }
    #define __alignObject()                     reflex->PObj = Reflex_alignAddress(pobj, biggestField); \
                                                if (reflex->AlignSize < biggestField) { \
                                                    reflex->AlignSize = biggestField; \
                                                }


    #define __startCheckComplexType(CB, ...)    if (fmt->Fields.Primary == Reflex_PrimaryType_Complex) { \
                                                    result = CB(reflex, pobj, __VA_ARGS__); \
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
    #define __startCheckComplexType(CB, ...)
    #define __endCheckComplexType(fmt, helper)
#endif

#if REFLEX_SUPPORT_BREAK_LAYER && REFLEX_SUPPORT_TYPE_COMPLEX
    #define __breakScanArray(reflex)            if (reflex->BreakLayer) { \
                                                    reflex->BreakLayer = 0; \
                                                    break; \
                                                }
    #define __breakScanArray2D(reflex)          if (reflex->BreakLayer2D) { \
                                                    reflex->BreakLayer = 0; \
                                                    reflex->BreakLayer2D = 0; \
                                                    break; \
                                                }                                                
#else
    #define __breakScanArray(reflex)
    #define __breakScanArray2D(reflex)
#endif

#if REFLEX_SUPPORT_VAR_INDEX
    #define __resetVarIndex(reflex)             reflex->VariableIndex = 0
    #define __increaseVarIndex(reflex)          reflex->VariableIndex++
    #define __updateOffsetIndex(reflex)         reflex->VariableIndexOffset += reflex->VariableIndex
    #define __increaseLayerIndex(reflex)        reflex->LayerIndex++
    #define __decreaseLayerIndex(reflex)        reflex->LayerIndex--
#else
    #define __resetVarIndex(reflex)
    #define __increaseVarIndex(reflex)
    #define __updateOffsetIndex(reflex)
    #define __increaseLayerIndex(reflex)
    #define __decreaseLayerIndex(reflex)
#endif

#if   REFLEX_ARCH == REFLEX_ARCH_64BIT
    typedef uint64_t Reflex_PtrType;
#elif REFLEX_ARCH == REFLEX_ARCH_32BIT
    typedef uint32_t Reflex_PtrType;
#else
    typedef uint16_t Reflex_PtrType;
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

static const Reflex_GetFieldFn REFLEX_GET_FIELD[] = {
#if REFLEX_FORMAT_MODE_PARAM
    Reflex_Param_getField,
#endif
#if REFLEX_FORMAT_MODE_PRIMARY
    Reflex_Primary_getField,
#endif
#if REFLEX_FORMAT_MODE_OFFSET
    Reflex_Offset_getField,
#endif
};

static const Reflex_OnFieldFn REFLEX_ON_FIELD_FNS[3] = {
    Reflex_scan_Callback,
    Reflex_scan_Driver,
    Reflex_scan_Compact,
};

#if REFLEX_SUPPORT_SIZE_FN
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
#endif // REFLEX_SUPPORT_SIZE_FN

#if REFLEX_ARCH == REFLEX_ARCH_8BIT
    #define Reflex_alignAddress(pValue, objSize)        pValue
#else
static void* Reflex_alignAddress(void* pValue, Reflex_LenType objSize) {
    Reflex_PtrType pobj = (Reflex_PtrType) pValue;
    Reflex_PtrType pad;

#if REFLEX_ARCH != REFLEX_ARCH_64BIT
    if (objSize > REFLEX_ARCH_BYTES) {
        objSize = REFLEX_ARCH_BYTES;
    }
#endif

    if ((pad = pobj & ((Reflex_PtrType) objSize - 1)) != 0) {
        pobj += objSize - pad;
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
    __resetVarIndex(reflex);
    reflex->Obj = obj;

    while (len-- > 0 && result == REFLEX_OK) {
        // start check complex type
        __startCheckComplexType(Reflex_Complex_scanRaw, fmt, onField);
        helper = &REFLEX_HELPER[fmt->Fields.Category];
        // check align
        pobj = helper->alignAddress(pobj, fmt);
        // onField
        if (onField) {
            result = onField(reflex, pobj, fmt);
        }
        // move pobj
        pobj = helper->moveAddress(pobj, fmt);
        __increaseVarIndex(reflex);
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

Reflex_GetResult Reflex_Param_getField(Reflex* reflex, void* obj, const void* fieldFmt, Reflex_Field* field) {
    Reflex_GetResult result = Reflex_GetResult_NotFound;
    uint8_t* pobj = (uint8_t*) obj;
    const Reflex_Type_Helper* helper;
    const Reflex_TypeParams* fmt = reflex->Schema->Fmt;
    Reflex_LenType len = reflex->Schema->Len;
    __isCustom_init(reflex->Schema);
    __biggestField_init();
    __resetVarIndex(reflex);
    reflex->Obj = obj;

    while (len-- > 0 && result != Reflex_GetResult_Ok) {
        // start check complex type
        __startCheckComplexType(Reflex_Complex_getField, fmt, fieldFmt, field);
        helper = &REFLEX_HELPER[fmt->Fields.Category];
        // check align
        pobj = helper->alignAddress(pobj, fmt);
        // check field
        if (fmt == fieldFmt) {
            // fill field
            field->Object = pobj;
            field->Fmt = fmt;
        #if REFLEX_SUPPORT_VAR_INDEX
            field->Index = reflex->VariableIndex;
        #if REFLEX_SUPPORT_TYPE_COMPLEX
            field->IndexOffset = reflex->VariableIndexOffset;
            field->LayerIndex = reflex->LayerIndex;
        #endif
        #endif // REFLEX_SUPPORT_VAR_INDEX
            return Reflex_GetResult_Ok;
        }
        // move pobj
        pobj = helper->moveAddress(pobj, fmt);
        __increaseVarIndex(reflex);
        // end check complex type
        __endCheckComplexType(fmt, helper);
        // next fmt
        __nextFmt(reflex->Schema, fmt);
    }
    __alignObject();

    return result;
}

#endif // REFLEX_FORMAT_MODE_PARAM

#if REFLEX_FORMAT_MODE_PRIMARY
Reflex_Result Reflex_Primary_scanRaw(Reflex* reflex, void* obj, Reflex_OnFieldFn onField) {
    Reflex_Result result = REFLEX_OK;
    Reflex_TypeParams fmt = {0};
    uint8_t* pobj = (uint8_t*) obj;
    const Reflex_Type_Helper* helper;
    const uint8_t* pfmt = reflex->Schema->PrimaryFmt;
    __biggestField_init_NoTemp();
    __resetVarIndex(reflex);
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
        __increaseVarIndex(reflex);
        // move pobj
        pobj = helper->moveAddress(pobj, &fmt);
    }
    __alignObject();

    return result;
}

Reflex_Result Reflex_Primary_scan(Reflex* reflex, void* obj) {
    return Reflex_Primary_scanRaw(reflex, obj, REFLEX_ON_FIELD_FNS[reflex->FunctionMode]);
}

Reflex_GetResult Reflex_Primary_getField(Reflex* reflex, void* obj, const void* fieldFmt, Reflex_Field* field) {
    Reflex_TypeParams fmt = {0};
    uint8_t* pobj = (uint8_t*) obj;
    const Reflex_Type_Helper* helper;
    const uint8_t* pfmt = reflex->Schema->PrimaryFmt;
    __biggestField_init_NoTemp();
    __resetVarIndex(reflex);
    reflex->Obj = obj;

    while (*pfmt != Reflex_Type_Unknown) {
        fmt.Type = *pfmt;
        helper = &REFLEX_HELPER[fmt.Fields.Category];
        // check align
        pobj = helper->alignAddress(pobj, &fmt);
        // check field
        if (pfmt == fieldFmt) {
            // fill field
            field->Object = pobj;
            field->PrimaryFmt = pfmt;
        #if REFLEX_SUPPORT_VAR_INDEX
            field->Index = reflex->VariableIndex;
        #if REFLEX_SUPPORT_TYPE_COMPLEX
            field->IndexOffset = reflex->VariableIndexOffset;
            field->LayerIndex = reflex->LayerIndex;
        #endif
        #endif // REFLEX_SUPPORT_VAR_INDEX
            return Reflex_GetResult_Ok;
        }
        __updateBiggestField(PRIMARY_TYPE_SIZE[fmt.Fields.Primary]);
        __increaseVarIndex(reflex);
        // move pobj
        pobj = helper->moveAddress(pobj, &fmt);
        pfmt++;
    }
    __alignObject();

    return Reflex_GetResult_NotFound;
}

Reflex_Result Reflex_Param_scanFieldRaw(Reflex* reflex, void* obj, const void* field, Reflex_OnFieldFn onField) {
    Reflex_Result result = REFLEX_OK;
    Reflex_TypeParams fmt = {0};
    uint8_t* pobj = (uint8_t*) obj;
    const Reflex_Type_Helper* helper;
    const uint8_t* pfmt = reflex->Schema->PrimaryFmt;
    __biggestField_init_NoTemp();
    __resetVarIndex(reflex);
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
        __increaseVarIndex(reflex);
        // move pobj
        pobj = helper->moveAddress(pobj, &fmt);
    }
    __alignObject();

    return result;
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
    __resetVarIndex(reflex);
    reflex->Obj = obj;

    while (len-- > 0 && result == REFLEX_OK) {
        // strat check complex type
        __startCheckComplexType(Reflex_Complex_scanRaw, fmt, onField);
        // check align
        pobj = (uint8_t*) obj + fmt->Offset;
        // onField
        if (onField) {
            result = onField(reflex, pobj, fmt);
        }
        __increaseVarIndex(reflex);
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

Reflex_GetResult Reflex_Offset_getField(Reflex* reflex, void* obj, const void* fieldFmt, Reflex_Field* field) {
    Reflex_GetResult result = Reflex_GetResult_NotFound;
    uint8_t* pobj = (uint8_t*) obj;
    const Reflex_TypeParams* fmt = reflex->Schema->Fmt;
    Reflex_LenType len = reflex->Schema->Len;
    __isCustom_init(reflex->Schema);
    __biggestField_init();
    __resetVarIndex(reflex);
    reflex->Obj = obj;

    while (len-- > 0 && result != Reflex_GetResult_Ok) {
        // strat check complex type
        __startCheckComplexType(Reflex_Complex_getField, fmt, fieldFmt, field);
        // check align
        pobj = (uint8_t*) obj + fmt->Offset;
        // check field
        if (fmt == fieldFmt) {
            // fill field
            field->Object = pobj;
            field->Fmt = fmt;
        #if REFLEX_SUPPORT_VAR_INDEX
            field->Index = reflex->VariableIndex;
        #if REFLEX_SUPPORT_TYPE_COMPLEX
            field->IndexOffset = reflex->VariableIndexOffset;
            field->LayerIndex = reflex->LayerIndex;
        #endif
        #endif // REFLEX_SUPPORT_VAR_INDEX
            return Reflex_GetResult_Ok;
        }
        __increaseVarIndex(reflex);
        // end check complex type
        __endCheckComplexType(fmt, &REFLEX_HELPER[fmt->Fields.Category]);
        // next fmt
        __nextFmt(reflex->Schema, fmt);
    }
    __alignObject();

    return result;
}

#endif // REFLEX_FORMAT_MODE_OFFSET

#if REFLEX_SUPPORT_TYPE_COMPLEX

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

    __updateOffsetIndex(reflex);
    reflex->Schema = fmt->Schema;
    res = REFLEX_SCAN[(uint8_t) reflex->Schema->FormatMode](reflex, obj, onField);
    reflex->Schema = tmpSchema;

    return res;
}

static Reflex_Result Reflex_Complex_Pointer_scan(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, Reflex_OnFieldFn onField) {
    Reflex_Result res;
    const Reflex_Schema* tmpSchema = reflex->Schema;

    __updateOffsetIndex(reflex);
    reflex->Schema = fmt->Schema;
    res = REFLEX_SCAN[(uint8_t) reflex->Schema->FormatMode](reflex, *(void**) obj, onField);
    reflex->Schema = tmpSchema;
    reflex->Obj = obj;
    // move obj
    reflex->PObj = Reflex_Pointer_moveAddress(obj, fmt);

    return res;
}

static Reflex_Result Reflex_Complex_Array_scan(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, Reflex_OnFieldFn onField) {
    Reflex_Result res = REFLEX_OK;
    Reflex_LenType len = fmt->Len;

    while (len-- > 0 && res == REFLEX_OK) {
        res = Reflex_Complex_Primary_scan(reflex, obj, fmt, onField);
        obj = reflex->PObj;
        __breakScanArray(reflex);
    }

    return res;
}

static Reflex_Result Reflex_Complex_PointerArray_scan(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, Reflex_OnFieldFn onField) {
    Reflex_Result res = REFLEX_OK;
    Reflex_LenType len = fmt->Len;

    while (len-- > 0 && res == REFLEX_OK) {
        res = Reflex_Complex_Pointer_scan(reflex, obj, fmt, onField);
        obj = reflex->PObj;
        __breakScanArray(reflex);
    }

    return res;
}

static Reflex_Result Reflex_Complex_Array2D_scan(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, Reflex_OnFieldFn onField) {
    Reflex_Result res = REFLEX_OK;
    Reflex_LenType len;
    Reflex_LenType mlen = fmt->MLen;

    while (mlen-- > 0 && res == REFLEX_OK) {
        len = fmt->Len;
        while (len-- > 0 && res == REFLEX_OK) {
            res = Reflex_Complex_Primary_scan(reflex, obj, fmt, onField);
            obj = reflex->PObj;
            __breakScanArray(reflex);
        }
        __breakScanArray2D(reflex);
    }

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
    Reflex_Result result;
    // Call begin callback
    REFLEX_COMPLEX_BEGIN[reflex->FunctionMode](reflex, obj, fmt);
    // Call scan call back for complex field
    __increaseLayerIndex(reflex);
    result = REFLEX_COMPLEX[fmt->Fields.Category](reflex, obj, fmt, onField);
    __decreaseLayerIndex(reflex);
    // Call end callback
    REFLEX_COMPLEX_END[reflex->FunctionMode](reflex, obj, fmt);

    return result;
}

#if REFLEX_SUPPORT_SCAN_FIELD

static Reflex_GetResult Reflex_Complex_Primary_getField(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, const void* fieldFmt, Reflex_Field* field) {
    Reflex_GetResult res;
    const Reflex_Schema* tmpSchema = reflex->Schema;
    Reflex_LenType lastIndex = reflex->VariableIndex;
    Reflex_LenType lastOffset = reflex->VariableIndexOffset;

    reflex->VariableIndexOffset += reflex->VariableIndex;
    reflex->Schema = fmt->Schema;
    res = REFLEX_GET_FIELD[(uint8_t) reflex->Schema->FormatMode](reflex, obj, fieldFmt, field);
    reflex->VariableIndexOffset = lastOffset + reflex->VariableIndex - lastIndex;
    reflex->VariableIndex = lastIndex;
    reflex->Schema = tmpSchema;

    return res;
}

static Reflex_GetResult Reflex_Complex_Pointer_getField(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, const void* fieldFmt, Reflex_Field* field) {
    Reflex_GetResult res;
    const Reflex_Schema* tmpSchema = reflex->Schema;
    Reflex_LenType lastIndex = reflex->VariableIndex;
    Reflex_LenType lastOffset = reflex->VariableIndexOffset;

    reflex->VariableIndexOffset += reflex->VariableIndex;
    reflex->Schema = fmt->Schema;
    res = REFLEX_GET_FIELD[(uint8_t) reflex->Schema->FormatMode](reflex, *(void**) obj, fieldFmt, field);
    reflex->Schema = tmpSchema;
    reflex->VariableIndexOffset = lastOffset + reflex->VariableIndex - lastIndex;
    reflex->VariableIndex = lastIndex;
    reflex->Obj = obj;
    // move obj
    reflex->PObj = Reflex_Pointer_moveAddress(obj, fmt);

    return res;
}

static Reflex_GetResult Reflex_Complex_Array_getField(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, const void* fieldFmt, Reflex_Field* field) {
    Reflex_GetResult res = Reflex_GetResult_NotFound;
    Reflex_LenType len = fmt->Len;

    while (len-- > 0 && res != Reflex_GetResult_Ok) {
        res = Reflex_Complex_Primary_getField(reflex, obj, fmt, fieldFmt, field);
        obj = reflex->PObj;
    #if REFLEX_SUPPORT_BREAK_LAYER
        if (reflex->BreakLayer) {
            reflex->BreakLayer = 0;
            break;
        }
    #endif
    }

    return res;
}

static Reflex_GetResult Reflex_Complex_PointerArray_getField(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, const void* fieldFmt, Reflex_Field* field) {
    Reflex_GetResult res = Reflex_GetResult_NotFound;
    Reflex_LenType len = fmt->Len;

    while (len-- > 0 && res != Reflex_GetResult_Ok) {
        res = Reflex_Complex_Pointer_getField(reflex, obj, fmt, fieldFmt, field);
        obj = reflex->PObj;
    #if REFLEX_SUPPORT_BREAK_LAYER
        if (reflex->BreakLayer) {
            reflex->BreakLayer = 0;
            break;
        }
    #endif
    }

    return res;
}

static Reflex_GetResult Reflex_Complex_Array2D_getField(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, const void* fieldFmt, Reflex_Field* field) {
    Reflex_GetResult res = Reflex_GetResult_NotFound;
    Reflex_LenType len;
    Reflex_LenType mlen = fmt->MLen;

    while (mlen-- > 0 && res != Reflex_GetResult_Ok) {
        len = fmt->Len;
        while (len-- > 0 && res != Reflex_GetResult_Ok) {
            res = Reflex_Complex_Primary_getField(reflex, obj, fmt, fieldFmt, field);
            obj = reflex->PObj;
        #if REFLEX_SUPPORT_BREAK_LAYER
            if (reflex->BreakLayer) {
                reflex->BreakLayer = 0;
                break;
            }
        #endif
        }
    }

    return res;
}

static const Reflex_ComplexGetFieldFn REFLEX_COMPLEX_GET_FIELD[] = {
    Reflex_Complex_Primary_getField,
    Reflex_Complex_Pointer_getField,
    Reflex_Complex_Array_getField,
    Reflex_Complex_PointerArray_getField,
    Reflex_Complex_Array2D_getField,
};

Reflex_GetResult Reflex_Complex_getField(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, const void* fieldFmt, Reflex_Field* field) {
    // check field
    if (fmt == fieldFmt) {
        // fill field
        field->Fmt = fmt;
        fmt = fmt->Schema->Fmt;
        field->Object = Reflex_alignAddress(obj, REFLEX_HELPER[fmt->Fields.Category].itemSize(fmt));
    #if REFLEX_SUPPORT_VAR_INDEX
        field->Index = reflex->VariableIndex;
    #if REFLEX_SUPPORT_TYPE_COMPLEX
        field->IndexOffset = reflex->VariableIndexOffset;
        field->LayerIndex = reflex->LayerIndex;
    #endif
    #endif //REFLEX_SUPPORT_VAR_INDEX
        return Reflex_GetResult_Ok;
    }
    else {
        Reflex_GetResult result;
        // Call getField
        __increaseLayerIndex(reflex);
        result = REFLEX_COMPLEX_GET_FIELD[fmt->Fields.Category](reflex, obj, fmt, fieldFmt, field);
        __decreaseLayerIndex(reflex);

        return result;
    }
}
#endif // REFLEX_SUPPORT_SCAN_FIELD

Reflex_Result Reflex_Complex_scan(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, Reflex_OnFieldFn onField) {
    return Reflex_Complex_scanRaw(reflex, obj, fmt, REFLEX_ON_FIELD_FNS[reflex->FunctionMode]);
}
#endif

Reflex_Result Reflex_scanRaw(Reflex* reflex, void* obj, Reflex_OnFieldFn onField) {
#if REFLEX_SUPPORT_TYPE_COMPLEX
    reflex->PObj = obj;
    reflex->AlignSize = 1;
#if REFLEX_SUPPORT_VAR_INDEX
    reflex->VariableIndexOffset = 0;
    reflex->LayerIndex = 0;
#endif 
#endif
    return REFLEX_SCAN[(uint8_t) reflex->Schema->FormatMode](reflex, obj, onField);
}

Reflex_Result Reflex_scan(Reflex* reflex, void* obj) {
    return Reflex_scanRaw(reflex, obj, REFLEX_ON_FIELD_FNS[reflex->FunctionMode]);
}

Reflex_LenType Reflex_sizeType(const Reflex_TypeParams* fmt) {
    return (Reflex_LenType) REFLEX_HELPER[fmt->Fields.Category].moveAddress((void*) 0, fmt);
}

Reflex_GetResult Reflex_getField(Reflex* reflex, void* obj, const void* fieldFmt, Reflex_Field* field) {
#if REFLEX_SUPPORT_TYPE_COMPLEX
    reflex->PObj = obj;
    reflex->AlignSize = 1;
#if REFLEX_SUPPORT_VAR_INDEX
    reflex->VariableIndexOffset = 0;
    reflex->LayerIndex = 0;
#endif
#endif
    return REFLEX_GET_FIELD[(uint8_t) reflex->Schema->FormatMode](reflex, obj, fieldFmt, field);

}

#if REFLEX_SUPPORT_SIZE_FN

#if REFLEX_FORMAT_MODE_PARAM

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

Reflex_LenType Reflex_size(const Reflex_Schema* schema, Reflex_SizeType type) {
    return REFLEX_SIZE[schema->FormatMode][type](schema);
}

#endif // REFLEX_SUPPORT_SIZE_FN

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
#if REFLEX_SUPPORT_VAR_INDEX
Reflex_LenType Reflex_getVariableIndex(Reflex* reflex) {
#if REFLEX_SUPPORT_TYPE_COMPLEX
    return reflex->VariableIndex + reflex->VariableIndexOffset;
#else
    return reflex->VariableIndex;
#endif
}
Reflex_LenType Reflex_getVariableIndexReal(Reflex* reflex) {
    return reflex->VariableIndex;
}
#if REFLEX_SUPPORT_TYPE_COMPLEX
Reflex_LenType Reflex_getVariableIndexOffset(Reflex* reflex) {
    return reflex->VariableIndexOffset;
}
#endif
#endif // REFLEX_SUPPORT_VAR_INDEX
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

#if REFLEX_SUPPORT_SCAN_FIELD
void*          Reflex_Field_getVariable(Reflex_Field* reflex) {
    return reflex->Object;
}
Reflex_LenType Reflex_Field_getIndex(Reflex_Field* reflex) {
#if REFLEX_SUPPORT_TYPE_COMPLEX
    return reflex->Index + reflex->IndexOffset;
#else
    return reflex->Index;
#endif
}
Reflex_LenType Reflex_Field_getIndexReal(Reflex_Field* reflex) {
    return reflex->Index;
}
#if REFLEX_SUPPORT_TYPE_COMPLEX
Reflex_LenType Reflex_Field_getLayerIndex(Reflex_Field* reflex) {
    return reflex->LayerIndex; 
}
Reflex_LenType Reflex_Field_getIndexOffset(Reflex_Field* reflex) {
    return reflex->IndexOffset;
}
#endif

#endif // REFLEX_SUPPORT_SCAN_FIELD

/**
 * @brief This function let you initialize reflex object
 *
 * @param reflex address of reflex object
 * @param schema address of schema
 */
void Reflex_init(Reflex* reflex, const Reflex_Schema* schema) {
    reflex->Schema = schema;
}
#if REFLEX_SUPPORT_BREAK_LAYER
/**
 * @brief This function help you to break scan for Complex arrays
 *
 * @param reflex
 */
void Reflex_break(Reflex* reflex) {
    reflex->BreakLayer = 1;
}
/**
 * @brief This function allows you to break scan for Complex Array2D 
 * 
 * @param reflex 
 */
void Reflex_break2D(Reflex* reflex) {
    reflex->BreakLayer = 1;
    reflex->BreakLayer2D = 1;
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
