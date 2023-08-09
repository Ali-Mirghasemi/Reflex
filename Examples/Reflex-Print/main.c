#include <stdio.h>
#include <string.h>
#include "Reflex.h"

typedef struct {
    int32_t id;
    char    name[50];
    float   price;
} Product;
// Define the Product data structure
const Reflex_TypeParams productParams[] = {
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Int32),
    REFLEX_TYPE_PARAMS(Reflex_Type_Array_Char, 50),
    REFLEX_TYPE_PARAMS(Reflex_Type_Primary_Float),
};
const Reflex_Schema productSchema = REFLEX_SCHEMA(Reflex_FormatMode_Param, productParams);

Reflex_Result Reflex_onFiled(Reflex* reflex, void* value, const Reflex_TypeParams* fmt) {
    // Do whatever you want to do with your fields, for example simple serialize
    switch (fmt->Type) {
        case Reflex_Type_Primary_Int32:
            printf("%d, ", *(int32_t*) value);
            break;
        case Reflex_Type_Array_Char:
            printf("%s, ", (char*) value);
            break;
        case Reflex_Type_Primary_Float:
            printf("%f, ", *(float*) value);
            break;
        default:
            // Not handled
            break;
    }

    return REFLEX_OK;
}

int main() {
    // Define Reflex handler instance
    Reflex reflex;

    // Create a Product instance
    Product product;
    product.id = 1;
    strcpy(product.name, "Example Product");
    product.price = 19.99;

    // Initialize reflex
    Reflex_init(&reflex, &productSchema);
    Reflex_setCallback(&reflex, Reflex_onFiled);

    // Serialize a Product with scan function
    Reflex_scan(&reflex, &product);

    return 0;
}
