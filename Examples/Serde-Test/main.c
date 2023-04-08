#include <stdio.h>

#include "Serde.h"

#define PRINTLN						puts
#define PRINTF						printf

uint32_t assertResult = 0;
#define assert(TYPE, ...)			if((assertResult = Assert_ ##TYPE (__VA_ARGS__, __LINE__)) != Str_Ok) return assertResult

#define ARRAY_LEN(ARR)			    (sizeof(ARR) / sizeof(ARR[0]))

#define setValue(IDX, TYPE, VAL)    values[IDX].Type = Param_ValueType_ ##TYPE;\
                                    values[IDX].TYPE = VAL

typedef uint32_t Test_Result;
typedef Test_Result (*Test_Fn)(void);

Test_Result Assert_Str(const char* str1, const char* str2, uint16_t line);
void Result_print(Test_Result result);


Test_Result Test_Primary_Serielize(void);
Test_Result Test_Serielize(void);

const Test_Fn Tests[] = {
    Test_Primary_Serielize,
    Test_Serielize,
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
		PRINTF("---------------------------------------------- \r\n", testIndex);
	}
	PRINTLN("Test Done\r\n");
	PRINTF("Tests Errors: %d\r\n", countTestError);
}

// ---------------- Test Serialize ---------------
void*       addressMap[12] = {0};
uint8_t     addressMapIndex = 0;

void Serde_checkAddress(void* out, void* value, Serde_Type type, Serde_LenType len, Serde_LenType len2) {
    if (addressMap[addressMapIndex] != value) {
        PRINTF("Address not match: %X != %X\n", addressMap[addressMapIndex], value);
    }
    addressMapIndex++;
}

const Serde_SerializeFn SERIALIZE[] = {
    Serde_checkAddress,
    Serde_checkAddress,
    Serde_checkAddress,
    Serde_checkAddress,
    Serde_checkAddress,
    Serde_checkAddress,
    Serde_checkAddress,
    Serde_checkAddress,
    Serde_checkAddress,
    Serde_checkAddress,
    Serde_checkAddress,
    Serde_checkAddress,
};

typedef struct {
    uint32_t    X;
    float       Y;
    uint8_t     Z;
} PrimaryTemp1;
const uint8_t PrimaryTemp1_FMT[] = {
    Serde_Type_Primary_UInt32,
    Serde_Type_Primary_Float,
    Serde_Type_Primary_UInt8,
    Serde_Type_Unknown,
};

typedef struct {
    uint16_t    X;
    float       Y;
    uint8_t     Z;
} PrimaryTemp2;
const uint8_t PrimaryTemp2_FMT[] = {
    Serde_Type_Primary_UInt16,
    Serde_Type_Primary_Float,
    Serde_Type_Primary_UInt8,
    Serde_Type_Unknown,
};

typedef struct {
    uint16_t    A;
    float       B;
    uint8_t     C;
    uint8_t     D;
} PrimaryTemp3;
const uint8_t PrimaryTemp3_FMT[] = {
    Serde_Type_Primary_UInt16,
    Serde_Type_Primary_Float,
    Serde_Type_Primary_UInt8,
    Serde_Type_Primary_UInt8,
    Serde_Type_Unknown,
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
    Serde_Type_Primary_UInt8,
    Serde_Type_Primary_UInt32,
    Serde_Type_Primary_UInt16,
    Serde_Type_Primary_Int16,
    Serde_Type_Primary_Int8,
    Serde_Type_Primary_Int64,
    Serde_Type_Primary_UInt64,
    Serde_Type_Primary_Float,
    Serde_Type_Primary_Double,
    Serde_Type_Primary_UInt8,
    Serde_Type_Primary_UInt16,
    Serde_Type_Primary_UInt32,
    Serde_Type_Unknown,
};


Test_Result Test_Primary_Serielize(void) {
    PrimaryTemp1 temp1;
    PrimaryTemp2 temp2;
    PrimaryTemp3 temp3;
    PrimaryTemp4 temp4;

    addressMapIndex = 0;
    addressMap[0] = &temp1.X;
    addressMap[1] = &temp1.Y;
    addressMap[2] = &temp1.Z;
    Serde_serializePrimary(NULL, PrimaryTemp1_FMT, &temp1, SERIALIZE);

    addressMapIndex = 0;
    addressMap[0] = &temp2.X;
    addressMap[1] = &temp2.Y;
    addressMap[2] = &temp2.Z;
    Serde_serializePrimary(NULL, PrimaryTemp2_FMT, &temp2, SERIALIZE);

    addressMapIndex = 0;
    addressMap[0] = &temp3.A;
    addressMap[1] = &temp3.B;
    addressMap[2] = &temp3.C;
    addressMap[3] = &temp3.D;
    Serde_serializePrimary(NULL, PrimaryTemp3_FMT, &temp3, SERIALIZE);

    addressMapIndex = 0;
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
    Serde_serializePrimary(NULL, PrimaryTemp4_FMT, &temp4, SERIALIZE);

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
const Serde_TypeParams Model1_FMT[] = {
    SERDE_TYPE_PARAMS(Serde_Type_Pointer_UInt8, 0, 0),
    SERDE_TYPE_PARAMS(Serde_Type_Array_Char, 10, 0),
    SERDE_TYPE_PARAMS(Serde_Type_Primary_UInt32, 0, 0),
    SERDE_TYPE_PARAMS(Serde_Type_PointerArray_Char, 4, 0),
    SERDE_TYPE_PARAMS(Serde_Type_2DArray_Char, 4, 32),
};
const uint8_t Model1_FMT_Len = sizeof(Model1_FMT) / sizeof(Model1_FMT[0]);

Test_Result Test_Serielize(void) {
    Model1 temp1;

    addressMapIndex = 0;
    addressMap[0] = &temp1.V0;
    addressMap[1] = &temp1.V1;
    addressMap[2] = &temp1.V2;
    addressMap[3] = &temp1.V3;
    addressMap[4] = &temp1.V4;
    Serde_serialize(NULL, Model1_FMT, Model1_FMT_Len, &temp1, SERIALIZE);

    return 0;
}

void Result_print(Test_Result result) {
    PRINTF("Line: %u, Index: %u\r\n", result >> 16, result & 0xFFFF);
}

