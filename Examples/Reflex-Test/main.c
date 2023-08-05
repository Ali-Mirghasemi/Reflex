#include <stdio.h>
#include <string.h>

#if REFLEX_ARCH != REFLEX_ARCH_64BIT && REFLEX_ARCH != REFLEX_ARCH_32BIT
	#error "Please set REFLEX_ARCH to 32 or 64 bit"
#endif

#include "Reflex.h"

#define PRINTLN						puts
#define PRINTF						printf
#define CONST_VAR_ATTR

uint32_t assertResult = 0;
#define assert(TYPE, ...)			if((assertResult = Assert_ ##TYPE (__VA_ARGS__, __LINE__)) != 0) return assertResult

#define ARRAY_LEN(ARR)			    (sizeof(ARR) / sizeof(ARR[0]))

#define setValue(IDX, TYPE, VAL)    values[IDX].Type = Param_ValueType_ ##TYPE;\
                                    values[IDX].TYPE = VAL

typedef uint32_t Test_Result;
typedef Test_Result (*Test_Fn)(void);
typedef struct {
    const char*     Name;
    Test_Fn         fn;
} TestCase;

Test_Result Assert_Str(const char* str1, const char* str2, uint16_t line);
Test_Result Assert_Num(int32_t val1, int32_t val2, uint16_t line);
void Result_print(Test_Result result);
void printTitle(uint8_t idx, const char* name);
static const char FOOTER[] CONST_VAR_ATTR = "----------------------------------------------------------------";

#if REFLEX_FORMAT_MODE_PARAM
    Test_Result Test_Param(void);
#endif
#if REFLEX_FORMAT_MODE_PRIMARY
    Test_Result Test_Primary(void);
#endif
#if REFLEX_FORMAT_MODE_OFFSET
    Test_Result Test_ParamsOffset(void);
#endif
#if REFLEX_SUPPORT_CUSTOM_TYPE_PARAMS
    Test_Result Test_CustomTypeParams(void);
#endif
#if REFLEX_SUPPORT_TYPE_COMPLEX
    Test_Result Test_ComplexType(void);
#endif
Test_Result Test_Size(void);

#define TEST_CASE_INIT(NAME)        { .Name = #NAME, .fn = NAME }

const TestCase Tests[] = {
#if REFLEX_FORMAT_MODE_PARAM
    TEST_CASE_INIT(Test_Param),
#endif
#if REFLEX_FORMAT_MODE_PRIMARY
    TEST_CASE_INIT(Test_Primary),
#endif
#if REFLEX_FORMAT_MODE_OFFSET
    TEST_CASE_INIT(Test_ParamsOffset),
#endif
#if REFLEX_SUPPORT_CUSTOM_TYPE_PARAMS
    TEST_CASE_INIT(Test_CustomTypeParams),
#endif
#if REFLEX_SUPPORT_TYPE_COMPLEX
    TEST_CASE_INIT(Test_ComplexType),
#endif
    TEST_CASE_INIT(Test_Size),
};
const uint32_t Tests_Len = sizeof(Tests) / sizeof(Tests[0]);

int main()
{
    int testIndex;
	int countTestError = 0;
	Test_Result res;

	for (testIndex = 0; testIndex < Tests_Len; testIndex++) {
		printTitle(testIndex, Tests[testIndex].Name);
		res = Tests[testIndex].fn();
		PRINTF("Test Result: %s\r\n", res ? "Error" : "Ok");
		if (res) {
			Result_print(res);
			countTestError++;
		}
		PRINTLN(FOOTER);
	}
	PRINTLN("Test Done\r\n");
	PRINTF("Tests Errors: %d\r\n", countTestError);
}

void printTitle(uint8_t idx, const char* name) {
    char tmpNum[4];
    char temp[65] = {0};
    uint8_t len = strlen(name);
    uint8_t nlen = snprintf(tmpNum, sizeof(tmpNum) - 1, " %d ", idx);
    uint8_t hlen = ((sizeof(FOOTER) - 1) - len - nlen - 1);
    uint8_t extra = hlen & 1; // for odd length
    hlen >>= 1; // divide 2
    strncpy(&temp[0], FOOTER, hlen);
    temp[hlen] = ' ';
    strncpy(&temp[hlen + 1], name, len);
    strncpy(&temp[hlen + len + 1], tmpNum, nlen);
    strncpy(&temp[hlen + len + nlen + 1], FOOTER, hlen + extra);
    temp[2 * hlen + len + nlen + 1 + extra] = '\0';
    PRINTLN(temp);
}

