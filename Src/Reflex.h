/**
 * @file Reflex.h
 * @author Ali Mirghasemi (ali.mirghasemi1376@gmail.com)
 * @brief Reflex is a powerful C library that provides reflection capabilities for C data structures. 
 *        It allows you to introspect C structures at runtime, 
 *        enabling you to access and manipulate the structure members dynamically. 
 *        This library is particularly useful for tasks like serialization, 
 *        deserialization, data validation, and generic data handling.
 * 
 * @version 0.1
 * @date 2023-04-08
 *
 * @copyright Copyright (c) 2023
 */
#ifndef _REFLEX_H_
#define _REFLEX_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

#define REFLEX_VER_MAJOR       0
#define REFLEX_VER_MINOR       1
#define REFLEX_VER_FIX         0

/********************************************************************************************/
/*                                     Configuration                                        */
/********************************************************************************************/
// Supported Architectures
#define REFLEX_ARCH_8BIT                    8
#define REFLEX_ARCH_16BIT                   16
#define REFLEX_ARCH_32BIT                   32
#define REFLEX_ARCH_64BIT                   64
/**
 * @brief Select your architecture bit width
 */
#define REFLEX_ARCH						    REFLEX_ARCH_64BIT
/**
 * @brief Support FormatMode_Param, it's affect on scan, getField, etc
 */
#define REFLEX_FORMAT_MODE_PARAM            1
/**
 * @brief Support FormatMode_Primary, it's affect on scan, getField, etc
 */
#define REFLEX_FORMAT_MODE_PRIMARY          1
/**
 * @brief Support FormatMode_Offset, it's affect on scan, getField, etc
 */
#define REFLEX_FORMAT_MODE_OFFSET           1
/**
 * @brief Support custom Reflex_TypeParams
 */
#define REFLEX_SUPPORT_CUSTOM_TYPE_PARAMS   1
/**
 * @brief Support args in reflex object
 */
#define REFLEX_SUPPORT_ARGS                 1
/**
 * @brief Support buffer in reflex object
 */
#define REFLEX_SUPPORT_BUFFER               1
/**
 * @brief Support Size functions API
 * If you want calculate size of schema in runtime, you need it
 */
#define REFLEX_SUPPORT_SIZE_FN              1
/**
 * @brief Support GetField functions API
 * If you want Serialize/Deserialize object in different way that they define in struct, you need it
 */
#define REFLEX_SUPPORT_SCAN_FIELD           1
/**
 * @brief Support Break functions API
 * Note: It's only available for complex objects
 * If you want to break process of Complex Array or 2D Array, you need it
 */
#define REFLEX_SUPPORT_BREAK_LAYER          1
/**
 * @brief Support VarIndex functions API
 * Note: Index of field give you a flat view over a complex object
 * Note 2: Complex objects don't count for index
 * If you want to know get field index, you need it
 */
#define REFLEX_SUPPORT_VAR_INDEX            1
/**
 * @brief Support MainObject functions API
 * If you want access address of parent object in callbacks, you need it
 */
#define REFLEX_SUPPORT_MAIN_OBJ             1
/**
 * @brief Support Scan Callback function mode
 */
#define REFLEX_SUPPORT_CALLBACK             1
/**
 * @brief Support Scan Driver function mode
 */
#define REFLEX_SUPPORT_DRIVER               1
/**
 * @brief Support Scan Compact function mode
 */
#define REFLEX_SUPPORT_COMPACT              1
/**
 * @brief Support 64-bit variables such as uint64_t and int64_t
 */
#define REFLEX_SUPPORT_TYPE_64BIT           1
/**
 * @brief Support double variables
 */
#define REFLEX_SUPPORT_TYPE_DOUBLE          1
/**
 * @brief Support Pointer category variables, it's affect on scan, getField, etc
 */
#define REFLEX_SUPPORT_TYPE_POINTER         1
/**
 * @brief Support Array category variables, it's affect on scan, getField, etc
 */
#define REFLEX_SUPPORT_TYPE_ARRAY           1
/**
 * @brief Support PointerArray category variables, it's affect on scan, getField, etc
 */
#define REFLEX_SUPPORT_TYPE_POINTER_ARRAY   1
/**
 * @brief Support Array2D category variables, it's affect on scan, getField, etc
 */
#define REFLEX_SUPPORT_TYPE_ARRAY_2D        1
/**
 * @brief Support complex variables such as struct or struct in struct, 
 * it's affect on scan, getField, etc
 */
#define REFLEX_SUPPORT_TYPE_COMPLEX         1
/**
 * @brief Define type of len variables
 * Note: it must be signed
 */
typedef int16_t Reflex_LenType;
/**
 * @brief Define type of offset type in Offset Schema
 * Note: it must be unsigned
 */
typedef uint8_t Reflex_OffsetType;
/**
 * @brief Define type of reflex result
 */
typedef uint32_t Reflex_Result;
/********************************************************************************************/

