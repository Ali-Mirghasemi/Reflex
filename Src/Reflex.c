#include "Reflex.h"

/* ------------------------------------ Private typedef ----------------------------------- */
typedef void* (*Reflex_AlignAddressFn)(void* pValue, const Reflex_TypeParams* fmt);
typedef void* (*Reflex_MoveAddressFn)(void* pValue, const Reflex_TypeParams* fmt);
typedef Reflex_LenType (*Reflex_ItemSizeFn)(const Reflex_TypeParams* fmt);
typedef Reflex_Result (*Reflex_ScanFn)(Reflex* reflex, void* obj, Reflex_OnFieldFn onField);
typedef Reflex_GetResult (*Reflex_GetFieldFn)(Reflex* reflex, void* obj, const void* filedFmt, Reflex_Field* field);
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
/* ------------------------------------ Private Functions -------------------------------- */
// Helper onField functions for scan
#if REFLEX_SUPPORT_CALLBACK
    static Reflex_Result  Reflex_scan_Callback(Reflex* reflex, void* obj, const void* fmt);
#endif
#if REFLEX_SUPPORT_DRIVER
    static Reflex_Result  Reflex_scan_Driver(Reflex* reflex, void* obj, const void* fmt);
#endif
#if REFLEX_SUPPORT_COMPACT
    static Reflex_Result  Reflex_scan_Compact(Reflex* reflex, void* obj, const void* fmt);
#endif
// Helper Functions for Align, Move, ItemSize
#if REFLEX_ARCH == REFLEX_ARCH_8BIT
    #define               Reflex_alignAddress(pValue, objSize)        pValue
#else
    static void*          Reflex_alignAddress(void* pValue, Reflex_LenType objSize);
#endif
    static Reflex_LenType Reflex_Primary_itemSize(const Reflex_TypeParams* fmt);
    static void*          Reflex_Primary_alignAddress(void* pValue, const Reflex_TypeParams* fmt);
    static void*          Reflex_Primary_moveAddress(void* pValue, const Reflex_TypeParams* fmt);
#if REFLEX_SUPPORT_TYPE_POINTER
    static Reflex_LenType Reflex_Pointer_itemSize(const Reflex_TypeParams* fmt);
    static void*          Reflex_Pointer_alignAddress(void* pValue, const Reflex_TypeParams* fmt);
    static void*          Reflex_Pointer_moveAddress(void* pValue, const Reflex_TypeParams* fmt);
#endif
#if REFLEX_SUPPORT_TYPE_ARRAY
    #define               Reflex_Array_itemSize             Reflex_Primary_itemSize
    #define               Reflex_Array_alignAddress         Reflex_Primary_alignAddress
    static void*          Reflex_Array_moveAddress(void* pValue, const Reflex_TypeParams* fmt);
#endif
#if REFLEX_SUPPORT_TYPE_POINTER_ARRAY
    #define               Reflex_PointerArray_itemSize      Reflex_Pointer_itemSize
    #define               Reflex_PointerArray_alignAddress  Reflex_Pointer_alignAddress
    static void*          Reflex_PointerArray_moveAddress(void* pValue, const Reflex_TypeParams* fmt);
#endif
#if REFLEX_SUPPORT_TYPE_ARRAY_2D
    #define               Reflex_Array2D_itemSize           Reflex_Primary_itemSize
    #define               Reflex_Array2D_alignAddress       Reflex_Primary_alignAddress
    static void*          Reflex_Array2D_moveAddress(void* pValue, const Reflex_TypeParams* fmt);
#endif
// Helper functions for scan over complex object
#if REFLEX_SUPPORT_TYPE_COMPLEX
    // Helper function for call scan functions for 3 modes
    #define              Reflex_Complex_Begin_Callback      Reflex_scan_Callback
    #define              Reflex_Complex_End_Callback        Reflex_scan_Callback
    static Reflex_Result Reflex_Complex_Begin_Driver(Reflex* reflex, void* obj, const void* fmt);
    static Reflex_Result Reflex_Complex_Begin_Compact(Reflex* reflex, void* obj, const void* fmt);
    static Reflex_Result Reflex_Complex_End_Driver(Reflex* reflex, void* obj, const void* fmt);
    static Reflex_Result Reflex_Complex_End_Compact(Reflex* reflex, void* obj, const void* fmt);
    static Reflex_Result Reflex_Complex_Primary_scan(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt ,Reflex_OnFieldFn onField);
#if REFLEX_SUPPORT_TYPE_POINTER
    static Reflex_Result Reflex_Complex_Pointer_scan(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, Reflex_OnFieldFn onField);
#endif
#if REFLEX_SUPPORT_TYPE_ARRAY
    static Reflex_Result Reflex_Complex_Array_scan(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, Reflex_OnFieldFn onField);
#endif
#if REFLEX_SUPPORT_TYPE_POINTER_ARRAY
    static Reflex_Result Reflex_Complex_PointerArray_scan(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, Reflex_OnFieldFn onField);
#endif
#if REFLEX_SUPPORT_TYPE_ARRAY_2D    
    static Reflex_Result Reflex_Complex_Array2D_scan(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, Reflex_OnFieldFn onField);
#endif
#if REFLEX_SUPPORT_SCAN_FIELD
    static Reflex_GetResult Reflex_Complex_Primary_getField(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, const void* fieldFmt, Reflex_Field* field);
#if REFLEX_SUPPORT_TYPE_POINTER
    static Reflex_GetResult Reflex_Complex_Pointer_getField(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, const void* fieldFmt, Reflex_Field* field);
#endif
#if REFLEX_SUPPORT_TYPE_ARRAY
    static Reflex_GetResult Reflex_Complex_Array_getField(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, const void* fieldFmt, Reflex_Field* field);
#endif
#if REFLEX_SUPPORT_TYPE_POINTER_ARRAY
    static Reflex_GetResult Reflex_Complex_PointerArray_getField(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, const void* fieldFmt, Reflex_Field* field);
