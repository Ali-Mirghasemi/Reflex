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

typedef int16_t Reflex_LenType;

/********************************************************************************************/

/* Pre-Defined data types */
struct __Reflex;
typedef struct __Reflex Reflex;

/**
 * @brief supported param categories
 */
typedef enum {
    Reflex_Category_Primary,       /**< Primary types, ex: u8,u16,char,float,... */
    Reflex_Category_Pointer,       /**< pointer mode for primary types */
    Reflex_Category_Array,         /**< array mode for primary types, required length */
    Reflex_Category_PointerArray,  /**< array of pointers for primary types, required length */
    Reflex_Category_2DArray,       /**< 2D array of primary types */
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
    // ------------- 2DArray Types --------------
    REFLEX_TYPE(2DArray, Unknown),
    REFLEX_TYPE(2DArray, Char),
    REFLEX_TYPE(2DArray, UInt8),
    REFLEX_TYPE(2DArray, Int8),
    REFLEX_TYPE(2DArray, UInt16),
    REFLEX_TYPE(2DArray, Int16),
    REFLEX_TYPE(2DArray, UInt32),
    REFLEX_TYPE(2DArray, Int32),
    REFLEX_TYPE(2DArray, UInt64),
    REFLEX_TYPE(2DArray, Int64),
    REFLEX_TYPE(2DArray, Float),
    REFLEX_TYPE(2DArray, Double),
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

typedef void (*Reflex_SerializeFn)(Reflex* reflex, void* out, void* value, Reflex_Type type, Reflex_LenType len, Reflex_LenType len2);

typedef void* (*Reflex_AlignAddressFn)(void* pValue, Reflex_Type type, Reflex_LenType len, Reflex_LenType len2);
typedef void* (*Reflex_MoveAddressFn)(void* pValue, Reflex_Type type, Reflex_LenType len, Reflex_LenType len2);
/**
 * @brief This struct hold helper functions for manage serialize and deserialize
 */
typedef struct {
    Reflex_AlignAddressFn    alignAddress;
    Reflex_MoveAddressFn     moveAddress;
} Reflex_Type_Helper;

typedef struct {
    Reflex_LenType               Len;
    Reflex_LenType               Len2;
    union {
        Reflex_Type              Type;
        Reflex_Type_BitFields    Fields;
    };
} Reflex_TypeParams;

#define REFLEX_TYPE_PARAMS(TY, LEN, LEN2)           { .Len = LEN, .Len2 = LEN2, .Type = TY }


typedef enum {
  Reflex_SizeType_Normal        = 0,
  Reflex_SizeType_Packed        = 1,
} Reflex_SizeType;

typedef enum {
    Reflex_FunctionMode_Single          = 0,
    Reflex_FunctionMode_Array           = 1,
} Reflex_FunctionMode;

typedef enum {
    Reflex_FormatMode_Param             = 0,
    Reflex_FormatMode_Primary           = 1,
} Reflex_FormatMode;

struct __Reflex {
    void*                           Args;
    void*                           Buffer;
    union {
        const uint8_t*              PrimaryFmt;
        const Reflex_TypeParams*    Fmt;
    };
    union {
        const Reflex_SerializeFn*   SerializeFunctions;
        Reflex_SerializeFn          Serialize;
    };
    Reflex_LenType                  VariablesLength;
    uint8_t                         FunctionMode        : 1;           /**< Reflex Serialize FunctionMode */
    uint8_t                         FormatMode          : 1;           /**< Reflex Serialize FunctionMode */
};

void Reflex_scanPrimary(Reflex* reflex, void* obj);

void Reflex_scan(Reflex* reflex, void* obj);

Reflex_LenType Reflex_size(Reflex* reflex, Reflex_SizeType type);
Reflex_LenType Reflex_sizeNormal(Reflex* reflex);
Reflex_LenType Reflex_sizePacked(Reflex* reflex);
Reflex_LenType Reflex_sizeNormalPrimary(Reflex* reflex);
Reflex_LenType Reflex_sizePackedPrimary(Reflex* reflex);

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif // _REFLEX_H_