#define __REFLEX_VER_STR(major, minor, fix)     #major "." #minor "." #fix
#define _REFLEX_VER_STR(major, minor, fix)      __REFLEX_VER_STR(major, minor, fix)
/**
 * @brief show reflex version in string format
 */
#define REFLEX_VER_STR                          _REFLEX_VER_STR(REFLEX_VER_MAJOR, REFLEX_VER_MINOR, REFLEX_VER_FIX)
/**
 * @brief show REFLEX version in integer format, ex: 0.2.0 -> 200
 */
#define REFLEX_VER                              ((REFLEX_VER_MAJOR * 10000UL) + (REFLEX_VER_MINOR * 100UL) + (REFLEX_VER_FIX))

/* ------------------------ Pre-Defined data types ----------------------- */
struct __Reflex;
typedef struct __Reflex Reflex;
struct __Reflex_TypeParams;
typedef struct __Reflex_TypeParams Reflex_TypeParams;
struct __Reflex_Schema;
typedef struct __Reflex_Schema Reflex_Schema;
/**
 * @brief Common return of scan functions
 */
#define REFLEX_OK                               0
/**
 * @brief Reserved value for unknown errors of scan functions
 */
#define REFLEX_ERROR                            ((Reflex_Result)(~0))
/**
 * @brief Architecture bytes width
 */
#define REFLEX_ARCH_BYTES                       (REFLEX_ARCH / 8)

/* ------------------- Supportive Macros (DO NOT CHANGE ANYTHINGS) ----------------- */
#if REFLEX_FORMAT_MODE_OFFSET
    #define __REFLEX_TYPE_PARAMS_FIELD_OFFSET()             Reflex_OffsetType Offset
    #define __REFLEX_TYPE_PARAMS_FIELD_OFFSET_INIT(T, F)    .Offset = (Reflex_PtrType) &((T*) 0)->F,
#else
    #define __REFLEX_TYPE_PARAMS_FIELD_OFFSET()
    #define __REFLEX_TYPE_PARAMS_FIELD_OFFSET_INIT(T, F)
#endif

#if REFLEX_SUPPORT_TYPE_COMPLEX
    #define __REFLEX_TYPE_PARAM_FIELD_COMPLEX()             const Reflex_Schema* Schema
    #define __REFLEX_TYPE_PARAM_FIELD_COMPLEX_INIT(SCHEMA)  .Schema = (SCHEMA),
#else
    #define __REFLEX_TYPE_PARAM_FIELD_COMPLEX()
    #define __REFLEX_TYPE_PARAM_FIELD_COMPLEX_INIT(SCHEMA)
#endif

#if REFLEX_SUPPORT_CUSTOM_TYPE_PARAMS
    #define __REFLEX_SCHEMA_FIELD_FMT_SIZE()                Reflex_LenType FmtSize
    #define __REFLEX_SCHEMA_FIELD_FMT_SIZE_INIT(FMT)        .FmtSize = sizeof(FMT[0])
#else
    #define __REFLEX_SCHEMA_FIELD_FMT_SIZE()
    #define __REFLEX_SCHEMA_FIELD_FMT_SIZE_INIT(FMT)
#endif

#if REFLEX_SUPPORT_TYPE_ARRAY || REFLEX_SUPPORT_TYPE_ARRAY_2D || REFLEX_SUPPORT_TYPE_POINTER_ARRAY
    #define __REFLEX_TYPE_PARAMS_FIELD_LEN()                Reflex_LenType Len
    #define __REFLEX_TYPE_PARAMS_FIELD_LEN_INIT(LEN)        .Len = (LEN),
#else
    #define __REFLEX_TYPE_PARAMS_FIELD_LEN()
    #define __REFLEX_TYPE_PARAMS_FIELD_LEN_INIT(LEN)
#endif

#if REFLEX_SUPPORT_TYPE_ARRAY_2D
    #define __REFLEX_TYPE_PARAMS_FIELD_MLEN()               Reflex_LenType MLen
    #define __REFLEX_TYPE_PARAMS_FIELD_MLEN_INIT(MLEN)      .MLen = (MLEN),
#else
    #define __REFLEX_TYPE_PARAMS_FIELD_MLEN()
    #define __REFLEX_TYPE_PARAMS_FIELD_MLEN_INIT(MLEN)
#endif

#define __REFLEX_TYPE(CAT, TY)      Reflex_Type_ ##CAT ##_ ##TY = Reflex_Category_ ##CAT << 5 | Reflex_PrimaryType_ ##TY
/* -------------------------------- Types ------------------------------------ */
#if   REFLEX_ARCH == REFLEX_ARCH_64BIT
    typedef uint64_t Reflex_PtrType;
#elif REFLEX_ARCH == REFLEX_ARCH_32BIT
    typedef uint32_t Reflex_PtrType;
#else
    typedef uint16_t Reflex_PtrType;
#endif
/**
 * @brief Result of getField Functions
 */