// ---------------- Test Driver ---------------
// define address map globally for prevent stack overflow
static void* addressMap[120] = {0};

Reflex_Result Reflex_checkAddress(Reflex* reflex, void* value, const Reflex_TypeParams* fmt) {
    void** addrMap = reflex->Args;

    if ( addrMap[Reflex_getVariableIndex(reflex)] != value
    #if REFLEX_SUPPORT_TYPE_COMPLEX
        && fmt->Fields.Primary != Reflex_PrimaryType_Complex
    #endif
    ) {
        PRINTF("Idx: %d, Address not match: %X != %X\r\n",
            Reflex_getVariableIndex(reflex),
            addrMap[Reflex_getVariableIndex(reflex)],
            value
        );
        return 1;
    }

    return REFLEX_OK;
}

// ----------------------------- Test Param ---------------------
#if REFLEX_FORMAT_MODE_PARAM
typedef struct {
    uint8_t*        V0;
    char            V1[10];
    uint32_t        V2;
    char*           V3[4];
    char            V4[4][32];
} Model1;
static const Reflex_TypeParams Model1_FMT[] CONST_VAR_ATTR = {
    REFLEX_TYPE_PARAMS(Reflex_Type_Pointer_UInt8),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array_Char, 10, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_UInt32),
    REFLEX_TYPE_PARAMS(Reflex_Type_PointerArray_Char, 4, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array2D_Char, 4, 32),
};
static const Reflex_Schema Model1_SCHEMA CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Param, Model1_FMT);

typedef struct {
    int32_t     V0;
    char        V1[32];
    uint8_t     V2[8];
    float       V3;
} Model2;
static const Reflex_TypeParams Model2_FMT[] CONST_VAR_ATTR = {
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Int32),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array_Char, 32, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array_UInt8, 8, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Float),
};
static const Reflex_Schema Model2_SCHEMA CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Param, Model2_FMT);

Test_Result Test_Param(void) {
    Reflex reflex = {0};
    Model1 temp1;
    Model2 temp2;

    Reflex_setCallback(&reflex, Reflex_checkAddress);
    Reflex_setArgs(&reflex, addressMap);

    addressMap[0] = &temp1.V0;
    addressMap[1] = &temp1.V1;
    addressMap[2] = &temp1.V2;
    addressMap[3] = &temp1.V3;
    addressMap[4] = &temp1.V4;
    reflex.Schema = &Model1_SCHEMA;
    assert(Num, Reflex_scan(&reflex, &temp1), REFLEX_OK);


    addressMap[0] = &temp2.V0;
    addressMap[1] = &temp2.V1;
    addressMap[2] = &temp2.V2;
    addressMap[3] = &temp2.V3;
    reflex.Schema = &Model2_SCHEMA;
    assert(Num, Reflex_scan(&reflex, &temp2), REFLEX_OK);


    return 0;
}
#endif
// -------------- Primary Models --------------
#if REFLEX_FORMAT_MODE_PRIMARY

typedef struct {
    uint32_t    X;
    float       Y;
    uint8_t     Z;
} PrimaryTemp1;
static const uint8_t PrimaryTemp1_FMT[] CONST_VAR_ATTR = {
    Reflex_Type_Primary_UInt32,
    Reflex_Type_Primary_Float,
    Reflex_Type_Primary_UInt8,
    Reflex_Type_Unknown,
};
static const Reflex_Schema PrimaryTemp1_SCHEMA CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Primary, PrimaryTemp1_FMT);

typedef struct {
    uint16_t    X;
    float       Y;
    uint8_t     Z;
} PrimaryTemp2;
static const uint8_t PrimaryTemp2_FMT[] CONST_VAR_ATTR = {
    Reflex_Type_Primary_UInt16,
    Reflex_Type_Primary_Float,
    Reflex_Type_Primary_UInt8,
    Reflex_Type_Unknown,
};
static const Reflex_Schema PrimaryTemp2_SCHEMA CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Primary, PrimaryTemp2_FMT);

