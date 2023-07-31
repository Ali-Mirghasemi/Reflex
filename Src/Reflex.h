/**
 * @file Reflex.h
 * @author Ali Mirghasemi (ali.mirghasemi1376@gmail.com)
 * @brief This library help user to serialize and deserialize objects in c languages
 * @version 0.1
 * @date 2023-04-08
 *
 * @copyright Copyright (c) 2023
 *
 */


#ifndef _REFLEX_H_
#define _REFLEX_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define REFLEX_VER_MAJOR       0
#define REFLEX_VER_MINOR       1
#define REFLEX_VER_FIX         0

#include <stdint.h>

/********************************************************************************************/
/*                                     Configuration                                        */
/********************************************************************************************/

/**
 * @brief select your architecture bit width
 */
#define REFLEX_ARCH_8BIT                    8
#define REFLEX_ARCH_16BIT                   16
#define REFLEX_ARCH_32BIT                   32
#define REFLEX_ARCH_64BIT                   64
#define REFLEX_ARCH						    REFLEX_ARCH_64BIT

#define REFLEX_FORMAT_MODE_PARAM            1
#define REFLEX_FORMAT_MODE_PRIMARY          1
#define REFLEX_FORMAT_MODE_OFFSET           1

#define REFLEX_SUPPORT_64BIT_INT            1
#define REFLEX_SUPPORT_DOUBLE               1
#define REFLEX_SUPPORT_COMPLEX_TYPE         1
#define REFLEX_SUPPORT_CUSTOM_TYPE_PARAMS   1
#define REFLEX_SUPPORT_ARGS                 1
#define REFLEX_SUPPORT_BUFFER               1

typedef int16_t Reflex_LenType;

typedef uint8_t Reflex_OffsetType;

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

/* Pre-Defined data types */
struct __Reflex;
typedef struct __Reflex Reflex;
struct __Reflex_TypeParams;
typedef struct __Reflex_TypeParams Reflex_TypeParams;
struct __Reflex_Schema;
typedef struct __Reflex_Schema Reflex_Schema;

#define REFLEX_OK                               0
#define REFLEX_ARCH_BYTES                       (REFLEX_ARCH / 8)

/* Supportive Macros */
#if REFLEX_FORMAT_MODE_OFFSET
    #define __REFLEX_TYPE_PARAMS_FIELD_OFFSET()             Reflex_OffsetType Offset
    #define __REFLEX_TYPE_PARAMS_FIELD_OFFSET_INIT(T, F)    .Offset = (Reflex_OffsetType) &((T*) 0)->F
#else
    #define __REFLEX_TYPE_PARAMS_FIELD_OFFSET()
    #define __REFLEX_TYPE_PARAMS_FIELD_OFFSET_INIT(T, F)
#endif

#if REFLEX_SUPPORT_COMPLEX_TYPE
    #define __REFLEX_TYPE_PARAM_FIELD_COMPLEX()             const Reflex_Schema* Schema
#else
    #define __REFLEX_TYPE_PARAM_FIELD_COMPLEX()
#endif

#if REFLEX_SUPPORT_CUSTOM_TYPE_PARAMS
    #define __REFLEX_SCHEMA_FIELD_FMT_SIZE()                Reflex_LenType FmtSize
    #define __REFLEX_SCHEMA_FIELD_FMT_SIZE_INIT(FMT)        .FmtSize = sizeof(FMT[0])
#else
    #define __REFLEX_SCHEMA_FIELD_FMT_SIZE()
    #define __REFLEX_SCHEMA_FIELD_FMT_SIZE_INIT(FMT)
#endif

/**
 * @brief supported param categories
 */
typedef enum {
    Reflex_Category_Primary,       /**< Primary types, ex: u8,u16,char,float,... */
    Reflex_Category_Pointer,       /**< pointer mode for primary types */
    Reflex_Category_Array,         /**< array mode for primary types, required length */
    Reflex_Category_PointerArray,  /**< array of pointers for primary types, required length */
    Reflex_Category_Array2D,       /**< 2D array of primary types */
    Reflex_Category_Length,
} Reflex_Category;
/**
 * @brief supported primary types
 */
typedef enum {
    Reflex_PrimaryType_Unknown,
    Reflex_PrimaryType_Char,
    Reflex_PrimaryType_UInt8,
    Reflex_PrimaryType_Int8,
    Reflex_PrimaryType_UInt16,
    Reflex_PrimaryType_Int16,
    Reflex_PrimaryType_UInt32,
    Reflex_PrimaryType_Int32,
#if REFLEX_SUPPORT_64BIT_INT
    Reflex_PrimaryType_UInt64,
    Reflex_PrimaryType_Int64,
#endif
    Reflex_PrimaryType_Float,
#if REFLEX_SUPPORT_DOUBLE
    Reflex_PrimaryType_Double,
#endif
#if REFLEX_SUPPORT_COMPLEX_TYPE
    Reflex_PrimaryType_Complex,
#endif
    Reflex_PrimaryType_Length,
} Reflex_PrimaryType;