typedef enum {
    Reflex_GetResult_Ok,
    Reflex_GetResult_NotFound,
} Reflex_GetResult;

/**
 * @brief Supported param categories
 */
typedef enum {
    Reflex_Category_Primary,       /**< Primary types, ex: u8,u16,char,float,etc */
#if REFLEX_SUPPORT_TYPE_POINTER
    Reflex_Category_Pointer,       /**< pointer mode for primary types */
#endif
#if REFLEX_SUPPORT_TYPE_ARRAY
    Reflex_Category_Array,         /**< array mode for primary types, required length */
#endif
#if REFLEX_SUPPORT_TYPE_POINTER_ARRAY
    Reflex_Category_PointerArray,  /**< array of pointers for primary types, required length */
#endif
#if REFLEX_SUPPORT_TYPE_ARRAY_2D
    Reflex_Category_Array2D,       /**< 2D array of primary types */
#endif
    Reflex_Category_Length,
} Reflex_Category;
/**
 * @brief Supported primary types
 */
typedef enum {
    Reflex_PrimaryType_Char,
    Reflex_PrimaryType_UInt8,
    Reflex_PrimaryType_Int8,
    Reflex_PrimaryType_UInt16,
    Reflex_PrimaryType_Int16,
    Reflex_PrimaryType_UInt32,
    Reflex_PrimaryType_Int32,
#if REFLEX_SUPPORT_TYPE_64BIT
    Reflex_PrimaryType_UInt64,
    Reflex_PrimaryType_Int64,
#endif
    Reflex_PrimaryType_Float,
#if REFLEX_SUPPORT_TYPE_DOUBLE
    Reflex_PrimaryType_Double,
#endif
#if REFLEX_SUPPORT_TYPE_COMPLEX
    Reflex_PrimaryType_Complex,
#endif
    Reflex_PrimaryType_Length,
} Reflex_PrimaryType;
/**
 * @brief Reflex type that user can use
 * format: Reflex_Type_<Category>_<PrimaryType>
 * ex: Reflex_Type_Pointer_UInt8
 *
 */
typedef enum {
    // ------------- Primary Types --------------
    __REFLEX_TYPE(Primary, Char),
    __REFLEX_TYPE(Primary, UInt8),
    __REFLEX_TYPE(Primary, Int8),
    __REFLEX_TYPE(Primary, UInt16),
    __REFLEX_TYPE(Primary, Int16),
    __REFLEX_TYPE(Primary, UInt32),
    __REFLEX_TYPE(Primary, Int32),
#if REFLEX_SUPPORT_TYPE_64BIT
    __REFLEX_TYPE(Primary, UInt64),
    __REFLEX_TYPE(Primary, Int64),
#endif
    __REFLEX_TYPE(Primary, Float),
#if REFLEX_SUPPORT_TYPE_DOUBLE
    __REFLEX_TYPE(Primary, Double),
#endif
#if REFLEX_SUPPORT_TYPE_COMPLEX
    __REFLEX_TYPE(Primary, Complex),
#endif
    // ------------- Pointer Types --------------
#if REFLEX_SUPPORT_TYPE_POINTER
    __REFLEX_TYPE(Pointer, Char),
    __REFLEX_TYPE(Pointer, UInt8),
    __REFLEX_TYPE(Pointer, Int8),
    __REFLEX_TYPE(Pointer, UInt16),
    __REFLEX_TYPE(Pointer, Int16),
    __REFLEX_TYPE(Pointer, UInt32),
    __REFLEX_TYPE(Pointer, Int32),
#if REFLEX_SUPPORT_TYPE_64BIT
    __REFLEX_TYPE(Pointer, UInt64),
    __REFLEX_TYPE(Pointer, Int64),
#endif
    __REFLEX_TYPE(Pointer, Float),
#if REFLEX_SUPPORT_TYPE_DOUBLE
    __REFLEX_TYPE(Pointer, Double),
#endif
#if REFLEX_SUPPORT_TYPE_COMPLEX
    __REFLEX_TYPE(Pointer, Complex),
#endif
#endif // REFLEX_SUPPORT_TYPE_POINTER
    // ------------- Array Types --------------
#if REFLEX_SUPPORT_TYPE_ARRAY
    __REFLEX_TYPE(Array, Char),
    __REFLEX_TYPE(Array, UInt8),
    __REFLEX_TYPE(Array, Int8),
    __REFLEX_TYPE(Array, UInt16),
    __REFLEX_TYPE(Array, Int16),
    __REFLEX_TYPE(Array, UInt32),
    __REFLEX_TYPE(Array, Int32),
#if REFLEX_SUPPORT_TYPE_64BIT
    __REFLEX_TYPE(Array, UInt64),
    __REFLEX_TYPE(Array, Int64),
#endif
    __REFLEX_TYPE(Array, Float),
#if REFLEX_SUPPORT_TYPE_DOUBLE
    __REFLEX_TYPE(Array, Double),
#endif
#if REFLEX_SUPPORT_TYPE_COMPLEX
    __REFLEX_TYPE(Array, Complex),
#endif
#endif // REFLEX_SUPPORT_TYPE_ARRAY
    // ------------- PointerArray Types --------------
#if REFLEX_SUPPORT_TYPE_POINTER_ARRAY
    __REFLEX_TYPE(PointerArray, Char),
    __REFLEX_TYPE(PointerArray, UInt8),
    __REFLEX_TYPE(PointerArray, Int8),
    __REFLEX_TYPE(PointerArray, UInt16),
    __REFLEX_TYPE(PointerArray, Int16),
    __REFLEX_TYPE(PointerArray, UInt32),
    __REFLEX_TYPE(PointerArray, Int32),
#if REFLEX_SUPPORT_TYPE_64BIT
    __REFLEX_TYPE(PointerArray, UInt64),
    __REFLEX_TYPE(PointerArray, Int64),
#endif
    __REFLEX_TYPE(PointerArray, Float),
#if REFLEX_SUPPORT_TYPE_DOUBLE
    __REFLEX_TYPE(PointerArray, Double),
#endif
#if REFLEX_SUPPORT_TYPE_COMPLEX
    __REFLEX_TYPE(PointerArray, Complex),
#endif
#endif // REFLEX_SUPPORT_TYPE_POINTER_ARRAY
    // ------------- Array2D Types --------------
#if REFLEX_SUPPORT_TYPE_ARRAY_2D
    __REFLEX_TYPE(Array2D, Char),
    __REFLEX_TYPE(Array2D, UInt8),
    __REFLEX_TYPE(Array2D, Int8),
    __REFLEX_TYPE(Array2D, UInt16),
    __REFLEX_TYPE(Array2D, Int16),
    __REFLEX_TYPE(Array2D, UInt32),
    __REFLEX_TYPE(Array2D, Int32),
#if REFLEX_SUPPORT_TYPE_64BIT
    __REFLEX_TYPE(Array2D, UInt64),
    __REFLEX_TYPE(Array2D, Int64),
#endif
    __REFLEX_TYPE(Array2D, Float),
#if REFLEX_SUPPORT_TYPE_DOUBLE
    __REFLEX_TYPE(Array2D, Double),
#endif
#if REFLEX_SUPPORT_TYPE_COMPLEX
    __REFLEX_TYPE(Array2D, Complex),
#endif
#endif // REFLEX_SUPPORT_TYPE_ARRAY_2D
} Reflex_Type;
/**
 * @brief Bit mask for primary part
 */
