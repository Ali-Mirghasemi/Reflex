#include <stdio.h>

#include "Reflex.h"

#define PRINTLN						puts
#define PRINTF						printf

uint32_t assertResult = 0;
#define assert(TYPE, ...)			if((assertResult = Assert_ ##TYPE (__VA_ARGS__, __LINE__)) != 0) return assertResult

#define ARRAY_LEN(ARR)			    (sizeof(ARR) / sizeof(ARR[0]))

#define setValue(IDX, TYPE, VAL)    values[IDX].Type = Param_ValueType_ ##TYPE;\
                                    values[IDX].TYPE = VAL

typedef uint32_t Test_Result;
typedef Test_Result (*Test_Fn)(void);

Test_Result Assert_Str(const char* str1, const char* str2, uint16_t line);
Test_Result Assert_Num(int32_t val1, int32_t val2, uint16_t line);
void Result_print(Test_Result result);


Test_Result Test_Primary_Serielize(void);
Test_Result Test_Serde(void);
Test_Result Test_Size(void);

const Test_Fn Tests[] = {
    Test_Primary_Serielize,
    Test_Serde,
    Test_Size,
};
const uint32_t Tests_Len = sizeof(Tests) / sizeof(Tests[0]);

int main()
{
    int testIndex;
	int countTestError = 0;
	Test_Result res;

	for (testIndex = 0; testIndex < Tests_Len; testIndex++) {
		PRINTF("---------------- Beginning Test[%d]------------ \r\n", testIndex);
		res = Tests[testIndex]();
		PRINTF("Test Result: %s\r\n", res ? "Error" : "Ok");
		if (res) {
			Result_print(res);
			countTestError++;
		}
		PRINTLN("---------------------------------------------- \r\n");
	}
	PRINTLN("Test Done\r\n");
	PRINTF("Tests Errors: %d\r\n", countTestError);
}

// ---------------- Test Serialize ---------------
Reflex_Result Reflex_checkAddress(Reflex* reflex, void* out, void* value, Reflex_Type type, Reflex_LenType len, Reflex_LenType len2) {
    void** addressMap = reflex->Args;

    if (addressMap[reflex->VariableIndex] != value) {
        PRINTF("Idx: %d, Address not match: %X != %X\r\n",
            reflex->VariableIndex,
            (uint32_t) addressMap[reflex->VariableIndex],
            (uint32_t) value
        );
    }

    return REFLEX_OK;
}
// -------------- Primary Models --------------
typedef struct {
    uint32_t    X;
    float       Y;
    uint8_t     Z;
} PrimaryTemp1;
const uint8_t PrimaryTemp1_FMT[] = {
    Reflex_Type_Primary_UInt32,
    Reflex_Type_Primary_Float,
    Reflex_Type_Primary_UInt8,
    Reflex_Type_Unknown,
};
const Reflex_Schema PrimaryTemp1_SCHEMA = {
    .PrimaryFmt = PrimaryTemp1_FMT,
    .FormatMode = Reflex_FormatMode_Primary,
};

typedef struct {
    uint16_t    X;
    float       Y;
    uint8_t     Z;
} PrimaryTemp2;
const uint8_t PrimaryTemp2_FMT[] = {
    Reflex_Type_Primary_UInt16,
    Reflex_Type_Primary_Float,
    Reflex_Type_Primary_UInt8,
    Reflex_Type_Unknown,
};
const Reflex_Schema PrimaryTemp2_SCHEMA = {
    .PrimaryFmt = PrimaryTemp2_FMT,
    .FormatMode = Reflex_FormatMode_Primary,
};

