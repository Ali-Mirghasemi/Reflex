# Reflex Library

Reflex is a powerful C library that provides reflection capabilities for C data structures. It allows you to introspect C structures at runtime, enabling you to access and manipulate the structure members dynamically. This library is particularly useful for tasks like serialization, deserialization, data validation, and generic data handling.

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [Getting Started](#getting-started)
- [Installation](#installation)
- [Usage](#usage)
- [Examples](#examples)
- [Contributing](#contributing)
- [License](#license)
- [Contact](#contact)

## Introduction

As a C library, Reflex aims to add dynamic features to the otherwise static C language. It allows developers to work with C data structures without knowing their exact layout at compile-time, making the code more flexible and adaptable to changes.

## Features

- **Runtime Reflection**: Introspect C data structures at runtime.
- **Serialization/Deserialization**: Easily convert data structures to and from various formats.
- **Data Validation**: Validate input data against the defined structure.
- **Dynamic Access**: Access structure members dynamically using their names.
- **Custom Type Support**: Define custom data types for more complex structures.
- **Portable**: Works on various architectures and platforms.
- **Easy Integration**: Simple API for easy integration into existing projects.
- **Cross-Platform**: Compatible with different operating systems.

## Getting Started

To get started with Reflex, follow these steps:

1. **Prerequisites**: Make sure you have a C compiler and standard C library installed on your system.

2. **Installation**: [See Installation section](#installation).

3. **Include Header**: Include the "Reflex.h" header file in your C source code.

4. **Define Data Structures**: Define the data structures you want to work with.

5. **Runtime Reflection**: Use Reflex to introspect and manipulate your data structures dynamically.

## Installation

To install Reflex, follow these steps:

```bash
$ git clone https://github.com/Ali-Mirghasemi/Reflex.git
$ cd Reflex
```

## Usage

To use Reflex in your C projects, include the "Reflex.h" header in your source code:

```c
#include "Reflex.h"
```

Then, define your C data structures that you want to work with and set up the appropriate schema.

## Examples

Here's a simple example demonstrating how to use Reflex:

```c
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
const Reflex_Schema productSchema = REFLEX_SCHEMA_INIT(Reflex_FormatMode_Param, productParams);

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
```

For more examples and detailed documentation, please refer to the "Examples" directory and the API documentation.

## Contributing

We welcome contributions from the community to enhance and improve the Reflex library. If you find any issues or have suggestions for new features, please submit them as GitHub issues or create pull requests.

## License

Reflex is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Contact

If you have any questions or need further assistance, you can reach us at ali.mirghasemi1376@gmail.com