#define REFLEX_TYPE_PRIMARY_MASK     0x1F
/**
 * @brief Bit mask for category part
 */
#define REFLEX_TYPE_CATEGORY_MASK    0xE0
/**
 * @brief Helper type for Reflex_Type
 */
typedef union {
    uint8_t         Type;
    struct {
        uint8_t     Primary     : 5;
        uint8_t     Category    : 3;
    };
} Reflex_Type_BitFields;
/**
 * @brief This function callback used for scan all of object fields
 * Note: It's used for scan callback
 * 
 * @param reflex
 * @param obj address of object, or field value
 * @param fmt address of field format, it can be Reflex_TypeParams or any custom TypeParams
 * 
 * @return return REFLEX_OK if you want continue scan otherwise you can return error
 */
typedef Reflex_Result (*Reflex_OnFieldFn)(Reflex* reflex, void* obj, const void* fmt);
/**
 * @brief This object hold function pointers for scan
 * It can be used for Serialize/Deserialize
 */
typedef union {
    Reflex_OnFieldFn      fn[Reflex_PrimaryType_Length];
    struct {
        Reflex_OnFieldFn  fnChar;
        Reflex_OnFieldFn  fnUInt8;
        Reflex_OnFieldFn  fnInt8;
        Reflex_OnFieldFn  fnUInt16;
        Reflex_OnFieldFn  fnInt16;
        Reflex_OnFieldFn  fnUInt32;
        Reflex_OnFieldFn  fnInt32;
    #if REFLEX_SUPPORT_TYPE_64BIT
        Reflex_OnFieldFn  fnUInt64;
        Reflex_OnFieldFn  fnInt64;
    #endif
        Reflex_OnFieldFn  fnFloat;
    #if REFLEX_SUPPORT_TYPE_DOUBLE
        Reflex_OnFieldFn  fnDouble;
    #endif
    #if REFLEX_SUPPORT_TYPE_COMPLEX
        Reflex_OnFieldFn  fnComplexBegin;
        Reflex_OnFieldFn  fnComplexEnd;
    #endif
    };
} Reflex_ScanFunctions;
/**
 * @brief This object hold all categories scan functions
 */