typedef struct {
    uint16_t    A;
    float       B;
    uint8_t     C;
    uint8_t     D;
} PrimaryTemp3;
const uint8_t PrimaryTemp3_FMT[] = {
    Reflex_Type_Primary_UInt16,
    Reflex_Type_Primary_Float,
    Reflex_Type_Primary_UInt8,
    Reflex_Type_Primary_UInt8,
    Reflex_Type_Unknown,
};
const Reflex_Schema PrimaryTemp3_SCHEMA = {
    .PrimaryFmt = PrimaryTemp3_FMT,
    .FormatMode = Reflex_FormatMode_Primary,
};

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
const uint8_t PrimaryTemp4_FMT[] = {
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
const Reflex_Schema PrimaryTemp4_SCHEMA = {
    .PrimaryFmt = PrimaryTemp4_FMT,
    .FormatMode = Reflex_FormatMode_Primary,
};


Test_Result Test_Primary_Serielize(void) {
    void* addressMap[64] = {0};
    Reflex reflex = {0};

    PrimaryTemp1 temp1;
    PrimaryTemp2 temp2;
    PrimaryTemp3 temp3;
    PrimaryTemp4 temp4;

    reflex.serializeCb = Reflex_checkAddress;
    reflex.deserializeCb = Reflex_checkAddress;
    reflex.FunctionMode = Reflex_FunctionMode_Callback;
    reflex.Args = addressMap;

    addressMap[0] = &temp1.X;
    addressMap[1] = &temp1.Y;
    addressMap[2] = &temp1.Z;
    reflex.Schema = &PrimaryTemp1_SCHEMA;
    Reflex_serialize(&reflex, &temp1);
    Reflex_deserialize(&reflex, &temp1);

    addressMap[0] = &temp2.X;
    addressMap[1] = &temp2.Y;
    addressMap[2] = &temp2.Z;
    reflex.Schema = &PrimaryTemp2_SCHEMA;
    Reflex_serialize(&reflex, &temp2);
    Reflex_deserialize(&reflex, &temp2);

    addressMap[0] = &temp3.A;
    addressMap[1] = &temp3.B;
    addressMap[2] = &temp3.C;
    addressMap[3] = &temp3.D;
    reflex.Schema = &PrimaryTemp3_SCHEMA;
    Reflex_serialize(&reflex, &temp3);
    Reflex_deserialize(&reflex, &temp3);

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
    Reflex_serialize(&reflex, &temp4);
    Reflex_deserialize(&reflex, &temp4);

    return 0;
}
// ----------------------------- Test Serielize ---------------------
typedef struct {
    uint8_t*        V0;
    char            V1[10];
    uint32_t        V2;
    char*           V3[4];
    char            V4[4][32];
} Model1;
const Reflex_TypeParams Model1_FMT[] = {
    REFLEX_TYPE_PARAMS(Reflex_Type_Pointer_UInt8, 0, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array_Char, 10, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_UInt32, 0, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_PointerArray_Char, 4, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array2D_Char, 4, 32),
};
const Reflex_Schema Model1_SCHEMA = {
    .Fmt = Model1_FMT,
    .Len = REFLEX_TYPE_PARAMS_LEN(Model1_FMT),
    .FormatMode = Reflex_FormatMode_Param,
};

typedef struct {
    int32_t     V0;
    char        V1[32];
    uint8_t     V2[8];
    float       V3;
} Model2;
const Reflex_TypeParams Model2_FMT[] = {
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Int32, 0, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array_Char, 32, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array_UInt8, 8, 0),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Float, 0, 0),
};
const Reflex_Schema Model2_SCHEMA = {
    .Fmt = Model2_FMT,
    .Len = REFLEX_TYPE_PARAMS_LEN(Model2_FMT),
    .FormatMode = Reflex_FormatMode_Param,
};

Test_Result Test_Serde(void) {
    void* addressMap[64] = {0};
    Reflex reflex = {0};
    Model1 temp1;
    Model2 temp2;

    reflex.serializeCb = Reflex_checkAddress;
    reflex.deserializeCb = Reflex_checkAddress;
    reflex.FunctionMode = Reflex_FunctionMode_Callback;
    reflex.Args = addressMap;

    addressMap[0] = &temp1.V0;
    addressMap[1] = &temp1.V1;
    addressMap[2] = &temp1.V2;
    addressMap[3] = &temp1.V3;
    addressMap[4] = &temp1.V4;
    reflex.Schema = &Model1_SCHEMA;
    Reflex_serialize(&reflex, &temp1);
    Reflex_deserialize(&reflex, &temp1);

    addressMap[0] = &temp2.V0;
    addressMap[1] = &temp2.V1;
    addressMap[2] = &temp2.V2;
    addressMap[3] = &temp2.V3;
    reflex.Schema = &Model2_SCHEMA;
    Reflex_serialize(&reflex, &temp2);
    Reflex_deserialize(&reflex, &temp2);

    return 0;
}
// -------------------------- Test Size -------------------------
Test_Result Test_Size(void) {
    Reflex reflex = {0};
    Model1 model1;
    Model2 model2;
    PrimaryTemp1 temp1;
    PrimaryTemp2 temp2;
    PrimaryTemp3 temp3;
    PrimaryTemp4 temp4;

    reflex.Schema = &Model1_SCHEMA;
    assert(Num, Reflex_size(&reflex, Reflex_SizeType_Normal), sizeof(model1));

    reflex.Schema = &Model2_SCHEMA;
    assert(Num, Reflex_size(&reflex, Reflex_SizeType_Normal), sizeof(model2));

    reflex.Schema = &PrimaryTemp1_SCHEMA;
    assert(Num, Reflex_size(&reflex, Reflex_SizeType_Normal), sizeof(temp1));

    reflex.Schema = &PrimaryTemp2_SCHEMA;
    assert(Num, Reflex_size(&reflex, Reflex_SizeType_Normal), sizeof(temp2));

    reflex.Schema = &PrimaryTemp3_SCHEMA;
    assert(Num, Reflex_size(&reflex, Reflex_SizeType_Normal), sizeof(temp3));

    reflex.Schema = &PrimaryTemp4_SCHEMA;
    assert(Num, Reflex_size(&reflex, Reflex_SizeType_Normal), sizeof(temp4));

    return 0;
}

void Result_print(Test_Result result) {
    PRINTF("Line: %u, Index: %u\r\n", result >> 16, result & 0xFFFF);
}

Test_Result Assert_Num(int32_t val1, int32_t val2, uint16_t line) {
    if (val1 != val2) {
        PRINTF("Assert Num: expected %d, found %d, Line: %d", val2, val1, line);
        return (Test_Result) line << 16;
    }
    else {
        return 0;
    }
}