typedef struct {
    uint16_t    A;
    float       B;
    uint8_t     C;
    uint8_t     D;
} PrimaryTemp3;
static const uint8_t PrimaryTemp3_FMT[] CONST_VAR_ATTR = {
    Reflex_Type_Primary_UInt16,
    Reflex_Type_Primary_Float,
    Reflex_Type_Primary_UInt8,
    Reflex_Type_Primary_UInt8,
    Reflex_Type_Unknown,
};
static const Reflex_Schema PrimaryTemp3_SCHEMA CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Primary, PrimaryTemp3_FMT);

typedef struct {
    uint8_t         V0;
    uint32_t        V1;
    uint16_t        V2;
    int16_t         V3;
    int8_t          V4;
    int64_t         V5;
    uint64_t        V6;
    float           V7;
    double          V8;
    uint8_t         V9;
    uint16_t        V10;
    uint32_t        V11;
} PrimaryTemp4;
static const uint8_t PrimaryTemp4_FMT[] CONST_VAR_ATTR = {
    Reflex_Type_Primary_UInt8,
    Reflex_Type_Primary_UInt32,
    Reflex_Type_Primary_UInt16,
    Reflex_Type_Primary_Int16,
    Reflex_Type_Primary_Int8,
    Reflex_Type_Primary_Int64,
    Reflex_Type_Primary_UInt64,
    Reflex_Type_Primary_Float,
    Reflex_Type_Primary_Double,
    Reflex_Type_Primary_UInt8,
    Reflex_Type_Primary_UInt16,
    Reflex_Type_Primary_UInt32,
    Reflex_Type_Unknown,
};
static const Reflex_Schema PrimaryTemp4_SCHEMA CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Primary, PrimaryTemp4_FMT);

Test_Result Test_Primary(void) {
    Reflex reflex = {0};

    PrimaryTemp1 temp1;
    PrimaryTemp2 temp2;
    PrimaryTemp3 temp3;
    PrimaryTemp4 temp4;

    Reflex_setCallback(&reflex, Reflex_checkAddress);
    Reflex_setArgs(&reflex, addressMap);

    addressMap[0] = &temp1.X;
    addressMap[1] = &temp1.Y;
    addressMap[2] = &temp1.Z;
    reflex.Schema = &PrimaryTemp1_SCHEMA;
    assert(Num, Reflex_scan(&reflex, &temp1), REFLEX_OK);


    addressMap[0] = &temp2.X;
    addressMap[1] = &temp2.Y;
    addressMap[2] = &temp2.Z;
    reflex.Schema = &PrimaryTemp2_SCHEMA;
    assert(Num, Reflex_scan(&reflex, &temp2), REFLEX_OK);


    addressMap[0] = &temp3.A;
    addressMap[1] = &temp3.B;
    addressMap[2] = &temp3.C;
    addressMap[3] = &temp3.D;
    reflex.Schema = &PrimaryTemp3_SCHEMA;
    assert(Num, Reflex_scan(&reflex, &temp3), REFLEX_OK);


    addressMap[0] = &temp4.V0;
    addressMap[1] = &temp4.V1;
    addressMap[2] = &temp4.V2;
    addressMap[3] = &temp4.V3;
    addressMap[4] = &temp4.V4;
    addressMap[5] = &temp4.V5;
    addressMap[6] = &temp4.V6;
    addressMap[7] = &temp4.V7;
    addressMap[8] = &temp4.V8;
    addressMap[9] = &temp4.V9;
    addressMap[10] = &temp4.V10;
    addressMap[11] = &temp4.V11;
    reflex.Schema = &PrimaryTemp4_SCHEMA;
    assert(Num, Reflex_scan(&reflex, &temp4), REFLEX_OK);


    return 0;
}
#endif
// ---------------------- Test Params Offset --------------------
#if REFLEX_FORMAT_MODE_OFFSET

static const Reflex_TypeParams Model1_FMT_OFFSET[] CONST_VAR_ATTR = {
    REFLEX_TYPE_PARAMS(Reflex_Type_Pointer_UInt8, 0, 0, Model1, V0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array_Char, 10, 0, Model1, V1),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_UInt32, 0, 0, Model1, V2),
    REFLEX_TYPE_PARAMS(Reflex_Type_PointerArray_Char, 4, 0, Model1, V3),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array2D_Char, 4, 32, Model1, V4),
};
static const Reflex_Schema Model1_SCHEMA_OFFSET CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Param, Model1_FMT_OFFSET);