typedef union {
    const Reflex_ScanFunctions*      Category[Reflex_Category_Length];
    struct {
        const Reflex_ScanFunctions*  Primary;
    #if REFLEX_SUPPORT_TYPE_POINTER
        const Reflex_ScanFunctions*  Pointer;
    #endif
    #if REFLEX_SUPPORT_TYPE_ARRAY
        const Reflex_ScanFunctions*  Array;
    #endif
    #if REFLEX_SUPPORT_TYPE_POINTER_ARRAY
        const Reflex_ScanFunctions*  PointerArray;
    #endif
    #if REFLEX_SUPPORT_TYPE_ARRAY_2D
        const Reflex_ScanFunctions*  Array2D;
    #endif
    };
} Reflex_ScanDriver;
/**
 * @brief This macro allows you to create custom Reflex_TypeParams
 * also it's support all configuration
 */
#define REFLEX_TYPE_PARAMS_STRUCT()     __REFLEX_TYPE_PARAM_FIELD_COMPLEX();   \
                                        __REFLEX_TYPE_PARAMS_FIELD_LEN();      /*< Used for Array Length, Array2D Row Length, Sizeof Struct, Sizeof Enum, Sizeof Union*/ \
                                        __REFLEX_TYPE_PARAMS_FIELD_MLEN();     /*< Used for Array2D Columns Length */ \
                                        union {                                \
                                            uint8_t                 Type;      \
                                            Reflex_Type_BitFields   Fields;    \
                                        };                                     \
                                        __REFLEX_TYPE_PARAMS_FIELD_OFFSET()
/**
 * @brief Basic TypeParams that user can use for create schema
 * Note: it's better to use REFLEX_TYPE_PARAMS macro for support all changes in configurations
 */
struct __Reflex_TypeParams {
    REFLEX_TYPE_PARAMS_STRUCT();
};
/**
 * @brief Supported modes for size functions
 */
typedef enum {
  Reflex_SizeType_Normal,
  Reflex_SizeType_Packed,
  Reflex_SizeType_Length,
} Reflex_SizeType;
/**
 * @brief Supported function mode for scan functions
 */
typedef enum {
#if REFLEX_SUPPORT_CALLBACK
    Reflex_FunctionMode_Callback,
#endif
#if REFLEX_SUPPORT_DRIVER
    Reflex_FunctionMode_Driver,
#endif
#if REFLEX_SUPPORT_COMPACT
    Reflex_FunctionMode_Compact,
#endif
    Reflex_FunctionMode_Length,
} Reflex_FunctionMode;
/**
 * @brief Supported format mode for scan, getField, etc
 */
typedef enum {
#if REFLEX_FORMAT_MODE_PARAM
    Reflex_FormatMode_Param,
#endif
#if REFLEX_FORMAT_MODE_PRIMARY
    Reflex_FormatMode_Primary,
#endif
#if REFLEX_FORMAT_MODE_OFFSET
    Reflex_FormatMode_Offset,
#endif
} Reflex_FormatMode;
/**
 * @brief This object hold properties of field
 * It used by getField, scanFieldRaw, scanField functions
 */
typedef struct {
    union {
        const Reflex_TypeParams*        Fmt;
        const uint8_t*                  PrimaryFmt;
        const void*                     CustomFmt;
    };
    void*                               Object;
#if REFLEX_SUPPORT_MAIN_OBJ
    void*                               MainObject;
#endif
#if REFLEX_SUPPORT_VAR_INDEX
    Reflex_LenType                      VarIndex;
#if REFLEX_SUPPORT_TYPE_COMPLEX
    Reflex_LenType                      VarOffset;
#endif
#endif
} Reflex_Field;
/**
 * @brief This object describe schema of object
 * Note: It's better to use REFLEX_SCHEMA macro to support all of configurations
 */
struct __Reflex_Schema {
    union {
        const uint8_t*                  PrimaryFmt;
        const Reflex_TypeParams*        Fmt;
        const void*                     CustomFmt;
    };
    Reflex_LenType                      Len;                            /**< number of variables */
    __REFLEX_SCHEMA_FIELD_FMT_SIZE();                                   /**< size of custom type params, it's used for custom type params */
    uint8_t                             FormatMode          : 2;        /**< Reflex Driver FunctionMode */
    uint8_t                             Reserved            : 6;
};
/**
 * @brief This is main reflex handler object
 * It hold all parameters that need
 */
struct __Reflex {
    union {
    #if REFLEX_SUPPORT_CALLBACK
        Reflex_OnFieldFn                    onField;
    #endif
    #if REFLEX_SUPPORT_DRIVER
        const Reflex_ScanDriver*            Driver;
    #endif
    #if REFLEX_SUPPORT_COMPACT
        const Reflex_ScanFunctions*         CompactFns;
    #endif
    };
    const Reflex_Schema*                    Schema;
#if REFLEX_SUPPORT_ARGS
    void*                                   Args;
#endif
#if REFLEX_SUPPORT_BUFFER
    void*                                   Buffer;
#endif
#if REFLEX_SUPPORT_MAIN_OBJ
    void*                                   MainObject;
#endif
#if REFLEX_SUPPORT_VAR_INDEX
#if REFLEX_SUPPORT_TYPE_COMPLEX
    void*                                   PObj;
    Reflex_LenType                          AlignSize;
    Reflex_LenType                          VarOffset;
    Reflex_LenType                          LayerIndex;
#endif
    Reflex_LenType                          VarIndex;
#endif
    uint8_t                                 FunctionMode        : 2;        /**< Reflex Driver FunctionMode, for PrimaryFmt it's optional */
    uint8_t                                 BreakLayer          : 1;
    uint8_t                                 BreakLayer2D        : 1;
    uint8_t                                 Reserved            : 4;
};

