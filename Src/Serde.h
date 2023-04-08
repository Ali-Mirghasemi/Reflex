/**
 * @file Serde.h
 * @author Ali Mirghasemi (ali.mirghasemi1376@gmail.com)
 * @brief This library help user to serialize and deserialize objects in c languages
 * @version 0.1
 * @date 2023-04-08
 *
 * @copyright Copyright (c) 2023
 *
 */


#ifndef _SERDE_H_
#define _SERDE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define SERDE_VER_MAJOR       0
#define SERDE_VER_MINOR       1
#define SERDE_VER_FIX         0

#include <stdint.h>

/********************************************************************************************/
/*                                     Configuration                                        */
/********************************************************************************************/

typedef int16_t Serde_LenType;

/********************************************************************************************/

/**
 * @brief supported param categories
 */
typedef enum {
    Serde_Category_Primary,       /**< Primary types, ex: u8,u16,char,float,... */
    Serde_Category_Pointer,       /**< pointer mode for primary types */
    Serde_Category_Array,         /**< array mode for primary types, required length */
    Serde_Category_PointerArray,  /**< array of pointers for primary types, required length */
    Serde_Category_2DArray,       /**< 2D array of primary types */
    Serde_Category_Length,
} Serde_Category;
/**
 * @brief supported primary types
 */
typedef enum {
    Serde_PrimaryType_Unknown,
    Serde_PrimaryType_Char,
    Serde_PrimaryType_UInt8,
    Serde_PrimaryType_Int8,
    Serde_PrimaryType_UInt16,
    Serde_PrimaryType_Int16,
    Serde_PrimaryType_UInt32,
    Serde_PrimaryType_Int32,
    Serde_PrimaryType_UInt64,
    Serde_PrimaryType_Int64,
    Serde_PrimaryType_Float,
    Serde_PrimaryType_Double,
    Serde_PrimaryType_Length,
} Serde_PrimaryType;

#define SERDE_TYPE(CAT, TY)         Serde_Type_ ##CAT ##_ ##TY = Serde_Category_ ##CAT << 5 | Serde_PrimaryType_ ##TY

typedef enum {
    // ------------- Primary Types --------------
    SERDE_TYPE(Primary, Unknown),
    SERDE_TYPE(Primary, Char),
    SERDE_TYPE(Primary, UInt8),
    SERDE_TYPE(Primary, Int8),
    SERDE_TYPE(Primary, UInt16),
    SERDE_TYPE(Primary, Int16),
    SERDE_TYPE(Primary, UInt32),
    SERDE_TYPE(Primary, Int32),
    SERDE_TYPE(Primary, UInt64),
    SERDE_TYPE(Primary, Int64),
    SERDE_TYPE(Primary, Float),
    SERDE_TYPE(Primary, Double),
    // ------------- Pointer Types --------------
    SERDE_TYPE(Pointer, Unknown),
    SERDE_TYPE(Pointer, Char),
    SERDE_TYPE(Pointer, UInt8),
    SERDE_TYPE(Pointer, Int8),
    SERDE_TYPE(Pointer, UInt16),
    SERDE_TYPE(Pointer, Int16),
    SERDE_TYPE(Pointer, UInt32),
    SERDE_TYPE(Pointer, Int32),
    SERDE_TYPE(Pointer, UInt64),
    SERDE_TYPE(Pointer, Int64),
    SERDE_TYPE(Pointer, Float),
    SERDE_TYPE(Pointer, Double),
    // ------------- Array Types --------------
    SERDE_TYPE(Array, Unknown),
    SERDE_TYPE(Array, Char),
    SERDE_TYPE(Array, UInt8),
    SERDE_TYPE(Array, Int8),
    SERDE_TYPE(Array, UInt16),
    SERDE_TYPE(Array, Int16),
    SERDE_TYPE(Array, UInt32),
    SERDE_TYPE(Array, Int32),
    SERDE_TYPE(Array, UInt64),
    SERDE_TYPE(Array, Int64),
    SERDE_TYPE(Array, Float),
    SERDE_TYPE(Array, Double),
    // ------------- PointerArray Types --------------
    SERDE_TYPE(PointerArray, Unknown),
    SERDE_TYPE(PointerArray, Char),
    SERDE_TYPE(PointerArray, UInt8),
    SERDE_TYPE(PointerArray, Int8),
    SERDE_TYPE(PointerArray, UInt16),
    SERDE_TYPE(PointerArray, Int16),
    SERDE_TYPE(PointerArray, UInt32),
    SERDE_TYPE(PointerArray, Int32),
    SERDE_TYPE(PointerArray, UInt64),
    SERDE_TYPE(PointerArray, Int64),
    SERDE_TYPE(PointerArray, Float),
    SERDE_TYPE(PointerArray, Double),
    // ------------- 2DArray Types --------------
    SERDE_TYPE(2DArray, Unknown),
    SERDE_TYPE(2DArray, Char),
    SERDE_TYPE(2DArray, UInt8),
    SERDE_TYPE(2DArray, Int8),
    SERDE_TYPE(2DArray, UInt16),
    SERDE_TYPE(2DArray, Int16),
    SERDE_TYPE(2DArray, UInt32),
    SERDE_TYPE(2DArray, Int32),
    SERDE_TYPE(2DArray, UInt64),
    SERDE_TYPE(2DArray, Int64),
    SERDE_TYPE(2DArray, Float),
    SERDE_TYPE(2DArray, Double),
} Serde_Type;

#define Serde_Type_Unknown          Serde_Type_Primary_Unknown

#define SERDE_TYPE_PRIMARY_MASK     0x1F
#define SERDE_TYPE_CATEGORY_MASK    0xE0
typedef union {
    uint8_t         Type;
    struct {
        uint8_t     Primary     : 5;
        uint8_t     Category    : 3;
    };
} Serde_Type_BitFields;

typedef void (*Serde_SerializeFn)(void* out, void* value, Serde_Type type, Serde_LenType len, Serde_LenType len2);

typedef void* (*Serde_AlignAddressFn)(void* pValue, Serde_Type type, Serde_LenType len, Serde_LenType len2);
typedef void* (*Serde_MoveAddressFn)(void* pValue, Serde_Type type, Serde_LenType len, Serde_LenType len2);
/**
 * @brief This struct hold helper functions for manage serialize and deserialize
 */
typedef struct {
    Serde_AlignAddressFn    alignAddress;
    Serde_MoveAddressFn     moveAddress;
} Serde_Type_Helper;


void Serde_serializePrimary(void* out, const uint8_t* fmt, void* obj, Serde_SerializeFn* serialize);


#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif // _SERDE_H_