#define REFLEX_TYPE(CAT, TY)         Reflex_Type_ ##CAT ##_ ##TY = Reflex_Category_ ##CAT << 5 | Reflex_PrimaryType_ ##TY

/**
 * @brief Reflex type that user can use
 * format: Reflex_Type_<Category>_<PrimaryType>
 * ex: Reflex_Type_Pointer_UInt8
 *
 */
typedef enum {
    // ------------- Primary Types --------------
    REFLEX_TYPE(Primary, Unknown),
    REFLEX_TYPE(Primary, Char),
    REFLEX_TYPE(Primary, UInt8),
    REFLEX_TYPE(Primary, Int8),
    REFLEX_TYPE(Primary, UInt16),
    REFLEX_TYPE(Primary, Int16),
    REFLEX_TYPE(Primary, UInt32),
    REFLEX_TYPE(Primary, Int32),
#if REFLEX_SUPPORT_64BIT_INT
    REFLEX_TYPE(Primary, UInt64),
    REFLEX_TYPE(Primary, Int64),
#endif
    REFLEX_TYPE(Primary, Float),
#if REFLEX_SUPPORT_DOUBLE
    REFLEX_TYPE(Primary, Double),
#endif
#if REFLEX_SUPPORT_COMPLEX_TYPE
    REFLEX_TYPE(Primary, Complex),
#endif
    // ------------- Pointer Types --------------
    REFLEX_TYPE(Pointer, Unknown),
    REFLEX_TYPE(Pointer, Char),
    REFLEX_TYPE(Pointer, UInt8),
    REFLEX_TYPE(Pointer, Int8),
    REFLEX_TYPE(Pointer, UInt16),
    REFLEX_TYPE(Pointer, Int16),
    REFLEX_TYPE(Pointer, UInt32),
    REFLEX_TYPE(Pointer, Int32),
#if REFLEX_SUPPORT_64BIT_INT
    REFLEX_TYPE(Pointer, UInt64),
    REFLEX_TYPE(Pointer, Int64),
#endif
    REFLEX_TYPE(Pointer, Float),
#if REFLEX_SUPPORT_DOUBLE
    REFLEX_TYPE(Pointer, Double),
#endif
#if REFLEX_SUPPORT_COMPLEX_TYPE
    REFLEX_TYPE(Pointer, Complex),
#endif
    // ------------- Array Types --------------
    REFLEX_TYPE(Array, Unknown),
    REFLEX_TYPE(Array, Char),
    REFLEX_TYPE(Array, UInt8),
    REFLEX_TYPE(Array, Int8),
    REFLEX_TYPE(Array, UInt16),
    REFLEX_TYPE(Array, Int16),
    REFLEX_TYPE(Array, UInt32),
    REFLEX_TYPE(Array, Int32),
#if REFLEX_SUPPORT_64BIT_INT
    REFLEX_TYPE(Array, UInt64),
    REFLEX_TYPE(Array, Int64),
#endif
    REFLEX_TYPE(Array, Float),
#if REFLEX_SUPPORT_DOUBLE
    REFLEX_TYPE(Array, Double),
#endif
#if REFLEX_SUPPORT_COMPLEX_TYPE
    REFLEX_TYPE(Array, Complex),
#endif
    // ------------- PointerArray Types --------------
    REFLEX_TYPE(PointerArray, Unknown),
    REFLEX_TYPE(PointerArray, Char),
    REFLEX_TYPE(PointerArray, UInt8),
    REFLEX_TYPE(PointerArray, Int8),
    REFLEX_TYPE(PointerArray, UInt16),
    REFLEX_TYPE(PointerArray, Int16),
    REFLEX_TYPE(PointerArray, UInt32),
    REFLEX_TYPE(PointerArray, Int32),
#if REFLEX_SUPPORT_64BIT_INT
    REFLEX_TYPE(PointerArray, UInt64),
    REFLEX_TYPE(PointerArray, Int64),
#endif
    REFLEX_TYPE(PointerArray, Float),
#if REFLEX_SUPPORT_DOUBLE
    REFLEX_TYPE(PointerArray, Double),
#endif
#if REFLEX_SUPPORT_COMPLEX_TYPE
    REFLEX_TYPE(PointerArray, Complex),
#endif
    // ------------- Array2D Types --------------
    REFLEX_TYPE(Array2D, Unknown),
    REFLEX_TYPE(Array2D, Char),
    REFLEX_TYPE(Array2D, UInt8),
    REFLEX_TYPE(Array2D, Int8),
    REFLEX_TYPE(Array2D, UInt16),
    REFLEX_TYPE(Array2D, Int16),
    REFLEX_TYPE(Array2D, UInt32),
    REFLEX_TYPE(Array2D, Int32),
#if REFLEX_SUPPORT_64BIT_INT
    REFLEX_TYPE(Array2D, UInt64),
    REFLEX_TYPE(Array2D, Int64),
#endif
    REFLEX_TYPE(Array2D, Float),
#if REFLEX_SUPPORT_DOUBLE
    REFLEX_TYPE(Array2D, Double),
#endif
#if REFLEX_SUPPORT_COMPLEX_TYPE
    REFLEX_TYPE(Array2D, Complex),
#endif
} Reflex_Type;