/* ----------------------------------- Main API ------------------------------------ */
void           Reflex_init(Reflex* reflex, const Reflex_Schema* schema);
Reflex_Result  Reflex_scanRaw(Reflex* reflex, void* obj, Reflex_OnFieldFn onField);
Reflex_Result  Reflex_scan(Reflex* reflex, void* obj);
Reflex_LenType Reflex_getVariablesLength(Reflex* reflex);
Reflex_LenType Reflex_sizeType(const Reflex_TypeParams* fmt);
#if REFLEX_SUPPORT_MAIN_OBJ
    void*      Reflex_getMainVariable(Reflex* reflex);
#endif
#if REFLEX_SUPPORT_ARGS
    void       Reflex_setArgs(Reflex* reflex, void* args);
    void*      Reflex_getArgs(Reflex* reflex);
#endif
#if REFLEX_SUPPORT_BUFFER
    void       Reflex_setBuffer(Reflex* reflex, void* buf);
    void*      Reflex_getBuffer(Reflex* reflex);
#endif
/* ------------------------------- FunctionMode API -------------------------------- */
#if REFLEX_SUPPORT_CALLBACK
    void       Reflex_setCallback(Reflex* reflex, Reflex_OnFieldFn fn);
#endif
#if REFLEX_SUPPORT_DRIVER
    void       Reflex_setDriver(Reflex* reflex, const Reflex_ScanDriver* driver);
#endif
#if REFLEX_SUPPORT_COMPACT
    void       Reflex_setCompact(Reflex* reflex, const Reflex_ScanFunctions* compact);
#endif
/* --------------------------------- GetField API ---------------------------------- */
#if REFLEX_SUPPORT_SCAN_FIELD
    Reflex_GetResult Reflex_getField(Reflex* reflex, void* obj, const void* fieldFmt, Reflex_Field* field);
    Reflex_Result    Reflex_scanFieldRaw(Reflex* reflex, Reflex_Field* field, Reflex_OnFieldFn onField);
    Reflex_Result    Reflex_scanField(Reflex* reflex, void* obj, const void* fieldFmt);
#endif
/* ---------------------------------- VarIndex API --------------------------------- */
#if REFLEX_SUPPORT_VAR_INDEX  
    Reflex_LenType Reflex_getVarIndex(Reflex* reflex);
#if REFLEX_SUPPORT_TYPE_COMPLEX
    Reflex_LenType Reflex_getVarOffset(Reflex* reflex);
    Reflex_LenType Reflex_getVarIndexReal(Reflex* reflex);
#else
    #define        Reflex_getVarIndexReal       Reflex_getVarIndex
#endif
#endif
/* ----------------------------------- Break API ----------------------------------- */
#if REFLEX_SUPPORT_BREAK_LAYER && REFLEX_SUPPORT_TYPE_COMPLEX && (REFLEX_SUPPORT_TYPE_ARRAY || REFLEX_SUPPORT_TYPE_ARRAY_2D || REFLEX_SUPPORT_TYPE_POINTER_ARRAY)
    void           Reflex_break(Reflex* reflex);
#if REFLEX_SUPPORT_TYPE_ARRAY_2D
    void           Reflex_break2D(Reflex* reflex);
#endif
#endif
/* ----------------------------------- Field API ----------------------------------- */
#if REFLEX_SUPPORT_SCAN_FIELD
    void*          Reflex_Field_getVariable(Reflex_Field* reflex);
#if REFLEX_SUPPORT_MAIN_OBJ
    void*          Reflex_Field_getMainVariable(Reflex_Field* reflex);
#endif
#if REFLEX_SUPPORT_VAR_INDEX
    Reflex_LenType Reflex_Field_getVarIndex(Reflex_Field* reflex);
#if REFLEX_SUPPORT_TYPE_COMPLEX
    Reflex_LenType Reflex_Field_getVarOffset(Reflex_Field* reflex);
    Reflex_LenType Reflex_Field_getVarIndexReal(Reflex_Field* reflex);
#else
    #define        Reflex_Field_getVarIndexReal     Reflex_Field_getVarIndex
#endif // REFLEX_SUPPORT_TYPE_COMPLEX
#endif // REFLEX_SUPPORT_VAR_INDEX
#endif // REFLEX_SUPPORT_SCAN_FIELD
/* ----------------------------------- Size API ----------------------------------- */
#if REFLEX_SUPPORT_SIZE_FN
    Reflex_LenType Reflex_size(const Reflex_Schema* schema, Reflex_SizeType type);