#endif
#if REFLEX_SUPPORT_TYPE_ARRAY_2D    
    static Reflex_GetResult Reflex_Complex_Array2D_getField(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, const void* fieldFmt, Reflex_Field* field);
#endif
#endif // REFLEX_SUPPORT_SCAN_FIELD
#endif // REFLEX_SUPPORT_TYPE_COMPLEX
/* ---------------------------------------- Private Variables ------------------------------------- */
// Store sizeof all primary types for accessing faster
static const uint8_t PRIMARY_TYPE_SIZE[] = {
    sizeof(char),
    sizeof(uint8_t),
    sizeof(int8_t),
    sizeof(uint16_t),
    sizeof(int16_t),
    sizeof(uint32_t),
    sizeof(int32_t),
#if REFLEX_SUPPORT_TYPE_64BIT
    sizeof(uint64_t),
    sizeof(int64_t),
#endif
    sizeof(float),
#if REFLEX_SUPPORT_TYPE_DOUBLE
    sizeof(double),
#endif
};
// Array of all possible FunctionMode
static const Reflex_OnFieldFn REFLEX_ON_FIELD_FNS[3] = {
#if REFLEX_SUPPORT_CALLBACK
    Reflex_scan_Callback,
#endif
#if REFLEX_SUPPORT_DRIVER
    Reflex_scan_Driver,
#endif
#if REFLEX_SUPPORT_COMPACT
    Reflex_scan_Compact,
#endif
};
// Array of all scan function format mode, for faster accessing
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
// Array of all getField function 
#if REFLEX_SUPPORT_SCAN_FIELD
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
#endif // REFLEX_SUPPORT_SCAN_FIELD
#if REFLEX_SUPPORT_SIZE_FN
// Array of all size functions
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

#define REFLEX_HELPER(CAT)           { Reflex_ ##CAT ##_alignAddress, Reflex_ ##CAT ##_moveAddress, Reflex_ ##CAT ##_itemSize }
// Definations of helper functions
static const Reflex_Type_Helper REFLEX_HELPER[Reflex_Category_Length] = {
    REFLEX_HELPER(Primary),
#if REFLEX_SUPPORT_TYPE_POINTER
    REFLEX_HELPER(Pointer),
#endif
#if REFLEX_SUPPORT_TYPE_ARRAY
    REFLEX_HELPER(Array),
#endif
#if REFLEX_SUPPORT_TYPE_POINTER_ARRAY
    REFLEX_HELPER(PointerArray),
#endif
#if REFLEX_SUPPORT_TYPE_ARRAY_2D
    REFLEX_HELPER(Array2D),
#endif
};

#if REFLEX_SUPPORT_TYPE_COMPLEX
// Array of all complex begin callback
static const Reflex_OnFieldFn REFLEX_COMPLEX_BEGIN[] = {
#if REFLEX_SUPPORT_CALLBACK
    Reflex_Complex_Begin_Callback,
#endif
#if REFLEX_SUPPORT_DRIVER
    Reflex_Complex_Begin_Driver,
#endif
#if REFLEX_SUPPORT_COMPACT
    Reflex_Complex_Begin_Compact,
#endif
};
// Array of all complex end callback
static const Reflex_OnFieldFn REFLEX_COMPLEX_END[] = {
#if REFLEX_SUPPORT_CALLBACK
    Reflex_Complex_End_Callback,
#endif
#if REFLEX_SUPPORT_DRIVER
    Reflex_Complex_End_Driver,
#endif
#if REFLEX_SUPPORT_COMPACT
    Reflex_Complex_End_Compact,
#endif
};
// Array of scan complex categories
static const Reflex_ComplexScanFn REFLEX_COMPLEX[] = {
    Reflex_Complex_Primary_scan,
#if REFLEX_SUPPORT_TYPE_POINTER
    Reflex_Complex_Pointer_scan,
#endif
#if REFLEX_SUPPORT_TYPE_ARRAY
    Reflex_Complex_Array_scan,
#endif
#if REFLEX_SUPPORT_TYPE_POINTER_ARRAY
    Reflex_Complex_PointerArray_scan,
#endif
#if REFLEX_SUPPORT_TYPE_ARRAY_2D
    Reflex_Complex_Array2D_scan,
#endif
};
// Array of getField Complex categories
static const Reflex_ComplexGetFieldFn REFLEX_COMPLEX_GET_FIELD[] = {
    Reflex_Complex_Primary_getField,
#if REFLEX_SUPPORT_TYPE_POINTER
    Reflex_Complex_Pointer_getField,
#endif
#if REFLEX_SUPPORT_TYPE_ARRAY
    Reflex_Complex_Array_getField,
#endif
#if REFLEX_SUPPORT_TYPE_POINTER_ARRAY
    Reflex_Complex_PointerArray_getField,
#endif
#if REFLEX_SUPPORT_TYPE_ARRAY_2D
    Reflex_Complex_Array2D_getField,
#endif
};

#endif // REFLEX_SUPPORT_TYPE_COMPLEX

/* ------------------------------------------ Private Macro ----------------------------------- */
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

    #define __initPObj(reflex, pobj)            reflex->PObj = pobj

#if REFLEX_SUPPORT_VAR_INDEX
    #define __initComplexParams(reflex, obj)    reflex->PObj = obj; \
                                                reflex->AlignSize = 1; \
                                                reflex->VarOffset = 0; \
                                                reflex->LayerIndex = 0;
#else
    #define __initComplexParams(reflex, obj)    reflex->PObj = obj; \
                                                reflex->AlignSize = 1; 
#endif // REFLEX_SUPPORT_VAR_INDEX

#else
    #define __biggestField_init()
    #define __updateBiggestField(SIZE)
    #define __alignObject()
    #define __startCheckComplexType(CB, ...)
    #define __endCheckComplexType(fmt, helper)
    #define __initPObj(reflex, pobj)
    #define __initComplexParams(reflex, obj)
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
    #define __resetVarIndex(reflex)             reflex->VarIndex = 0
    #define __increaseVarIndex(reflex)          reflex->VarIndex++
    #define __increaseLayerIndex(reflex)        reflex->LayerIndex++
    #define __decreaseLayerIndex(reflex)        reflex->LayerIndex--