static const Reflex_TypeParams Model2_FMT_OFFSET[] CONST_VAR_ATTR = {
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Int32, 0, 0, Model2, V0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array_Char, 32, 0, Model2, V1),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array_UInt8, 8, 0, Model2, V2),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Float, 0, 0, Model2, V3),
};
static const Reflex_Schema Model2_SCHEMA_OFFSET CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Param, Model2_FMT_OFFSET);


Test_Result Test_ParamsOffset(void) {
    Reflex reflex = {0};
    Model1 temp1;
    Model2 temp2;

    Reflex_setCallback(&reflex, Reflex_checkAddress);
    Reflex_setArgs(&reflex, addressMap);

    addressMap[0] = &temp1.V0;
    addressMap[1] = &temp1.V1;
    addressMap[2] = &temp1.V2;
    addressMap[3] = &temp1.V3;
    addressMap[4] = &temp1.V4;
    reflex.Schema = &Model1_SCHEMA_OFFSET;
    assert(Num, Reflex_scan(&reflex, &temp1), REFLEX_OK);


    addressMap[0] = &temp2.V0;
    addressMap[1] = &temp2.V1;
    addressMap[2] = &temp2.V2;
    addressMap[3] = &temp2.V3;
    reflex.Schema = &Model2_SCHEMA_OFFSET;
    assert(Num, Reflex_scan(&reflex, &temp2), REFLEX_OK);


    return 0;
}

#endif
// ---------------------- Test Custom Type Params  ----------------------
#if REFLEX_SUPPORT_CUSTOM_TYPE_PARAMS

typedef struct {
    REFLEX_TYPE_PARAMS_STRUCT();
    uint32_t        V0;
    uint8_t         V1;
} CustomTypeParams1;

typedef struct {
    Reflex_TypeParams   TypeParams;
    uint32_t            V0;
    uint8_t             V1;
    const char*         V2;
} CustomTypeParams2;

const CustomTypeParams1 Model1_CFMT[] = {
    REFLEX_TYPE_PARAMS(Reflex_Type_Pointer_UInt8),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array_Char, 10, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_UInt32),
    REFLEX_TYPE_PARAMS(Reflex_Type_PointerArray_Char, 4, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array2D_Char, 4, 32),
};
static const Reflex_Schema Model1_CSCHEMA CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Param, Model1_CFMT);

const CustomTypeParams2 Model2_CFMT[] = {
    { .TypeParams = REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Int32), },
    { .TypeParams = REFLEX_TYPE_PARAMS(Reflex_Type_Array_Char, 32, 0), },
    { .TypeParams = REFLEX_TYPE_PARAMS(Reflex_Type_Array_UInt8, 8, 0), },
    { .TypeParams = REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Float), },
};
static const Reflex_Schema Model2_CSCHEMA CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Param, Model2_CFMT);

Test_Result Test_CustomTypeParams(void) {
    Reflex reflex = {0};
    Model1 temp1;
    Model2 temp2;

    Reflex_setCallback(&reflex, Reflex_checkAddress);
    Reflex_setArgs(&reflex, addressMap);

    addressMap[0] = &temp1.V0;
    addressMap[1] = &temp1.V1;
    addressMap[2] = &temp1.V2;
    addressMap[3] = &temp1.V3;
    addressMap[4] = &temp1.V4;
    reflex.Schema = &Model1_CSCHEMA;
    assert(Num, Reflex_scan(&reflex, &temp1), REFLEX_OK);


    addressMap[0] = &temp2.V0;
    addressMap[1] = &temp2.V1;
    addressMap[2] = &temp2.V2;
    addressMap[3] = &temp2.V3;
    reflex.Schema = &Model2_CSCHEMA;
    assert(Num, Reflex_scan(&reflex, &temp2), REFLEX_OK);


    return 0;
}
#endif
// ----------------------- Custom Object ------------------------
#if REFLEX_SUPPORT_TYPE_COMPLEX

typedef struct {
    Model1      M1;
    Model2      M2;
} CModel1;
static const Reflex_TypeParams CModel1_FMT[] CONST_VAR_ATTR = {
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Complex, 0, 0, &Model1_SCHEMA),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Complex, 0, 0, &Model2_SCHEMA),
};
static const Reflex_Schema CModel1_SCHEMA CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Param, CModel1_FMT);