#if REFLEX_FORMAT_MODE_PARAM
    Reflex_LenType Reflex_Param_sizeNormal(const Reflex_Schema* schema);
    Reflex_LenType Reflex_Param_sizePacked(const Reflex_Schema* schema);
#endif
#if REFLEX_FORMAT_MODE_PRIMARY
    Reflex_LenType Reflex_Primary_sizeNormal(const Reflex_Schema* schema);
    Reflex_LenType Reflex_Primary_sizePacked(const Reflex_Schema* schema);
#endif
#if REFLEX_FORMAT_MODE_OFFSET
    Reflex_LenType Reflex_Offset_sizeNormal(const Reflex_Schema* schema);
    Reflex_LenType Reflex_Offset_sizePacked(const Reflex_Schema* schema);
#endif
#endif // REFLEX_SUPPORT_SIZE_FN
/* --------------------------------- LowLevel Scan API ----------------------------------- */
#if REFLEX_FORMAT_MODE_PARAM
    Reflex_Result Reflex_Param_scanRaw(Reflex* reflex, void* obj, Reflex_OnFieldFn onField);
    Reflex_Result Reflex_Param_scan(Reflex* reflex, void* obj);
#endif
#if REFLEX_FORMAT_MODE_PRIMARY
    Reflex_Result Reflex_Primary_scanRaw(Reflex* reflex, void* obj, Reflex_OnFieldFn onField);
    Reflex_Result Reflex_Primary_scan(Reflex* reflex, void* obj);
#endif
#if REFLEX_FORMAT_MODE_OFFSET
    Reflex_Result Reflex_Offset_scanRaw(Reflex* reflex, void* obj, Reflex_OnFieldFn onField);
    Reflex_Result Reflex_Offset_scan(Reflex* reflex, void* obj);
#endif
#if REFLEX_SUPPORT_TYPE_COMPLEX
    Reflex_Result Reflex_Complex_scanRaw(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, Reflex_OnFieldFn onField);
    Reflex_Result Reflex_Complex_scan(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, Reflex_OnFieldFn onField);
#endif
/* -------------------------------- LowLevel GetField API ----------------------------------- */
#if REFLEX_SUPPORT_SCAN_FIELD
#if REFLEX_FORMAT_MODE_PARAM
    Reflex_GetResult Reflex_Param_getField(Reflex* reflex, void* obj, const void* fieldFmt, Reflex_Field* field);
#endif
#if REFLEX_FORMAT_MODE_PRIMARY
    Reflex_GetResult Reflex_Primary_getField(Reflex* reflex, void* obj, const void* fieldFmt, Reflex_Field* field);
#endif
#if REFLEX_FORMAT_MODE_OFFSET
    Reflex_GetResult Reflex_Offset_getField(Reflex* reflex, void* obj, const void* fieldFmt, Reflex_Field* field);
#endif
#if REFLEX_SUPPORT_TYPE_COMPLEX
    Reflex_GetResult Reflex_Complex_getField(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, const void* fieldFmt, Reflex_Field* field);
#endif
#endif // REFLEX_SUPPORT_SCAN_FIELD
/* -------------------------------- Helper Macros API ----------------------------------- */
/**
 * @brief This macro help you to fill Reflex_TypeParams struct with default
 * parameters, and also support all possible configurations
 * @param TY Reflex_Type
 * @param LEN length of array
 * @param MLEN number of columns of 2D array
 * @param C schema of field, used for complex type
 * @param T struct name, used for offset schema
 * @param F field name, used for offset schema
 */
#define REFLEX_TYPE_PARAMS_FULL(TY, LEN, MLEN, C, T, F) {                   \
                                                            .Type = (TY),   \
                                                            __REFLEX_TYPE_PARAMS_FIELD_LEN_INIT(LEN)    \
                                                            __REFLEX_TYPE_PARAMS_FIELD_MLEN_INIT(MLEN)  \
                                                            __REFLEX_TYPE_PARAMS_FIELD_OFFSET_INIT(T, F)\
                                                            __REFLEX_TYPE_PARAM_FIELD_COMPLEX_INIT(C)   \
                                                        }
/**
 * @brief This macro help you to fill Reflex_TypeParams struct with default
 * parameters
 * @param TY Reflex_Type
 */
#define REFLEX_TYPE_PARAMS_PRIMARY(TY)                  {                 \
                                                            .Type = TY,   \
                                                        }
/**
 * @brief This macro help you to fill Reflex_TypeParams struct with default
 * parameters
 * @param TY Reflex_Type
 * @param LEN length of array
 */
#define REFLEX_TYPE_PARAMS_ARRAY(TY, LEN)               {                 \
                                                            .Type = TY,   \
                                                            __REFLEX_TYPE_PARAMS_FIELD_LEN_INIT(LEN)    \
                                                        }
