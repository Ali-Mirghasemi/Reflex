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
 * 
 */
#define REFLEX_ARCH						32 // bits

typedef int16_t Reflex_LenType;

typedef uint32_t Reflex_Result;

/********************************************************************************************/

/* Pre-Defined data types */
struct __Reflex;
typedef struct __Reflex Reflex;

#define REFLEX_OK           0

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
    Reflex_PrimaryType_UInt64,
    Reflex_PrimaryType_Int64,
    Reflex_PrimaryType_Float,
    Reflex_PrimaryType_Double,
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
    REFLEX_TYPE(Primary, UInt64),
    REFLEX_TYPE(Primary, Int64),
    REFLEX_TYPE(Primary, Float),
    REFLEX_TYPE(Primary, Double),
    // ------------- Pointer Types --------------
    REFLEX_TYPE(Pointer, Unknown),
    REFLEX_TYPE(Pointer, Char),
    REFLEX_TYPE(Pointer, UInt8),
    REFLEX_TYPE(Pointer, Int8),
    REFLEX_TYPE(Pointer, UInt16),
    REFLEX_TYPE(Pointer, Int16),
    REFLEX_TYPE(Pointer, UInt32),
    REFLEX_TYPE(Pointer, Int32),
    REFLEX_TYPE(Pointer, UInt64),
    REFLEX_TYPE(Pointer, Int64),
    REFLEX_TYPE(Pointer, Float),
    REFLEX_TYPE(Pointer, Double),
    // ------------- Array Types --------------
    REFLEX_TYPE(Array, Unknown),
    REFLEX_TYPE(Array, Char),
    REFLEX_TYPE(Array, UInt8),
    REFLEX_TYPE(Array, Int8),
    REFLEX_TYPE(Array, UInt16),
    REFLEX_TYPE(Array, Int16),
    REFLEX_TYPE(Array, UInt32),
    REFLEX_TYPE(Array, Int32),
    REFLEX_TYPE(Array, UInt64),
    REFLEX_TYPE(Array, Int64),
    REFLEX_TYPE(Array, Float),
    REFLEX_TYPE(Array, Double),
    // ------------- PointerArray Types --------------
    REFLEX_TYPE(PointerArray, Unknown),
    REFLEX_TYPE(PointerArray, Char),
    REFLEX_TYPE(PointerArray, UInt8),
    REFLEX_TYPE(PointerArray, Int8),
    REFLEX_TYPE(PointerArray, UInt16),
    REFLEX_TYPE(PointerArray, Int16),
    REFLEX_TYPE(PointerArray, UInt32),
    REFLEX_TYPE(PointerArray, Int32),
    REFLEX_TYPE(PointerArray, UInt64),
    REFLEX_TYPE(PointerArray, Int64),
    REFLEX_TYPE(PointerArray, Float),
    REFLEX_TYPE(PointerArray, Double),
    // ------------- Array2D Types --------------
    REFLEX_TYPE(Array2D, Unknown),
    REFLEX_TYPE(Array2D, Char),
    REFLEX_TYPE(Array2D, UInt8),
    REFLEX_TYPE(Array2D, Int8),
    REFLEX_TYPE(Array2D, UInt16),
    REFLEX_TYPE(Array2D, Int16),
    REFLEX_TYPE(Array2D, UInt32),
    REFLEX_TYPE(Array2D, Int32),
    REFLEX_TYPE(Array2D, UInt64),
    REFLEX_TYPE(Array2D, Int64),
    REFLEX_TYPE(Array2D, Float),
    REFLEX_TYPE(Array2D, Double),
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
 * @brief This function used for serialize/deserialize specific type and field
 */
typedef Reflex_Result (*Reflex_SerializeFn)(Reflex* reflex, void* buf, void* value, Reflex_Type type, Reflex_LenType len, Reflex_LenType mLen);
typedef Reflex_Result (*Reflex_DeserializeFn)(Reflex* reflex, void* buf, void* value, Reflex_Type type, Reflex_LenType len, Reflex_LenType mLen);

typedef union {
    Reflex_SerializeFn      fn[Reflex_PrimaryType_Length];
    struct {
        Reflex_SerializeFn  serializeUnknown;
        Reflex_SerializeFn  serializeChar;
        Reflex_SerializeFn  serializeUInt8;
        Reflex_SerializeFn  serializeInt8;
        Reflex_SerializeFn  serializeUInt16;
        Reflex_SerializeFn  serializeInt16;
        Reflex_SerializeFn  serializeUInt32;
        Reflex_SerializeFn  serializeInt32;
        Reflex_SerializeFn  serializeUInt64;
        Reflex_SerializeFn  serializeInt64;
        Reflex_SerializeFn  serializeFloat;
        Reflex_SerializeFn  serializeDouble;
    };
} Reflex_SerializeFunctions;