#if REFLEX_SUPPORT_TYPE_COMPLEX 
    #define __updateOffsetIndex(reflex)         reflex->VarOffset += reflex->VarIndex
    #define __initVarIndex(A, B)                (A)->VarIndex = (B)->VarIndex;\
                                                (A)->VarOffset = (B)->VarOffset
#else
    #define __initVarIndex(A, B)                (A)->VarIndex = (B)->VarIndex
#endif
#else
    #define __resetVarIndex(reflex)
    #define __increaseVarIndex(reflex)
    #define __updateOffsetIndex(reflex)
    #define __increaseLayerIndex(reflex)
    #define __decreaseLayerIndex(reflex)
    #define __initVarIndex(A, B)
#endif

#if REFLEX_SUPPORT_MAIN_OBJ
    #define __initMainObj(reflex, obj)          reflex->MainObject = obj
#else
    #define __initMainObj(reflex, obj)
#endif

/* -------------------------------------- Public Functions ---------------------------------- */
/* ----------------------------------------- Main API --------------------------------------- */
/**
 * @brief This function let you initialize reflex object
 *
 * @param reflex address of reflex object
 * @param schema address of schema
 */
void Reflex_init(Reflex* reflex, const Reflex_Schema* schema) {
    reflex->Schema = schema;
}
/**
 * @brief This function allows you to scan over all fields in object and run custom function
 * 
 * @param reflex 
 * @param obj 
 * @param onField 
 * @return Reflex_Result 
 */
Reflex_Result Reflex_scanRaw(Reflex* reflex, void* obj, Reflex_OnFieldFn onField) {
    __initComplexParams(reflex, obj);
    return REFLEX_SCAN[(uint8_t) reflex->Schema->FormatMode](reflex, obj, onField);
}
/**
 * @brief This function allows you to scan over all fields in object
 * 
 * @param reflex 
 * @param obj 
 * @return Reflex_Result 
 */
Reflex_Result Reflex_scan(Reflex* reflex, void* obj) {
    return Reflex_scanRaw(reflex, obj, REFLEX_ON_FIELD_FNS[reflex->FunctionMode]);
}
/**
 * @brief This function return numbers of fields in current schema
 * 
 * @param reflex 
 * @return Reflex_LenType 
 */
Reflex_LenType Reflex_getVariablesLength(Reflex* reflex) {
    return reflex->Schema->Len;
}
/**
 * @brief This function return size of given type
 * 
 * @param fmt 
 * @return Reflex_LenType 
 */
Reflex_LenType Reflex_sizeType(const Reflex_TypeParams* fmt) {
    return (Reflex_PtrType) REFLEX_HELPER[fmt->Fields.Category].moveAddress((void*) 0, fmt);
}
#if REFLEX_SUPPORT_MAIN_OBJ
/**
 * @brief This function returns address of parent object of current field
 * 
 * @param reflex 
 * @return void* 
 */
void* Reflex_getMainVariable(Reflex* reflex) {
    return reflex->MainObject;
}
#endif
#if REFLEX_SUPPORT_ARGS
/**
 * @brief This function allows you te set Args
 * 
 * @param reflex 
 * @param args 
 */
void  Reflex_setArgs(Reflex* reflex, void* args) {
    reflex->Args = args;
}
/**
 * @brief This function return Args
 * 
 * @param reflex 
 * @return void* 
 */
void* Reflex_getArgs(Reflex* reflex) {
    return reflex->Args;
}
#endif // REFLEX_SUPPORT_ARGS
#if REFLEX_SUPPORT_BUFFER
/**
 * @brief This function allows you to set buffer
 * 
 * @param reflex 
 * @param buf 
 */
void Reflex_setBuffer(Reflex* reflex, void* buf) {
    reflex->Buffer = buf;
}
/**
 * @brief This function return buffer
 * 
 * @param reflex 
 * @return void* 
 */
void* Reflex_getBuffer(Reflex* reflex) {
    return reflex->Buffer;
}
#endif // REFLEX_SUPPORT_BUFFER
/* -------------------------------------- FunctionMode API ---------------------------------------- */
#if REFLEX_SUPPORT_CALLBACK
/**
 * @brief This function set callback for scan
 * 
 * @param reflex 
 * @param fn 
 */
void Reflex_setCallback(Reflex* reflex, Reflex_OnFieldFn fn) {
    reflex->onField = fn;
    reflex->FunctionMode = Reflex_FunctionMode_Callback;
}
#endif
#if REFLEX_SUPPORT_DRIVER
/**
 * @brief This function set Driver for scan
 * 
 * @param reflex 
 * @param driver 
 */
void Reflex_setDriver(Reflex* reflex, const Reflex_ScanDriver* driver) {
    reflex->Driver = driver;
    reflex->FunctionMode = Reflex_FunctionMode_Driver;
}
#endif
#if REFLEX_SUPPORT_COMPACT
/**
 * @brief This function set Compact for scan
 * 
 * @param reflex 
 * @param compact 
 */
void Reflex_setCompact(Reflex* reflex, const Reflex_ScanFunctions* compact) {
    reflex->CompactFns = compact;
    reflex->FunctionMode = Reflex_FunctionMode_Compact;
}
#endif
/* ----------------------------------------- GetField API ---------------------------------------- */
#if REFLEX_SUPPORT_SCAN_FIELD
/**
 * @brief This function get properties of field
 * Note: This function just support flat Layer or complex objects that DON'T have duplicate type within
 * 
 * @param reflex 
 * @param obj 
 * @param fieldFmt 
 * @param field 
 * @return Reflex_GetResult 
 */
Reflex_GetResult Reflex_getField(Reflex* reflex, void* obj, const void* fieldFmt, Reflex_Field* field) {
    __initComplexParams(reflex, obj);
    return REFLEX_GET_FIELD[(uint8_t) reflex->Schema->FormatMode](reflex, obj, fieldFmt, field);
}
/**
 * @brief This function allows you to scan over just a single field
 * Note: This function just support flat Layer or complex objects that DON'T have duplicate type within
 * 
 * @param reflex 
 * @param field 
 * @param onField 
 * @return Reflex_Result 
 */
