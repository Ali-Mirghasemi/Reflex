/**
 * @file main.c
 * @author Ali Mirghasemi (ali.mirghasemi1376@gmai.com)
 * @brief This examples shows how to serialize a structure into string
 * @version 0.1
 * @date 2023-07-04
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdio.h>
#include <stdint.h>
#include "Reflex.h"

#define PRINTLN						puts
#define PRINTF						printf

typedef struct {
    char*       Text;
    int32_t     Size;
    int32_t     Index;
} StrBuf;

/* --------------------------- Serialize Functions ----------------------- */
#define IMPL_PRIMARY(TY, FMT)   Reflex_Result ReflexStr_Serialize_Primary_ ##TY(Reflex* reflex, void* value, const Reflex_TypeParams* fmt) { \
                                    StrBuf* strBuf = (StrBuf*) Reflex_getBuffer(reflex); \
                                    strBuf->Index += snprintf(&strBuf->Text[strBuf->Index], strBuf->Size - strBuf->Index, FMT,  *(TY*) value); \
                                    return REFLEX_OK; \
                                }

IMPL_PRIMARY(size_t, "%llX,");
IMPL_PRIMARY(char, "%c,");
IMPL_PRIMARY(uint8_t, "%u,");
IMPL_PRIMARY(uint16_t, "%u,");
IMPL_PRIMARY(uint32_t, "%u,");
IMPL_PRIMARY(uint64_t, "%llu,");
IMPL_PRIMARY(int8_t, "%d,");
IMPL_PRIMARY(int16_t, "%d,");
IMPL_PRIMARY(int32_t, "%d,");
IMPL_PRIMARY(int64_t, "%lld,");
IMPL_PRIMARY(float, "%f,");
IMPL_PRIMARY(double, "%f,");

#define IMPL_STR(CAT, TY)       Reflex_Result ReflexStr_Serialize_ ##CAT ##_ ##TY(Reflex* reflex, void* value, const Reflex_TypeParams* fmt) { \
                                    StrBuf* strBuf = (StrBuf*) Reflex_getBuffer(reflex); \
                                    strBuf->Index += snprintf(&strBuf->Text[strBuf->Index], strBuf->Size - strBuf->Index, "%s,",  (TY*) value); \
                                    return REFLEX_OK; \
                                }

#define IMPL_ARRAY(TY, FMT)     Reflex_Result ReflexStr_Serialize_Array_ ##TY(Reflex* reflex, void* value, const Reflex_TypeParams* fmt) { \
                                    TY* arr = (TY*) value; \
                                    StrBuf* strBuf = (StrBuf*) Reflex_getBuffer(reflex); \
                                    strBuf->Index += snprintf(&strBuf->Text[strBuf->Index], strBuf->Size - strBuf->Index, "["); \
                                    Reflex_LenType len = fmt->Len; \
                                    while (len-- > 0) { \
                                        strBuf->Index += snprintf(&strBuf->Text[strBuf->Index], strBuf->Size - strBuf->Index, FMT,  *arr++); \
                                    } \
                                    strBuf->Index += snprintf(&strBuf->Text[strBuf->Index], strBuf->Size - strBuf->Index, "],"); \
                                    return REFLEX_OK; \
                                }

IMPL_ARRAY(size_t, "%X,");
IMPL_STR(Array, char);
IMPL_ARRAY(uint8_t, "0x%02X,");
IMPL_ARRAY(uint16_t, "%u,");
IMPL_ARRAY(uint32_t, "%u,");
IMPL_ARRAY(uint64_t, "%u,");
IMPL_ARRAY(int8_t, "%d,");
IMPL_ARRAY(int16_t, "%d,");
IMPL_ARRAY(int32_t, "%d,");
IMPL_ARRAY(int64_t, "%d,");
IMPL_ARRAY(float, "%f,");
IMPL_ARRAY(double, "%f,");

const Reflex_ScanFunctions STR_SERIALIZE_PRIMARY = {
    .fnUnknown   = ReflexStr_Serialize_Primary_size_t,
    .fnChar      = ReflexStr_Serialize_Primary_char,
    .fnUInt8     = ReflexStr_Serialize_Primary_uint8_t,
    .fnUInt16    = ReflexStr_Serialize_Primary_uint16_t,
    .fnUInt32    = ReflexStr_Serialize_Primary_uint32_t,
    .fnUInt64    = ReflexStr_Serialize_Primary_uint64_t,
    .fnInt8      = ReflexStr_Serialize_Primary_int8_t,
    .fnInt16     = ReflexStr_Serialize_Primary_int16_t,
    .fnInt32     = ReflexStr_Serialize_Primary_int32_t,
    .fnInt64     = ReflexStr_Serialize_Primary_int64_t,
    .fnFloat     = ReflexStr_Serialize_Primary_float,
    .fnDouble    = ReflexStr_Serialize_Primary_double,
};