/**
 * @brief This macro help you to fill Reflex_TypeParams struct with default
 * parameters
 * @param TY Reflex_Type
 * @param LEN length of array
 * @param MLEN number of columns of 2D array
 */
#define REFLEX_TYPE_PARAMS_ARRAY_2D(TY, LEN, MLEN)      {                 \
                                                            .Type = TY,   \
                                                            __REFLEX_TYPE_PARAMS_FIELD_LEN_INIT(LEN)   \
                                                            __REFLEX_TYPE_PARAMS_FIELD_MLEN_INIT(MLEN) \
                                                        }
/**
 * @brief This macro help you to fill Reflex_TypeParams struct with default
 * parameters
 * @param TY Reflex_Type
 * @param LEN length of array
 * @param MLEN number of columns of 2D array
 * @param T struct name, used for offset schema
 * @param F field name, used for offset schema
 */
#define REFLEX_TYPE_PARAMS_COMPLEX(TY, LEN, MLEN, C)    {                 \
                                                            .Type = TY,   \
                                                            __REFLEX_TYPE_PARAMS_FIELD_LEN_INIT(LEN)   \
                                                            __REFLEX_TYPE_PARAMS_FIELD_MLEN_INIT(MLEN)  \
                                                            __REFLEX_TYPE_PARAM_FIELD_COMPLEX_INIT(C)  \
                                                        }
/**
 * @brief This macro help you to fill Reflex_TypeParams struct with default
 * parameters
 * @param TY Reflex_Type
 * @param LEN length of array
 * @param MLEN number of columns of 2D array
 * @param T struct name, used for offset schema
 * @param F field name, used for offset schema
 */
#define REFLEX_TYPE_PARAMS_OFFSET(TY, LEN, MLEN, T, F)  {                 \
                                                            .Type = TY,   \
                                                            __REFLEX_TYPE_PARAMS_FIELD_LEN_INIT(LEN)   \
                                                            __REFLEX_TYPE_PARAMS_FIELD_MLEN_INIT(MLEN)  \
                                                            __REFLEX_TYPE_PARAMS_FIELD_OFFSET_INIT(T, F) \
                                                        }


/**
 * @brief This macro can be used for initialize Reflex_TypeParams with default parameters
 * Input Format:
 * - (TY)                       -> Primary, Pointer
 * - (TY, LEN)                  -> Array, PointerArray,
 * - (TY, LEN, MLEN)            -> Array2D
 * - (TY, LEN, MLEN, C)         -> ComplexType
 * - (TY, LEN, MLEN, T, F)      -> Offset Schema
 * - (TY, LEN, MLEN, C, T, F)   -> Full
 * *All format support backward compatibility
 *
 * @param TY Reflex_Type
 * @param LEN length of array
 * @param MLEN number of columns of 2D array
 * @param C schema of field, used for complex type
 * @param T struct name, used for offset schema
 * @param F field name, used for offset schema
 */
#define REFLEX_TYPE_PARAMS(...)                         __REFLEX_TYPE_PARAMS(__VA_ARGS__)
/**
 * @brief This macro allows you to fill schema, also it's support all if configurations
 * 
 * @param MODE mode of schema, ex: Reflex_FormatMode_Param
 * @param FMT name of TypeParams array
 */
#define REFLEX_SCHEMA(MODE, FMT)                   {                       \
                                                            .FormatMode = MODE, \
                                                            .CustomFmt = FMT,   \
                                                            .Len = REFLEX_TYPE_PARAMS_LEN(FMT), \
                                                            __REFLEX_SCHEMA_FIELD_FMT_SIZE_INIT(FMT) \
                                                        }
/**
 * @brief This macro help you to calculate length of fields format
 * Note: It also can use for other arrays
 * 
 * @param ARR name of array
 */
#define REFLEX_TYPE_PARAMS_LEN(ARR)                     (sizeof(ARR) / sizeof(ARR[0]))

// ----------------------------------- Private Helper Macros --------------------------
#define __REFLEX_TYPE_PARAMS_N_(_0, _1, _2, _3, _4, _5, FMT, ...) REFLEX_TYPE_PARAMS_ ##FMT
#define __REFLEX_TYPE_PARAMS_N(...)                     __REFLEX_TYPE_PARAMS_N_(__VA_ARGS__)
#define __REFLEX_TYPE_PARAMS_FMT_PAT()                  FULL, OFFSET, COMPLEX, ARRAY_2D, ARRAY, PRIMARY
#define __REFLEX_TYPE_PARAMS_BUILD_(FMT, ...)           FMT(__VA_ARGS__)
#define __REFLEX_TYPE_PARAMS_BUILD(...)                 __REFLEX_TYPE_PARAMS_BUILD_(__REFLEX_TYPE_PARAMS_N(__VA_ARGS__, __REFLEX_TYPE_PARAMS_FMT_PAT()), __VA_ARGS__)
#define __REFLEX_TYPE_PARAMS(...)                       __REFLEX_TYPE_PARAMS_BUILD(__VA_ARGS__)

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif // _REFLEX_H_