Reflex_Result Reflex_scanFieldRaw(Reflex* reflex, Reflex_Field* field, Reflex_OnFieldFn onField) {
    __initVarIndex(reflex, field);
    __initMainObj(reflex, field->MainObject);
    __initPObj(reflex, field->Object);
    return onField(reflex, field->Object, field->Fmt);
}
/**
 * @brief This function allows you to scan over just a single field
 * Note: This function just support flat Layer or complex objects that DON'T have duplicate type within
 * 
 * @param reflex 
 * @param obj 
 * @param fieldFmt 
 * @return Reflex_Result 
 */
Reflex_Result Reflex_scanField(Reflex* reflex, void* obj, const void* fieldFmt) {
    Reflex_Field field = {0};
    Reflex_Result result = REFLEX_ERROR;

    if (Reflex_getField(reflex, obj, fieldFmt, &field) == Reflex_GetResult_Ok) {
        result = Reflex_scanFieldRaw(reflex, &field, REFLEX_ON_FIELD_FNS[reflex->FunctionMode]);
    }

    return result;
}
#endif // REFLEX_SUPPORT_SCAN_FIELD
/* ------------------------------------------- VarIndex API --------------------------------------- */
#if REFLEX_SUPPORT_VAR_INDEX
/**
 * @brief This function return index of current field
 * Note: for complex types it's Offset + Index
 * 
 * @param reflex 
 * @return Reflex_LenType 
 */
Reflex_LenType Reflex_getVarIndex(Reflex* reflex) {
#if REFLEX_SUPPORT_TYPE_COMPLEX
    return reflex->VarIndex + reflex->VarOffset;
#else
    return reflex->VarIndex;
#endif
}
#if REFLEX_SUPPORT_TYPE_COMPLEX
/**
 * @brief This function return offset of field
 * 
 * @param reflex 
 * @return Reflex_LenType 
 */
Reflex_LenType Reflex_getVarOffset(Reflex* reflex) {
    return reflex->VarOffset;
}
/**
 * @brief This function return real index
 * 
 * @param reflex 
 * @return Reflex_LenType 
 */
Reflex_LenType Reflex_getVarIndexReal(Reflex* reflex) {
    return reflex->VarIndex;
}
#endif
#endif // REFLEX_SUPPORT_VAR_INDEX
/* -------------------------------------------- Break API ----------------------------------------- */
#if REFLEX_SUPPORT_BREAK_LAYER && REFLEX_SUPPORT_TYPE_COMPLEX && (REFLEX_SUPPORT_TYPE_ARRAY || REFLEX_SUPPORT_TYPE_ARRAY_2D || REFLEX_SUPPORT_TYPE_POINTER_ARRAY)
/**
 * @brief This function help you to break scan for Complex arrays
 *
 * @param reflex
 */
void Reflex_break(Reflex* reflex) {
    reflex->BreakLayer = 1;
}
#if REFLEX_SUPPORT_TYPE_ARRAY_2D
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
#endif // REFLEX_SUPPORT_BREAK_LAYER

/* ------------------------------------------- Field API ------------------------------------------ */
#if REFLEX_SUPPORT_SCAN_FIELD
/**
 * @brief This function return address of field
 * 
 * @param reflex 
 * @return void* 
 */
void* Reflex_Field_getVariable(Reflex_Field* reflex) {
    return reflex->Object;
}
#if REFLEX_SUPPORT_MAIN_OBJ
/**
 * @brief This function return address of parent object of current field
 * 
 * @param reflex 
 * @return void* 
 */
void* Reflex_Field_getMainVariable(Reflex_Field* reflex) {
    return reflex->MainObject;
}
#endif // REFLEX_SUPPORT_MAIN_OBJ
/**
 * @brief This function return index of field
 * Note: for complex object it's Offset + Index
 * 
 * @param reflex 
 * @return Reflex_LenType 
 */
Reflex_LenType Reflex_Field_getVarIndex(Reflex_Field* reflex) {
#if REFLEX_SUPPORT_TYPE_COMPLEX
    return reflex->VarIndex + reflex->VarOffset;
#else
    return reflex->VarIndex;
#endif
}
#if REFLEX_SUPPORT_TYPE_COMPLEX
/**
 * @brief This function return Offset of field
 * 
 * @param reflex 
 * @return Reflex_LenType 
 */
Reflex_LenType Reflex_Field_getVarOffset(Reflex_Field* reflex) {
    return reflex->VarOffset;
}
/**
 * @brief This function rerun real index
 * 
 * @param reflex 
 * @return Reflex_LenType 
 */
Reflex_LenType Reflex_Field_getVarIndexReal(Reflex_Field* reflex) {
    return reflex->VarIndex;
}
#endif
#endif // REFLEX_SUPPORT_SCAN_FIELD
/* ---------------------------------------- Size API ------------------------------------------ */
#if REFLEX_SUPPORT_SIZE_FN
/**
 * @brief This function calculate size of object in runtime
 * 
 * @param schema 
 * @param type 
 * @return Reflex_LenType 
 */
Reflex_LenType Reflex_size(const Reflex_Schema* schema, Reflex_SizeType type) {
    return REFLEX_SIZE[schema->FormatMode][type](schema);
}
#if REFLEX_FORMAT_MODE_PARAM
/**
 * @brief This function calculate size of Param object in runtime
 * 
 * @param schema 
 * @return Reflex_LenType 
 */
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

    return (Reflex_PtrType) pobj;
}
/**
 * @brief This function calculate size of Param object in runtime without paddings
 * 
 * @param schema 
 * @return Reflex_LenType 
 */
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
    return (Reflex_PtrType) pobj;
}
#endif // REFLEX_FORMAT_MODE_PARAM
#if REFLEX_FORMAT_MODE_PRIMARY
/**
 * @brief This function calculate size of Primary object in runtime
 * 
 * @param schema 
 * @return Reflex_LenType 
 */