#define Reflex_Type_Unknown          Reflex_Type_Primary_Unknown

#define REFLEX_TYPE_PRIMARY_MASK     0x1F
#define REFLEX_TYPE_CATEGORY_MASK    0xE0
typedef union {
    uint8_t         Type;
    struct {
        uint8_t     Primary     : 5;
        uint8_t     Category    : 3;
    };
} Reflex_Type_BitFields;

/**
 * @brief This function callback used for scan all of object fields
 *
 */
typedef Reflex_Result (*Reflex_OnFieldFn)(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt);

typedef union {
    Reflex_OnFieldFn      fn[Reflex_PrimaryType_Length];
    struct {
        Reflex_OnFieldFn  fnUnknown;
        Reflex_OnFieldFn  fnChar;
        Reflex_OnFieldFn  fnUInt8;
        Reflex_OnFieldFn  fnInt8;
        Reflex_OnFieldFn  fnUInt16;
        Reflex_OnFieldFn  fnInt16;
        Reflex_OnFieldFn  fnUInt32;
        Reflex_OnFieldFn  fnInt32;
    #if REFLEX_SUPPORT_64BIT_INT
        Reflex_OnFieldFn  fnUInt64;
        Reflex_OnFieldFn  fnInt64;
    #endif
        Reflex_OnFieldFn  fnFloat;
    #if REFLEX_SUPPORT_DOUBLE
        Reflex_OnFieldFn  fnDouble;
    #endif
    #if REFLEX_SUPPORT_COMPLEX_TYPE
        Reflex_OnFieldFn  fnComplexBegin;
        Reflex_OnFieldFn  fnComplexEnd;
    #endif
    };
} Reflex_ScanFunctions;

typedef union {
    const Reflex_ScanFunctions*      Category[Reflex_Category_Length];
    struct {
        const Reflex_ScanFunctions*  Primary;
        const Reflex_ScanFunctions*  Pointer;
        const Reflex_ScanFunctions*  Array;
        const Reflex_ScanFunctions*  PointerArray;
        const Reflex_ScanFunctions*  Array2D;
    };
} Reflex_ScanDriver;

#define REFLEX_TYPE_PARAMS_STRUCT()     Reflex_LenType              Len;       /*< Used for Array Length, Array2D Row Length, Sizeof Struct, Sizeof Enum, Sizeof Union*/ \
                                        Reflex_LenType              MLen;      /*< Used for Array2D Columns Length */ \
                                        union {                                \
                                            uint8_t                 Type;      \
                                            Reflex_Type_BitFields   Fields;    \
                                        };                                     \
                                        __REFLEX_TYPE_PARAM_FIELD_COMPLEX();    \
                                        __REFLEX_TYPE_PARAMS_FIELD_OFFSET()

struct __Reflex_TypeParams {
    REFLEX_TYPE_PARAMS_STRUCT();
};

#define REFLEX_TYPE_PARAMS(TY, LEN, MLEN)               {                 \
                                                            .Type = TY,   \
                                                            .Len = LEN,   \
                                                            .MLen = MLEN, \
                                                        }
#define REFLEX_TYPE_PRIMARY(TY)                         {                 \
                                                            .Type = TY,   \
                                                            .Len = 0,     \
                                                            .MLen = 0,    \
                                                        }
#define REFLEX_TYPE_PARAMS_OFFSET(TY, LEN, MLEN, F, T)  {                 \
                                                            .Type = TY,   \
                                                            .Len = LEN,   \
                                                            .MLen = MLEN, \
                                                            __REFLEX_TYPE_PARAMS_FIELD_OFFSET_INIT(F, T) \
                                                        }
#define REFLEX_TYPE_PARAMS_COMPLEX(TY, LEN, MLEN, C)    {                 \
                                                            .Type = TY,   \
                                                            .Len = LEN,   \
                                                            .MLen = MLEN, \
                                                            .Schema = C,  \
                                                        }

