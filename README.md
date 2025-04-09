# jclass
Library for building class files with C
Most functions have the same name as their JVM assembly counterpart with the exceptions of drem (j_drem) and ldiv (j_ldiv) because one of the std C libs have those defined

| JVM Feature                         | Version 1  | Notes                                                   |
|-------------------------------------|------------|---------------------------------------------------------|
| Magic Number / Version Header       | ✅         | Hardcoded as Java 8 (major_version = 52)                |
| Constant Pool (Basic Types)         | ✅         | |
| Constant Pool (Refs)                | ✅         | Class, String, FieldRef, MethodRef, InterfaceMethodRef  |
| Basic Bytecode Instructions         | ✅         | Most standard opcodes supported                         |
| Field and Method Definitions        | ✅         | Includes access flags and attributes                    |
| Code Attribute                      | ✅         | Supports max_stack, max_locals, bytecode, exceptions    |
| Exception Table                     | ✅         | Basic support (can emit empty table)                    |
| Interfaces                          | ✅         | Can declare interface implementation                    |
| Custom Attributes                   | ✅         | Can emit raw attributes manually                        |
| `tableswitch` / `lookupswitch`      | ❌         | Commented out, needs testing                            |
| StackMapTable                       | ❌         | Required for Java 7+ verification                       |
| LineNumberTable                     | ❌         | Needed for debugging                                    |
| LocalVariableTable                  | ❌         | Needed for debugging/local variable scopes              |
| Annotations                         | ❌         | No support for runtime or compile-time annotations      |
| Generic Type Signatures             | ❌         | Signature attribute not implemented                     |
| BootstrapMethods (`invokedynamic`)  | ❌         | Opcode is present, but bootstrap method support missing |
| InnerClasses                        | ❌         | No InnerClasses attribute support                       |
| Enum Support                        | ❌         | No handling for enum-related attributes                 |
| Module System (Java 9+)             | ❌         | No `module-info.class` or module attributes             |
| Debug Metadata (`SourceFile`, etc.) | ❌         | Not implemented                                         |




[jclass wiki](https://github.com/hydrophobis/jclass/wiki/Home) (WIP)

Doesnt have any dependencies other than the std C lib