Reflex_LenType Reflex_Primary_sizeNormal(const Reflex_Schema* schema) {
    Reflex_TypeParams fmt = {0};
    uint8_t* pobj = (uint8_t*) 0;
    const Reflex_Type_Helper* helper;
    const uint8_t* pfmt = schema->PrimaryFmt;
    Reflex_LenType len = schema->Len;
    Reflex_LenType objsize = 0;

    while (len-- > 0) {
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

    return (Reflex_PtrType) pobj;
}
/**
 * @brief This function calculate size of Primary object in runtime without paddings
 * 
 * @param schema 
 * @return Reflex_LenType 
 */
Reflex_LenType Reflex_Primary_sizePacked(const Reflex_Schema* schema) {
    Reflex_TypeParams fmt = {0};
    uint8_t* pobj = (uint8_t*) 0;
    const Reflex_Type_Helper* helper;
    const uint8_t* pfmt = schema->PrimaryFmt;
    Reflex_LenType len = schema->Len;

    while (len-- > 0) {
        fmt.Type = *pfmt++;
        helper = &REFLEX_HELPER[fmt.Fields.Category];
        // move pobj
        pobj = helper->moveAddress(pobj, &fmt);
    }
    return (Reflex_PtrType) pobj;
}
#endif // REFLEX_FORMAT_MODE_PRIMARY
#if REFLEX_FORMAT_MODE_OFFSET
/**
 * @brief This function calculate size of Offset object in runtime
 * 
 * @param schema 
 * @return Reflex_LenType 
 */
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
    pobj = (uint8_t*) ((Reflex_PtrType) maxOffset->Offset + REFLEX_HELPER[maxOffset->Fields.Category].itemSize(maxOffset));
    // align to biggest field
    pobj = Reflex_alignAddress(pobj, objsize);

    return (Reflex_PtrType) pobj;
}
/**
 * @brief This function calculate size of Offset object in runtime without paddings
 * 
 * @param schema 
 * @return Reflex_LenType 
 */
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
    return (Reflex_PtrType) pobj;
}
#endif // REFLEX_FORMAT_MODE_OFFSET
#endif // REFLEX_SUPPORT_SIZE_FN
/* ------------------------------------ LowLevel Scan API ------------------------------------- */
#if REFLEX_FORMAT_MODE_PARAM
/**
 * @brief This function scan over Param schema
 * 
 * @param reflex 
 * @param obj 
 * @param onField 
 * @return Reflex_Result 
 */