typedef struct {
    float       V0;
    uint8_t     V1;
    Model1      M1;
    Model2      M2[2];
    int16_t     V2;
    uint8_t     V3;
} CModel2;
static const Reflex_TypeParams CModel2_FMT[] CONST_VAR_ATTR = {
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Float),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_UInt8),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Complex, 0, 0, &Model1_SCHEMA),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array_Complex, 2, 0, &Model2_SCHEMA),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Int16),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_UInt8),
};
static const Reflex_Schema CModel2_SCHEMA CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Param, CModel2_FMT);

typedef struct {
    char            V0;
    uint16_t        V1;
    Model1          V2;
    char            V3[11];
    Model2          V4[3];
    double          V5;
    Model1*         V6;
    int16_t         V7;
    Model1          V8[2][3];
    uint8_t*        V9;
    Model2*         V10[4];
    PrimaryTemp4    V11[2];
    CModel2         V12;
} CModel3;
static const Reflex_TypeParams CModel3_FMT[] CONST_VAR_ATTR = {
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Char),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_UInt16),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Complex, 0, 0, &Model1_SCHEMA),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array_Char, 11, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array_Complex, 3, 0, &Model2_SCHEMA),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Double),
    REFLEX_TYPE_PARAMS(Reflex_Type_Pointer_Complex, 0, 0, &Model1_SCHEMA_OFFSET),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Int16),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array2D_Complex, 3, 2, &Model1_SCHEMA_OFFSET),
    REFLEX_TYPE_PARAMS(Reflex_Type_Pointer_UInt8),
    REFLEX_TYPE_PARAMS(Reflex_Type_PointerArray_Complex, 4, 0, &Model2_SCHEMA_OFFSET),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array_Complex, 2, 0, &PrimaryTemp4_SCHEMA),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Complex, 0, 0, &CModel2_SCHEMA),
};
static const Reflex_Schema CModel3_SCHEMA CONST_VAR_ATTR = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Param, CModel3_FMT);