typedef union {
    const Reflex_SerializeFunctions*      Category[Reflex_Category_Length];
    struct {
        const Reflex_SerializeFunctions*  Primary;
        const Reflex_SerializeFunctions*  Pointer;
        const Reflex_SerializeFunctions*  Array;
        const Reflex_SerializeFunctions*  PointerArray;
        const Reflex_SerializeFunctions*  Array2D;
    };
} Reflex_SerializeDriver;

typedef union {
    Reflex_DeserializeFn      fn[Reflex_PrimaryType_Length];
    struct {
        Reflex_DeserializeFn  deserializeUnknown;
        Reflex_DeserializeFn  deserializeChar;
        Reflex_DeserializeFn  deserializeUInt8;
        Reflex_DeserializeFn  deserializeInt8;
        Reflex_DeserializeFn  deserializeUInt16;
        Reflex_DeserializeFn  deserializeInt16;
        Reflex_DeserializeFn  deserializeUInt32;
        Reflex_DeserializeFn  deserializeInt32;
        Reflex_DeserializeFn  deserializeUInt64;
        Reflex_DeserializeFn  deserializeInt64;
        Reflex_DeserializeFn  deserializeFloat;
        Reflex_DeserializeFn  deserializeDouble;
    };
} Reflex_DeserializeFunctions;

typedef union {
    const Reflex_DeserializeFunctions*      Category[Reflex_Category_Length];
    struct {
        const Reflex_DeserializeFunctions*  Primary;
        const Reflex_DeserializeFunctions*  Pointer;
        const Reflex_DeserializeFunctions*  Array;
        const Reflex_DeserializeFunctions*  PointerArray;
        const Reflex_DeserializeFunctions*  Array2D;
    };
} Reflex_DeserializeDriver;

typedef struct {
    Reflex_LenType               Len;       /*< Used for Array Length, Array2D Row Length, Sizeof Struct, Sizeof Enum, Sizeof Union*/
    Reflex_LenType               MLen;      /*< Used for Array2D Columns Length */
    union {
        Reflex_Type              Type;
        Reflex_Type_BitFields    Fields;
    };
} Reflex_TypeParams;

#define REFLEX_TYPE_PARAMS(TY, LEN, MLEN)           { .Len = LEN, .MLen = MLEN, .Type = TY }
#define REFLEX_TYPE_PARAMS_LEN(ARR)                 (sizeof(ARR) / sizeof(ARR[0]))

typedef enum {
  Reflex_SizeType_Normal        = 0,
  Reflex_SizeType_Packed        = 1,
} Reflex_SizeType;

typedef enum {
    Reflex_FunctionMode_Callback        = 0,
    Reflex_FunctionMode_Driver          = 1,
} Reflex_FunctionMode;

typedef enum {
    Reflex_FormatMode_Param             = 0,
    Reflex_FormatMode_Primary           = 1,
} Reflex_FormatMode;

typedef struct {
    union {
        const uint8_t*                  PrimaryFmt;
        const Reflex_TypeParams*        Fmt;
    };
    Reflex_LenType                      Len;                            /**< number of variables, for PrimaryFmt is optional */
    uint8_t                             FormatMode          : 1;        /**< Reflex Serialize FunctionMode */
    uint8_t                             Reserved            : 7;
} Reflex_Schema;

struct __Reflex {
    void*                               Args;
    void*                               Buffer;
    void*                               Obj;
    union {
        const Reflex_SerializeDriver*   Serialize;
        Reflex_SerializeFn              serializeCb;
    };
    union {
        const Reflex_DeserializeDriver* Deserialize;
        Reflex_DeserializeFn            deserializeCb;
    };
    const Reflex_Schema*                Schema;
    Reflex_LenType                      VariableIndex;
    uint8_t                             FunctionMode        : 1;        /**< Reflex Serialize FunctionMode */
    uint8_t                             Reserved            : 7;
};

// --------- Serialize Functions ------------
Reflex_Result Reflex_serializePrimary(Reflex* reflex, void* obj);
Reflex_Result Reflex_serializeParam(Reflex* reflex, void* obj);
Reflex_Result Reflex_serialize(Reflex* reflex, void* obj);
// -------- Deserialize Functions -----------
Reflex_Result Reflex_deserializePrimary(Reflex* reflex, void* obj);
Reflex_Result Reflex_deserializeParam(Reflex* reflex, void* obj);
Reflex_Result Reflex_deserialize(Reflex* reflex, void* obj);
// ----------- Utils Functions --------------
Reflex_LenType Reflex_getVariableIndex(Reflex* reflex);
Reflex_LenType Reflex_getVariablesLength(Reflex* reflex);
void*          Reflex_getMainVariable(Reflex* reflex);

Reflex_LenType Reflex_size(Reflex* reflex, Reflex_SizeType type);
Reflex_LenType Reflex_sizeNormal(Reflex* reflex);
Reflex_LenType Reflex_sizePacked(Reflex* reflex);
Reflex_LenType Reflex_sizeNormalPrimary(Reflex* reflex);
Reflex_LenType Reflex_sizePackedPrimary(Reflex* reflex);

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif // _REFLEX_H_