Reflex_Result Reflex_Param_scanRaw(Reflex* reflex, void* obj, Reflex_OnFieldFn onField) {
    Reflex_Result result = REFLEX_OK;
    uint8_t* pobj = (uint8_t*) obj;
    const Reflex_Type_Helper* helper;
    const Reflex_TypeParams* fmt = reflex->Schema->Fmt;
    Reflex_LenType len = reflex->Schema->Len;
    __isCustom_init(reflex->Schema);
    __biggestField_init();
    __resetVarIndex(reflex);
    __initMainObj(reflex, obj);

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
/**
 * @brief This function scan over Param schema
 * 
 * @param reflex 
 * @param obj 
 * @return Reflex_Result 
 */
Reflex_Result Reflex_Param_scan(Reflex* reflex, void* obj) {
    return Reflex_Param_scanRaw(reflex, obj, REFLEX_ON_FIELD_FNS[reflex->FunctionMode]);
}
#endif // REFLEX_FORMAT_MODE_PARAM
#if REFLEX_FORMAT_MODE_PRIMARY
/**
 * @brief This function scan over Primary schema
 * 
 * @param reflex 
 * @param obj 
 * @param onField 
 * @return Reflex_Result 
 */
Reflex_Result Reflex_Primary_scanRaw(Reflex* reflex, void* obj, Reflex_OnFieldFn onField) {
    Reflex_Result result = REFLEX_OK;
    Reflex_TypeParams fmt = {0};
    uint8_t* pobj = (uint8_t*) obj;
    Reflex_LenType len = reflex->Schema->Len;
    const Reflex_Type_Helper* helper;
    const uint8_t* pfmt = reflex->Schema->PrimaryFmt;
    __biggestField_init_NoTemp();
    __resetVarIndex(reflex);
    __initMainObj(reflex, obj);

    while (len-- > 0 && result == REFLEX_OK) {
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
/**
 * @brief This function scan over Primary schema
 * 
 * @param reflex 
 * @param obj 
 * @return Reflex_Result 
 */
Reflex_Result Reflex_Primary_scan(Reflex* reflex, void* obj) {
    return Reflex_Primary_scanRaw(reflex, obj, REFLEX_ON_FIELD_FNS[reflex->FunctionMode]);
}
#endif // REFLEX_FORMAT_MODE_PRIMARY
#if REFLEX_FORMAT_MODE_OFFSET
/**
 * @brief This function scan over Offset schema
 * 
 * @param reflex 
 * @param obj 
 * @param onField 
 * @return Reflex_Result 
 */
Reflex_Result Reflex_Offset_scanRaw(Reflex* reflex, void* obj, Reflex_OnFieldFn onField) {
    Reflex_Result result = REFLEX_OK;
    uint8_t* pobj = (uint8_t*) obj;
    const Reflex_TypeParams* fmt = reflex->Schema->Fmt;
    Reflex_LenType len = reflex->Schema->Len;
    __isCustom_init(reflex->Schema);
    __biggestField_init();
    __resetVarIndex(reflex);
    __initMainObj(reflex, obj);

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
/**
 * @brief This function scan over Offset schema
 * 
 * @param reflex 
 * @param obj 
 * @return Reflex_Result 
 */
Reflex_Result Reflex_Offset_scan(Reflex* reflex, void* obj) {
    return Reflex_Offset_scanRaw(reflex, obj, REFLEX_ON_FIELD_FNS[reflex->FunctionMode]);
}
#endif // REFLEX_FORMAT_MODE_OFFSET
#if REFLEX_SUPPORT_TYPE_COMPLEX
/**
 * @brief This function scan over Complex Field
 * 
 * @param reflex 
 * @param obj 
 * @param fmt 
 * @param onField 
 * @return Reflex_Result 
 */
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
/**
 * @brief This function scan over Complex field
 * 
 * @param reflex 
 * @param obj 
 * @param fmt 
 * @param onField 
 * @return Reflex_Result 
 */
Reflex_Result Reflex_Complex_scan(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, Reflex_OnFieldFn onField) {
    return Reflex_Complex_scanRaw(reflex, obj, fmt, REFLEX_ON_FIELD_FNS[reflex->FunctionMode]);
}
#endif // REFLEX_SUPPORT_TYPE_COMPLEX
/* -------------------------------- LowLevel GetField API ----------------------------------- */
#if REFLEX_SUPPORT_SCAN_FIELD
#if REFLEX_FORMAT_MODE_PARAM
/**
 * @brief This function find a field in Param schema
 * 
 * @param reflex 
 * @param obj 
 * @param fieldFmt 
 * @param field 
 * @return Reflex_GetResult 
 */
Reflex_GetResult Reflex_Param_getField(Reflex* reflex, void* obj, const void* fieldFmt, Reflex_Field* field) {
    Reflex_GetResult result = Reflex_GetResult_NotFound;
    uint8_t* pobj = (uint8_t*) obj;
    const Reflex_Type_Helper* helper;
    const Reflex_TypeParams* fmt = reflex->Schema->Fmt;
    Reflex_LenType len = reflex->Schema->Len;
    __isCustom_init(reflex->Schema);
    __biggestField_init();
    __resetVarIndex(reflex);
    __initMainObj(reflex, obj);

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
            __initMainObj(field, obj);
            __initVarIndex(field, reflex);
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
/**
 * @brief This function find a field in Primary schema
 * 
 * @param reflex 
 * @param obj 
 * @param fieldFmt 
 * @param field 
 * @return Reflex_GetResult 
 */
Reflex_GetResult Reflex_Primary_getField(Reflex* reflex, void* obj, const void* fieldFmt, Reflex_Field* field) {
    Reflex_TypeParams fmt = {0};
    uint8_t* pobj = (uint8_t*) obj;
    const Reflex_Type_Helper* helper;
    const uint8_t* pfmt = reflex->Schema->PrimaryFmt;
    Reflex_LenType len = reflex->Schema->Len;
    __biggestField_init_NoTemp();
    __resetVarIndex(reflex);
    __initMainObj(reflex, obj);

    while (len-- > 0) {
        fmt.Type = *pfmt;
        helper = &REFLEX_HELPER[fmt.Fields.Category];
        // check align
        pobj = helper->alignAddress(pobj, &fmt);
        // check field
        if (pfmt == fieldFmt) {
            // fill field
            field->Object = pobj;
            field->PrimaryFmt = pfmt;
            __initMainObj(field, obj);
            __initVarIndex(field, reflex);
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
#endif // REFLEX_FORMAT_MODE_PRIMARY
#if REFLEX_FORMAT_MODE_OFFSET
/**
 * @brief This function find a field in Offset schema
 * 
 * @param reflex 
 * @param obj 
 * @param fieldFmt 
 * @param field 
 * @return Reflex_GetResult 
 */
Reflex_GetResult Reflex_Offset_getField(Reflex* reflex, void* obj, const void* fieldFmt, Reflex_Field* field) {
    Reflex_GetResult result = Reflex_GetResult_NotFound;
    uint8_t* pobj = (uint8_t*) obj;
    const Reflex_TypeParams* fmt = reflex->Schema->Fmt;
    Reflex_LenType len = reflex->Schema->Len;
    __isCustom_init(reflex->Schema);
    __biggestField_init();
    __resetVarIndex(reflex);
    __initMainObj(reflex, obj);

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
            __initMainObj(field, obj);
            __initVarIndex(field, reflex);
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
/**
 * @brief This function find a field in Complex schema
 * 
 * @param reflex 
 * @param obj 
 * @param fmt 
 * @param fieldFmt 
 * @param field 
 * @return Reflex_GetResult 
 */
Reflex_GetResult Reflex_Complex_getField(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, const void* fieldFmt, Reflex_Field* field) {
    // check field
    if (fmt == fieldFmt) {
        // fill field
        field->Fmt = fmt;
        fmt = fmt->Schema->Fmt;
        field->Object = Reflex_alignAddress(obj, REFLEX_HELPER[fmt->Fields.Category].itemSize(fmt));
        __initMainObj(field, reflex->MainObject);
        __initVarIndex(field, reflex);
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
#endif // REFLEX_SUPPORT_TYPE_COMPLEX
#endif // REFLEX_SUPPORT_SCAN_FIELD
/* ---------------------- Private Functions (Implementation) ----------------------- */
#if REFLEX_ARCH != REFLEX_ARCH_8BIT
/**
 * @brief This function align address of field,
 * it is prevent hard fault
 * 
 * @param pValue 
 * @param objSize objSize must be 2^x, ex: 1, 2, 4, 8
 * @return void* 
 */
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
/**
 * @brief This function return item size
 * 
 * @param fmt 
 * @return Reflex_LenType 
 */
static Reflex_LenType Reflex_Primary_itemSize(const Reflex_TypeParams* fmt) {
    return PRIMARY_TYPE_SIZE[fmt->Fields.Primary];
}
/**
 * @brief This function align address of field
 * 
 * @param pValue 
 * @param fmt 
 * @return void* 
 */
static void* Reflex_Primary_alignAddress(void* pValue, const Reflex_TypeParams* fmt) {
    return Reflex_alignAddress(pValue, PRIMARY_TYPE_SIZE[fmt->Fields.Primary]);
}
/**
 * @brief This function move address of field based on field format
 * 
 * @param pValue 
 * @param fmt 
 * @return void* 
 */
static void* Reflex_Primary_moveAddress(void* pValue, const Reflex_TypeParams* fmt) {
    uint8_t objSize = PRIMARY_TYPE_SIZE[fmt->Fields.Primary];
    return (uint8_t*) pValue + objSize;
}
#if REFLEX_SUPPORT_TYPE_POINTER
/**
 * @brief This function return item size
 * 
 * @param fmt 
 * @return Reflex_LenType 
 */
static Reflex_LenType Reflex_Pointer_itemSize(const Reflex_TypeParams* fmt) {
    return sizeof(void*);
}
/**
 * @brief This function align address of pointer field
 * 
 * @param pValue 
 * @param fmt 
 * @return void* 
 */
static void* Reflex_Pointer_alignAddress(void* pValue, const Reflex_TypeParams* fmt) {
    return Reflex_alignAddress(pValue, sizeof(void*));
}
/**
 * @brief This function move address of field
 * 
 * @param pValue 
 * @param fmt 
 * @return void* 
 */
static void* Reflex_Pointer_moveAddress(void* pValue, const Reflex_TypeParams* fmt) {
    uint8_t objSize = sizeof(void*);
    return (uint8_t*) pValue + objSize;
}
#endif // REFLEX_SUPPORT_TYPE_POINTER
#if REFLEX_SUPPORT_TYPE_ARRAY
/**
 * @brief This function move address of array field
 * 
 * @param pValue 
 * @param fmt 
 * @return void* 
 */
static void* Reflex_Array_moveAddress(void* pValue, const Reflex_TypeParams* fmt) {
    uint8_t objSize = PRIMARY_TYPE_SIZE[fmt->Fields.Primary] * fmt->Len;
    return (uint8_t*) pValue + objSize;
}
#endif // REFLEX_SUPPORT_TYPE_ARRAY
#if REFLEX_SUPPORT_TYPE_POINTER_ARRAY
/**
 * @brief This function move address of pointer array field
 * 
 * @param pValue 
 * @param fmt 
 * @return void* 
 */
static void* Reflex_PointerArray_moveAddress(void* pValue, const Reflex_TypeParams* fmt) {
    uint8_t objSize = sizeof(void*) * fmt->Len;
    return (uint8_t*) pValue + objSize;
}
#endif // REFLEX_SUPPORT_TYPE_POINTER_ARRAY
#if REFLEX_SUPPORT_TYPE_ARRAY_2D
/**
 * @brief This function move address of 2D array field
 * 
 * @param pValue 
 * @param fmt 
 * @return void* 
 */
static void* Reflex_Array2D_moveAddress(void* pValue, const Reflex_TypeParams* fmt) {
    uint8_t objSize = PRIMARY_TYPE_SIZE[fmt->Fields.Primary] * fmt->Len * fmt->MLen;
    return (uint8_t*) pValue + objSize;
}
#endif
#if REFLEX_SUPPORT_TYPE_COMPLEX
#if REFLEX_SUPPORT_DRIVER
/**
 * @brief This is a helper function for call begin driver callback
 * 
 * @param reflex 
 * @param obj 
 * @param fmt 
 * @return Reflex_Result 
 */
static Reflex_Result Reflex_Complex_Begin_Driver(Reflex* reflex, void* obj, const void* fmt) {
    return reflex->Driver->Category[((const Reflex_TypeParams*) fmt)->Fields.Category]->fnComplexBegin(reflex, obj, fmt);
}
/**
 * @brief This is a helper function for call end driver callback
 * 
 * @param reflex 
 * @param obj 
 * @param fmt 
 * @return Reflex_Result 
 */
static Reflex_Result Reflex_Complex_End_Driver(Reflex* reflex, void* obj, const void* fmt) {
    return reflex->Driver->Category[((const Reflex_TypeParams*) fmt)->Fields.Category]->fnComplexEnd(reflex, obj, fmt);
}
#endif // REFLEX_SUPPORT_DRIVER
#if REFLEX_SUPPORT_COMPACT
/**
 * @brief This is a helper function for call begin compact callback
 * 
 * @param reflex 
 * @param obj 
 * @param fmt 
 * @return Reflex_Result 
 */
static Reflex_Result Reflex_Complex_Begin_Compact(Reflex* reflex, void* obj, const void* fmt) {
    return reflex->CompactFns->fnComplexBegin(reflex, obj, fmt);
}
/**
 * @brief This is a helper function for call end compact callback
 * 
 * @param reflex 
 * @param obj 
 * @param fmt 
 * @return Reflex_Result 
 */
static Reflex_Result Reflex_Complex_End_Compact(Reflex* reflex, void* obj, const void* fmt) {
    return reflex->CompactFns->fnComplexEnd(reflex, obj, fmt);
}
#endif // REFLEX_SUPPORT_COMPACT
/**
 * @brief This function run scan function over a complex field
 * 
 * @param reflex 
 * @param obj 
 * @param fmt 
 * @param onField 
 * @return Reflex_Result 
 */
static Reflex_Result Reflex_Complex_Primary_scan(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt ,Reflex_OnFieldFn onField) {
    Reflex_Result res;
    const Reflex_Schema* tmpSchema = reflex->Schema;

    __updateOffsetIndex(reflex);
    reflex->Schema = fmt->Schema;
    res = REFLEX_SCAN[(uint8_t) reflex->Schema->FormatMode](reflex, obj, onField);
    reflex->Schema = tmpSchema;

    return res;
}
#if REFLEX_SUPPORT_TYPE_POINTER
/**
 * @brief This function run scan function over a complex pointer field
 * 
 * @param reflex 
 * @param obj 
 * @param fmt 
 * @param onField 
 * @return Reflex_Result 
 */
static Reflex_Result Reflex_Complex_Pointer_scan(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, Reflex_OnFieldFn onField) {
    Reflex_Result res;
    const Reflex_Schema* tmpSchema = reflex->Schema;

    __updateOffsetIndex(reflex);
    reflex->Schema = fmt->Schema;
    res = REFLEX_SCAN[(uint8_t) reflex->Schema->FormatMode](reflex, *(void**) obj, onField);
    reflex->Schema = tmpSchema;
    __initMainObj(reflex, obj);
    // move obj
    reflex->PObj = Reflex_Pointer_moveAddress(obj, fmt);

    return res;
}
#endif // REFLEX_SUPPORT_TYPE_POINTER
#if REFLEX_SUPPORT_TYPE_ARRAY
/**
 * @brief This function run scan function over a complex array field
 * 
 * @param reflex 
 * @param obj 
 * @param fmt 
 * @param onField 
 * @return Reflex_Result 
 */
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
#endif // REFLEX_SUPPORT_TYPE_ARRAY
#if REFLEX_SUPPORT_TYPE_POINTER_ARRAY
/**
 * @brief This function run scan function over a complex pointer array field
 * 
 * @param reflex 
 * @param obj 
 * @param fmt 
 * @param onField 
 * @return Reflex_Result 
 */
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
#endif // REFLEX_SUPPORT_TYPE_POINTER_ARRAY
#if REFLEX_SUPPORT_TYPE_ARRAY_2D
/**
 * @brief This function run scan function over a complex 2D array field
 * 
 * @param reflex 
 * @param obj 
 * @param fmt 
 * @param onField 
 * @return Reflex_Result 
 */
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
#endif // REFLEX_SUPPORT_TYPE_ARRAY_2D
#if REFLEX_SUPPORT_SCAN_FIELD
/**
 * @brief This function find a field in complex object
 * 
 * @param reflex 
 * @param obj 
 * @param fmt 
 * @param fieldFmt 
 * @param field 
 * @return Reflex_GetResult 
 */
static Reflex_GetResult Reflex_Complex_Primary_getField(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, const void* fieldFmt, Reflex_Field* field) {
    Reflex_GetResult res;
    const Reflex_Schema* tmpSchema = reflex->Schema;

    __updateOffsetIndex(reflex);
    reflex->Schema = fmt->Schema;
    res = REFLEX_GET_FIELD[(uint8_t) reflex->Schema->FormatMode](reflex, obj, fieldFmt, field);
    reflex->Schema = tmpSchema;

    return res;
}
#if REFLEX_SUPPORT_TYPE_POINTER
/**
 * @brief This function find a field in complex pointer object
 * 
 * @param reflex 
 * @param obj 
 * @param fmt 
 * @param fieldFmt 
 * @param field 
 * @return Reflex_GetResult 
 */
static Reflex_GetResult Reflex_Complex_Pointer_getField(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, const void* fieldFmt, Reflex_Field* field) {
    Reflex_GetResult res;
    const Reflex_Schema* tmpSchema = reflex->Schema;

    __updateOffsetIndex(reflex);
    reflex->Schema = fmt->Schema;
    res = REFLEX_GET_FIELD[(uint8_t) reflex->Schema->FormatMode](reflex, *(void**) obj, fieldFmt, field);
    reflex->Schema = tmpSchema;
    __initMainObj(reflex, obj);
    // move obj
    reflex->PObj = Reflex_Pointer_moveAddress(obj, fmt);

    return res;
}
#endif // REFLEX_SUPPORT_TYPE_POINTER
#if REFLEX_SUPPORT_TYPE_ARRAY
/**
 * @brief This function find a field in complex array object
 * 
 * @param reflex 
 * @param obj 
 * @param fmt 
 * @param fieldFmt 
 * @param field 
 * @return Reflex_GetResult 
 */
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
#endif // REFLEX_SUPPORT_TYPE_ARRAY
#if REFLEX_SUPPORT_TYPE_POINTER_ARRAY
/**
 * @brief This function find a field in complex pointer array object
 * 
 * @param reflex 
 * @param obj 
 * @param fmt 
 * @param fieldFmt 
 * @param field 
 * @return Reflex_GetResult 
 */
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
#endif // REFLEX_SUPPORT_TYPE_POINTER_ARRAY
#if REFLEX_SUPPORT_TYPE_ARRAY_2D
/**
 * @brief This function find a field in complex 2D array object
 * 
 * @param reflex 
 * @param obj 
 * @param fmt 
 * @param fieldFmt 
 * @param field 
 * @return Reflex_GetResult 
 */
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
#endif //REFLEX_SUPPORT_TYPE_ARRAY_2D
#endif // REFLEX_SUPPORT_SCAN_FIELD
#endif // REFLEX_SUPPORT_TYPE_COMPLEX
#if REFLEX_SUPPORT_CALLBACK
/**
 * @brief This is a helper function that call onField callback
 * 
 * @param reflex 
 * @param obj 
 * @param fmt 
 * @return Reflex_Result 
 */
static Reflex_Result Reflex_scan_Callback(Reflex* reflex, void* obj, const void* fmt) {
    return reflex->onField(reflex, obj, fmt);
}
#endif // REFLEX_SUPPORT_CALLBACK
#if REFLEX_SUPPORT_DRIVER
/**
 * @brief This is a helper function that call callback of Driver
 * 
 * @param reflex 
 * @param obj 
 * @param fmt 
 * @return Reflex_Result 
 */
static Reflex_Result Reflex_scan_Driver(Reflex* reflex, void* obj, const void* fmt) {
    return reflex->Driver->Category[((const Reflex_TypeParams*) fmt)->Fields.Category]->fn[((const Reflex_TypeParams*) fmt)->Fields.Primary](reflex, obj, fmt);
}
#endif // REFLEX_SUPPORT_DRIVER
#if REFLEX_SUPPORT_COMPACT
/**
 * @brief This is a helper function that call callback of Compact functions
 * 
 * @param reflex 
 * @param obj 
 * @param fmt 
 * @return Reflex_Result 
 */
static Reflex_Result Reflex_scan_Compact(Reflex* reflex, void* obj, const void* fmt) {
    return reflex->CompactFns->fn[((const Reflex_TypeParams*) fmt)->Fields.Primary](reflex, obj, fmt);
}
#endif //REFLEX_SUPPORT_COMPACT
