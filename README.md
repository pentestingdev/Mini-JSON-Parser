# cpp-json-parser

A minimal JSON parser written in modern C++ (C++17).

Built from scratch using recursive descent parsing.  
No external libraries.

---

## Features

- Supports:
  - Objects
  - Arrays
  - Strings
  - Numbers
  - Booleans
  - Null
-  Printing
- Recursive parsing
- Modern C++ (`std::variant`, RAII)

---

## Example

Input:

```json
{
  "name": "SWSCODES",
  "isDeveloper": true,
}