#define REFLEX_TYPE_PARAMS_LEN(ARR)                     (sizeof(ARR) / sizeof(ARR[0]))

typedef enum {
  Reflex_SizeType_Normal,
  Reflex_SizeType_Packed,
} Reflex_SizeType;

typedef enum {
    Reflex_FunctionMode_Callback,
    Reflex_FunctionMode_Driver,
    Reflex_FunctionMode_Compact,
} Reflex_FunctionMode;

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

#define REFLEX_SCHEMA_INIT(MODE, FMT)           {                       \
                                                    .FormatMode = MODE, \
                                                    .CustomFmt = FMT,   \
                                                    .Len = REFLEX_TYPE_PARAMS_LEN(FMT), \
                                                    __REFLEX_SCHEMA_FIELD_FMT_SIZE_INIT(FMT) \
                                                }

struct __Reflex {
#if REFLEX_SUPPORT_ARGS
    void*                                   Args;
#endif
#if REFLEX_SUPPORT_BUFFER
    void*                                   Buffer;
#endif
    void*                                   Obj;
    union {
        Reflex_OnFieldFn                    onField;
        const Reflex_ScanDriver*            Driver;
        const Reflex_ScanFunctions*         CompactFns;
    };
    const Reflex_Schema*                    Schema;
#if REFLEX_SUPPORT_COMPLEX_TYPE
    void*                                   PObj;
    Reflex_LenType                          AlignSize;
    Reflex_LenType                          VariableIndexOffset;
#endif
    Reflex_LenType                          VariableIndex;
    uint8_t                                 FunctionMode        : 2;        /**< Reflex Driver FunctionMode, for PrimaryFmt it's optional */
    uint8_t                                 Reserved            : 6;
};

// ---------------------- Main API ----------------------
Reflex_Result Reflex_scanRaw(Reflex* reflex, void* obj, Reflex_OnFieldFn onField);
Reflex_Result Reflex_scan(Reflex* reflex, void* obj);
Reflex_LenType Reflex_size(const Reflex_Schema* schema, Reflex_SizeType type);
Reflex_LenType Reflex_sizeType(const Reflex_TypeParams* fmt);
// ------------------- Utils Functions ------------------
void Reflex_setCallback(Reflex* reflex, Reflex_OnFieldFn fn);
void Reflex_setDriver(Reflex* reflex, const Reflex_ScanDriver* driver);
void Reflex_setCompact(Reflex* reflex, const Reflex_ScanFunctions* compact);

Reflex_LenType Reflex_getVariableIndex(Reflex* reflex);
Reflex_LenType Reflex_getVariablesLength(Reflex* reflex);
void*          Reflex_getMainVariable(Reflex* reflex);

#if REFLEX_SUPPORT_ARGS
    void  Reflex_setArgs(Reflex* reflex, void* args);
    void* Reflex_getArgs(Reflex* reflex);
#endif

#if REFLEX_SUPPORT_BUFFER
    void  Reflex_setBuffer(Reflex* reflex, void* buf);
    void* Reflex_getBuffer(Reflex* reflex);
#endif

// ------------------------ Scan Functions ----------------------
#if REFLEX_FORMAT_MODE_PARAM
    Reflex_Result Reflex_Param_scanRaw(Reflex* reflex, void* obj, Reflex_OnFieldFn onField);
#endif
#if REFLEX_FORMAT_MODE_PRIMARY
    Reflex_Result Reflex_Primary_scanRaw(Reflex* reflex, void* obj, Reflex_OnFieldFn onField);
#endif
#if REFLEX_FORMAT_MODE_OFFSET
    Reflex_Result Reflex_Offset_scanRaw(Reflex* reflex, void* obj, Reflex_OnFieldFn onField);
#endif
#if REFLEX_SUPPORT_COMPLEX_TYPE
    Reflex_Result Reflex_Complex_scanRaw(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, Reflex_OnFieldFn onField);
#endif
// --------------------- Driver Functions -------------------
#if REFLEX_FORMAT_MODE_PARAM
    Reflex_Result Reflex_Param_scan(Reflex* reflex, void* obj);
#endif
#if REFLEX_FORMAT_MODE_PRIMARY
    Reflex_Result Reflex_Primary_scan(Reflex* reflex, void* obj);
#endif
#if REFLEX_FORMAT_MODE_OFFSET
    Reflex_Result Reflex_Offset_scan(Reflex* reflex, void* obj);
#endif
#if REFLEX_SUPPORT_COMPLEX_TYPE
    Reflex_Result Reflex_Complex_scan(Reflex* reflex, void* obj, const Reflex_TypeParams* fmt, Reflex_OnFieldFn onField);
#endif

// ------------------------ Size Functions --------------------
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

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif // _REFLEX_H_