const Reflex_ScanFunctions STR_SERIALIZE_ARRAY = {
    .fnUnknown   = ReflexStr_Serialize_Array_size_t,
    .fnChar      = ReflexStr_Serialize_Array_char,
    .fnUInt8     = ReflexStr_Serialize_Array_uint8_t,
    .fnUInt16    = ReflexStr_Serialize_Array_uint16_t,
    .fnUInt32    = ReflexStr_Serialize_Array_uint32_t,
    .fnUInt64    = ReflexStr_Serialize_Array_uint64_t,
    .fnInt8      = ReflexStr_Serialize_Array_int8_t,
    .fnInt16     = ReflexStr_Serialize_Array_int16_t,
    .fnInt32     = ReflexStr_Serialize_Array_int32_t,
    .fnInt64     = ReflexStr_Serialize_Array_int64_t,
    .fnFloat     = ReflexStr_Serialize_Array_float,
    .fnDouble    = ReflexStr_Serialize_Array_double,
};

const Reflex_ScanDriver STR_SERIALIZE = {
    .Primary = &STR_SERIALIZE_PRIMARY,
    .Array   = &STR_SERIALIZE_ARRAY,
};

/* ---------------------------  Structures ----------------------- */
typedef struct {
    int32_t     X;
    float       P;
    uint8_t     Z;
} PrimaryModelA;

const uint8_t PrimaryModelA_FMT[] = {
    Reflex_Type_Primary_Int32,
    Reflex_Type_Primary_Float,
    Reflex_Type_Primary_UInt8,
};
const Reflex_Schema PrimaryModelA_SCHEMA = REFLEX_SCHEMA(Reflex_FormatMode_Primary, PrimaryModelA_FMT);
typedef struct {
    int32_t     Id;
    char        Name[32];
    uint8_t     Secret[8];
    float       Price;
} ModelA;

const Reflex_TypeParams ModelA_FMT[] = {
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Int32),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array_Char, 32),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array_UInt8, 8),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Float),
};
const Reflex_Schema ModelA_SCHEMA = REFLEX_SCHEMA(Reflex_FormatMode_Param, ModelA_FMT);
typedef struct {
    uint8_t  A;
    char     Array[20];
    float    C;
}ModelB;


const Reflex_TypeParams ModelB_FMT[] = {
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_UInt8),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array_Char, 20),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Float, 8),
};
const Reflex_Schema ModelB_SCHEMA = REFLEX_SCHEMA(Reflex_FormatMode_Param, ModelB_FMT);


void serializeModel(StrBuf* buf, const Reflex_Schema* schema, void* obj);

int main()
{
    char txt[128];
    StrBuf strBuf = {
        .Text = txt,
        .Size = sizeof(txt) - 1,
        .Index = 0,
    };

    PrimaryModelA primaryModelA = {
        .X = 120,
        .P = 2.5f,
        .Z = 45,
    };

    serializeModel(&strBuf, &PrimaryModelA_SCHEMA, &primaryModelA);
    PRINTLN(txt);

    ModelA modelA = {
        .Id = 112,
        .Name = "King",
        .Secret = {0xAA, 0xBB, 0xCC, 0xDD, 0x11, 0x22, 0x33, 0x44},
        .Price = 4.5f,
    };
    serializeModel(&strBuf, &ModelA_SCHEMA, &modelA);
    PRINTLN(txt);

    ModelB modelB = {
        .A = 12,
        .Array = "hello Array",
        .C = 12.5
    };
    serializeModel(&strBuf, &ModelB_SCHEMA, &modelB);
    PRINTLN(txt);
}

/* ------------------------------ Implement Functions --------------------------- */
void serializeModel(StrBuf* buf, const Reflex_Schema* schema, void* obj) {
    Reflex reflex;

    Reflex_setBuffer(&reflex, buf);
    Reflex_setDriver(&reflex, &STR_SERIALIZE);

    reflex.Schema = schema;
    buf->Index = 0;
    Reflex_scan(&reflex, obj);
}