Test_Result Test_ComplexType(void) {
    Reflex reflex = {0};
    CModel1 temp1 = {0};
    CModel2 temp2 = {0};
    CModel3 temp3 = {0};

    Reflex_setCallback(&reflex, Reflex_checkAddress);
    Reflex_setArgs(&reflex, addressMap);

    addressMap[0] = &temp1.M1.V0;
    addressMap[1] = &temp1.M1.V1;
    addressMap[2] = &temp1.M1.V2;
    addressMap[3] = &temp1.M1.V3;
    addressMap[4] = &temp1.M1.V4;
    addressMap[5] = &temp1.M2.V0;
    addressMap[6] = &temp1.M2.V1;
    addressMap[7] = &temp1.M2.V2;
    addressMap[8] = &temp1.M2.V3;
    reflex.Schema = &CModel1_SCHEMA;
    assert(Num, Reflex_scan(&reflex, &temp1), REFLEX_OK);


    addressMap[0]  = &temp2.V0;
    addressMap[1]  = &temp2.V1;
    addressMap[2]  = &temp2.M1.V0;
    addressMap[3]  = &temp2.M1.V1;
    addressMap[4]  = &temp2.M1.V2;
    addressMap[5]  = &temp2.M1.V3;
    addressMap[6]  = &temp2.M1.V4;
    addressMap[7]  = &temp2.M2[0].V0;
    addressMap[8]  = &temp2.M2[0].V1;
    addressMap[9]  = &temp2.M2[0].V2;
    addressMap[10] = &temp2.M2[0].V3;
    addressMap[11] = &temp2.M2[1].V0;
    addressMap[12] = &temp2.M2[1].V1;
    addressMap[13] = &temp2.M2[1].V2;
    addressMap[14] = &temp2.M2[1].V3;
    addressMap[15] = &temp2.V2;
    addressMap[16] = &temp2.V3;
    reflex.Schema = &CModel2_SCHEMA;
    assert(Num, Reflex_scan(&reflex, &temp2), REFLEX_OK);



    addressMap[0]  = &temp3.V0;
    addressMap[1]  = &temp3.V1;
    addressMap[2]  = &temp3.V2.V0;
    addressMap[3]  = &temp3.V2.V1;
    addressMap[4]  = &temp3.V2.V2;
    addressMap[5]  = &temp3.V2.V3;
    addressMap[6]  = &temp3.V2.V4;
    addressMap[7]  = &temp3.V3;
    addressMap[8]  = &temp3.V4[0].V0;
    addressMap[9]  = &temp3.V4[0].V1;
    addressMap[10] = &temp3.V4[0].V2;
    addressMap[11] = &temp3.V4[0].V3;
    addressMap[12] = &temp3.V4[1].V0;
    addressMap[13] = &temp3.V4[1].V1;
    addressMap[14] = &temp3.V4[1].V2;
    addressMap[15] = &temp3.V4[1].V3;
    addressMap[16] = &temp3.V4[2].V0;
    addressMap[17] = &temp3.V4[2].V1;
    addressMap[18] = &temp3.V4[2].V2;
    addressMap[19] = &temp3.V4[2].V3;
    addressMap[20] = &temp3.V5;
    addressMap[21] = &temp3.V6->V0;
    addressMap[22] = &temp3.V6->V1;
    addressMap[23] = &temp3.V6->V2;
    addressMap[24] = &temp3.V6->V3;
    addressMap[25] = &temp3.V6->V4;
    addressMap[26] = &temp3.V7;
    addressMap[27] = &temp3.V8[0][0].V0;
    addressMap[28] = &temp3.V8[0][0].V1;
    addressMap[29] = &temp3.V8[0][0].V2;
    addressMap[30] = &temp3.V8[0][0].V3;
    addressMap[31] = &temp3.V8[0][0].V4;
    addressMap[32] = &temp3.V8[0][1].V0;
    addressMap[33] = &temp3.V8[0][1].V1;
    addressMap[34] = &temp3.V8[0][1].V2;
    addressMap[35] = &temp3.V8[0][1].V3;
    addressMap[36] = &temp3.V8[0][1].V4;
    addressMap[37] = &temp3.V8[0][2].V0;
    addressMap[38] = &temp3.V8[0][2].V1;
    addressMap[39] = &temp3.V8[0][2].V2;
    addressMap[40] = &temp3.V8[0][2].V3;
    addressMap[41] = &temp3.V8[0][2].V4;
    addressMap[42] = &temp3.V8[1][0].V0;
    addressMap[43] = &temp3.V8[1][0].V1;
    addressMap[44] = &temp3.V8[1][0].V2;
    addressMap[45] = &temp3.V8[1][0].V3;
    addressMap[46] = &temp3.V8[1][0].V4;
    addressMap[47] = &temp3.V8[1][1].V0;
    addressMap[48] = &temp3.V8[1][1].V1;
    addressMap[49] = &temp3.V8[1][1].V2;
    addressMap[50] = &temp3.V8[1][1].V3;
    addressMap[51] = &temp3.V8[1][1].V4;
    addressMap[52] = &temp3.V8[1][2].V0;
    addressMap[53] = &temp3.V8[1][2].V1;
    addressMap[54] = &temp3.V8[1][2].V2;
    addressMap[55] = &temp3.V8[1][2].V3;
    addressMap[56] = &temp3.V8[1][2].V4;
    addressMap[57] = &temp3.V9;
    addressMap[58] = &temp3.V10[0]->V0;
    addressMap[59] = &temp3.V10[0]->V1;
    addressMap[60] = &temp3.V10[0]->V2;
    addressMap[61] = &temp3.V10[0]->V3;
    addressMap[62] = &temp3.V10[1]->V0;
    addressMap[63] = &temp3.V10[1]->V1;
    addressMap[64] = &temp3.V10[1]->V2;
    addressMap[65] = &temp3.V10[1]->V3;
    addressMap[66] = &temp3.V10[2]->V0;
    addressMap[67] = &temp3.V10[2]->V1;
    addressMap[68] = &temp3.V10[2]->V2;
    addressMap[69] = &temp3.V10[2]->V3;
    addressMap[70] = &temp3.V10[3]->V0;
    addressMap[71] = &temp3.V10[3]->V1;
    addressMap[72] = &temp3.V10[3]->V2;
    addressMap[73] = &temp3.V10[3]->V3;
    addressMap[74] = &temp3.V11[0].V0;
    addressMap[75] = &temp3.V11[0].V1;
    addressMap[76] = &temp3.V11[0].V2;
    addressMap[77] = &temp3.V11[0].V3;
    addressMap[78] = &temp3.V11[0].V4;
    addressMap[79] = &temp3.V11[0].V5;
    addressMap[80] = &temp3.V11[0].V6;
    addressMap[81] = &temp3.V11[0].V7;
    addressMap[82] = &temp3.V11[0].V8;
    addressMap[83] = &temp3.V11[0].V9;
    addressMap[84] = &temp3.V11[0].V10;
    addressMap[85] = &temp3.V11[0].V11;
    addressMap[86] = &temp3.V11[1].V0;
    addressMap[87] = &temp3.V11[1].V1;
    addressMap[88] = &temp3.V11[1].V2;
    addressMap[89] = &temp3.V11[1].V3;
    addressMap[90] = &temp3.V11[1].V4;
    addressMap[91] = &temp3.V11[1].V5;
    addressMap[92] = &temp3.V11[1].V6;
    addressMap[93] = &temp3.V11[1].V7;
    addressMap[94] = &temp3.V11[1].V8;
    addressMap[95] = &temp3.V11[1].V9;
    addressMap[96] = &temp3.V11[1].V10;
    addressMap[97] = &temp3.V11[1].V11;
    addressMap[98] = &temp3.V12.V0;
    addressMap[99] = &temp3.V12.V1;
    addressMap[100] = &temp3.V12.M1.V0;
    addressMap[101] = &temp3.V12.M1.V1;
    addressMap[102] = &temp3.V12.M1.V2;
    addressMap[103] = &temp3.V12.M1.V3;
    addressMap[104] = &temp3.V12.M1.V4;
    addressMap[105] = &temp3.V12.M2[0].V0;
    addressMap[106] = &temp3.V12.M2[0].V1;
    addressMap[107] = &temp3.V12.M2[0].V2;
    addressMap[108] = &temp3.V12.M2[0].V3;
    addressMap[109] = &temp3.V12.M2[1].V0;
    addressMap[110] = &temp3.V12.M2[1].V1;
    addressMap[111] = &temp3.V12.M2[1].V2;
    addressMap[112] = &temp3.V12.M2[1].V3;
    addressMap[113] = &temp3.V12.V2;
    addressMap[114] = &temp3.V12.V3;
    reflex.Schema = &CModel3_SCHEMA;
    assert(Num, Reflex_scan(&reflex, &temp3), REFLEX_OK);


    return 0;
}
#endif
// -------------------------- Test Size -------------------------
Test_Result Test_Size(void) {

    assert(Num, Reflex_size(&Model1_SCHEMA, Reflex_SizeType_Normal), sizeof(Model1));
    assert(Num, Reflex_size(&Model2_SCHEMA, Reflex_SizeType_Normal), sizeof(Model2));
    assert(Num, Reflex_size(&PrimaryTemp1_SCHEMA, Reflex_SizeType_Normal), sizeof(PrimaryTemp1));
    assert(Num, Reflex_size(&PrimaryTemp2_SCHEMA, Reflex_SizeType_Normal), sizeof(PrimaryTemp2));
    assert(Num, Reflex_size(&PrimaryTemp3_SCHEMA, Reflex_SizeType_Normal), sizeof(PrimaryTemp3));
    assert(Num, Reflex_size(&PrimaryTemp4_SCHEMA, Reflex_SizeType_Normal), sizeof(PrimaryTemp4));

#if REFLEX_SUPPORT_CUSTOM_TYPE_PARAMS
    assert(Num, Reflex_size(&Model1_CSCHEMA, Reflex_SizeType_Normal), sizeof(Model1));
    assert(Num, Reflex_size(&Model2_CSCHEMA, Reflex_SizeType_Normal), sizeof(Model2));
#endif

#if REFLEX_FORMAT_MODE_OFFSET
    assert(Num, Reflex_size(&Model1_SCHEMA_OFFSET, Reflex_SizeType_Normal), sizeof(Model1));
    assert(Num, Reflex_size(&Model2_SCHEMA_OFFSET, Reflex_SizeType_Normal), sizeof(Model2));
#endif

    return 0;
}

void Result_print(Test_Result result) {
    PRINTF("Line: %u, Index: %u\r\n", result >> 16, result & 0xFFFF);
}

Test_Result Assert_Num(int32_t val1, int32_t val2, uint16_t line) {
    if (val1 != val2) {
        PRINTF("Assert Num: expected %d, found %d, Line: %d\r\n", val2, val1, line);
        return (Test_Result) line << 16;
    }
    else {
        return 0;
    }
}
