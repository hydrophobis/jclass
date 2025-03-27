/*
    This code was created by hydrophobis on GitHub and is liscenced under GPL v3.0
    Please leave this comment in the library if you intend on using it in any context  
*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/** @brief Readability macro to mark where class generation starts */
#define J_CLASS_BEGIN {}

/** @brief Readability macro to mark where class generation ends */
#define J_CLASS_END {}

/** @brief Class or method is public (accessible from anywhere). */
#define ACC_PUBLIC 0x0001

/** @brief Class or method is private (accessible only within the defining
 * class). */
#define ACC_PRIVATE 0x0002

/** @brief Class or method is protected (accessible within the same package and
 * subclasses). */
#define ACC_PROTECTED 0x0004

/** @brief Class, method, or field is static (belongs to the class rather than
 * instances). */
#define ACC_STATIC 0x0008

/** @brief Class or method is final (cannot be subclassed or overridden). */
#define ACC_FINAL 0x0010

/** @brief Class uses the "super" keyword for method calls. */
#define ACC_SUPER 0x0020

/** @brief Method is synchronized (can only be executed by one thread at a
 * time). */
#define ACC_SYNCHRONIZED 0x0020

/** @brief Method is native (implemented in a language other than Java, such as
 * C or Assembly). */
#define ACC_NATIVE 0x0100

/** @brief Class is an interface (cannot have instance fields or non-abstract
 * methods). */
#define ACC_INTERFACE 0x0200

/** @brief Class is abstract (cannot be instantiated). */
#define ACC_ABSTRACT 0x0400

/** @brief Method is strict (follows strict floating-point calculations). */
#define ACC_STRICT 0x0800

/**
 * @brief Creates a byte from the given value
 * @param v The value to make the byte out of
 */
#define u1(v) (uint8_t)(v)

/**
 * @brief Creates 2 bytes from the given value
 * @param v The value to make the bytes out of
 */
#define u2(v) { \
    (uint8_t)(((v) >> 8) & 0xFF), \
    (uint8_t)((v) & 0xFF) \
}

/**
 * @brief Creates 4 bytes from the given value
 * @param v The value to make the bytes out of
 */
#define u4(v) { \
    (uint8_t)((v) >> 24), \
    (uint8_t)(((v) >> 16) & 0xFF), \
    (uint8_t)(((v) >> 8) & 0xFF), \
    (uint8_t)((v) & 0xFF) \
}

/** @brief The size of the buffer for bytecode, may be overidden if needed */
#define BUFFER_SIZE 65536

/** @brief Output buffer where the JVM bytecode is stored */
static uint8_t outputBuffer[BUFFER_SIZE];
/** @brief Current index of the output buffer */
static size_t outputIndex = 0;

/** 
* @brief Helper function. Do not use unless you know what you're doing
* 
*/
static void emit_byte(uint8_t b) {
    if (outputIndex >= BUFFER_SIZE) {
        fprintf(stderr, "Buffer overflow\n");
        exit(1);
    }
    outputBuffer[outputIndex++] = b;
}

/**
    * @brief Writes a byte to the buffer
    * @param v The value to place in the buffer
    * 
*/
static void emit_u1(uint8_t v) {
    emit_byte(v);
}

/** 
    * @brief Takes a uint16_t and splits it into 2 bytes (0x1024 -> { 0x10, 0x24 }), which it then writes into the buffer
    * @param v The two values to place in the buffer
    * 
*/
static void emit_u2(uint16_t v) {
    emit_byte((v >> 8) & 0xFF);
    emit_byte(v & 0xFF);
}

/**
    * @brief Splits a uint32_t into 4 bytes (0x12345678 -> { 0x12, 0x34, 0x56, 0x78 }) and writes them into the buffer
    * @param v The values to place into the buffer
    * 
*/
static void emit_u4(uint32_t v) {
    emit_byte((v >> 24) & 0xFF);
    emit_byte((v >> 16) & 0xFF);
    emit_byte((v >> 8) & 0xFF);
    emit_byte(v & 0xFF);
}

/**
    * @brief Returns the current buffer position that will be emitted to
    * 
*/
static size_t current_offset() {
    return outputIndex;
}

/**
    @brief Places 2 bytes in the given position in the buffer (0x1024 -> (0x10 -> pos) & (0x24 -> pos + 1))
    @param pos Where in the buffer to place the bytes
    @param v The two bytes to place
    
*/
static void patch_u2(size_t pos, uint16_t v) {
    if (pos + 1 >= BUFFER_SIZE) {
        fprintf(stderr, "Patch position out of range\n");
        exit(1);
    }
    outputBuffer[pos] = (v >> 8) & 0xFF;
    outputBuffer[pos + 1] = v & 0xFF;
}

/**
    @brief Places 4 bytes in the given position in the buffer (0x12345678 -> (0x12 -> pos) & (0x34 -> pos + 1) & (0x56 -> pos + 2) & (0x78 -> pos + 3))
    @param pos The 4 bytes to place in the buffer
    
*/
static void patch_u4(size_t pos, uint32_t v) {
    if (pos + 3 >= BUFFER_SIZE) {
        fprintf(stderr, "Patch position out of range\n");
        exit(1);
    }
    outputBuffer[pos] = (v >> 24) & 0xFF;
    outputBuffer[pos + 1] = (v >> 16) & 0xFF;
    outputBuffer[pos + 2] = (v >> 8) & 0xFF;
    outputBuffer[pos + 3] = v & 0xFF;
}

// ------------------------
// constant_pool macros
// ------------------------

// Global variables for constant pool state
/**
 * @brief Do not modify
 * 
 */
static size_t cp_count_offset = 0;
/**
 * @brief Keeps track of the size of the constant pool
 * 
 */
static uint16_t constant_pool_counter = 0;

/**
    @brief Begins the constant pool
    
*/
void constant_pool_start() {
    // u2 constant_pool_count
    cp_count_offset = current_offset();
    emit_u2(0); // placeholder for constant_pool_count
    constant_pool_counter = 1;
}

/**
    @brief Helper function. Do not use unless you know what you are doing
    
*/
static void increment_cp_counter() {
    constant_pool_counter++;
}

/**
    @brief Converts a char* to a constant UTF-8 string
    @param string The string to convert
    
*/
void constant_utf8(const char *string) {
    emit_u1(1);                      // u1 1 (tag)
    uint16_t len = (uint16_t)strlen(string);
    emit_u2(len);                    // u2 length
    for (uint16_t i = 0; i < len; i++) { // ..data: db string
        emit_u1((uint8_t)string[i]);
    }
    increment_cp_counter();
}

/**
    @brief Creates a constant integer
    @param value The integer to add to the constant pool
    
*/
void constant_integer(uint32_t value) {
    emit_u1(3);                      // u1 3 (tag)
    emit_u4(value);                  // u4 value
    increment_cp_counter();
}

/**
    @brief Creates a constant float
    @param value The float to add to the constant pool
    
*/
void constant_float(uint32_t value) {
    emit_u1(4);                      // u1 4 (tag)
    emit_u4(value);                  // u4 value
    increment_cp_counter();
}

/**
    @brief Creates a constant long
    @param value The 64 bit value to add to the constant pool
    
*/
void constant_long(uint64_t value) {
    emit_u1(5);                      // u1 5 (tag)
    emit_u4((uint32_t)(value >> 32)); // u4 high part
    emit_u4((uint32_t)(value & 0xFFFFFFFF)); // u4 low part
    increment_cp_counter();
}

/**
    @brief Creates a constant double
    @param value The 64 bit value to add to the constant pool
    
*/
void constant_double(uint64_t value) {
    emit_u1(6);                      // u1 6 (tag)
    emit_u4((uint32_t)(value >> 32)); // u4 high part
    emit_u4((uint32_t)(value & 0xFFFFFFFF)); // u4 low part
    increment_cp_counter();
}

/**
    @brief Creates a constant class reference
    @param name_index The index of the UTF-8 which is the class name (java/lang/Object)
    
*/
void constant_class(uint16_t name_index) {
    emit_u1(7);                      // u1 7 (tag)
    emit_u2(name_index);             // u2 name_index
    increment_cp_counter();
}

/**
    @brief Creates a constant string from a constant UTF-8
    @param string_index Constant pool index of the UTF-8
    
*/
void constant_string(uint16_t string_index) {
    emit_u1(8);                      // u1 8 (tag)
    emit_u2(string_index);           // u2 string_index
    increment_cp_counter();
}

/**
    @brief Builds a reference to a field
    @param class_index Constant pool index of the class from which the field is from
    @param name_and_type_index Constant pool index of the name and type of the field
    
*/
void constant_fieldref(uint16_t class_index, uint16_t name_and_type_index) {
    emit_u1(9);                      // u1 9 (tag)
    emit_u2(class_index);            // u2 class_index
    emit_u2(name_and_type_index);    // u2 name_and_type_index
    increment_cp_counter();
}

/**
    @brief Builds a reference to a method
    @param class_index Constant pool index of the class from which the field is from
    @param name_and_type_index Constant pool index of the name and type of the field
    
*/
void constant_methodref(uint16_t class_index, uint16_t name_and_type_index) {
    emit_u1(10);                     // u1 10 (tag)
    emit_u2(class_index);            // u2 class_index
    emit_u2(name_and_type_index);    // u2 name_and_type_index
    increment_cp_counter();
}

/**
    @brief Builds a reference to a method from an interface
    @param class_index Constant pool index of the class from which the field is from
    @param name_and_type_index Constant pool index of the name and type of the field
    
*/
void constant_interfacemethodref(uint16_t class_index, uint16_t name_and_type_index) {
    emit_u1(11);                     // u1 11 (tag)
    emit_u2(class_index);            // u2 class_index
    emit_u2(name_and_type_index);    // u2 name_and_type_index
    increment_cp_counter();
}

/**
    @brief Builds a name and type
    @param name_index Constant pool index of the name
    @param descriptor_index Constant pool index of the descriptor, a return type and params
    
*/
void constant_nameandtype(uint16_t name_index, uint16_t descriptor_index) {
    emit_u1(12);                     // u1 12 (tag)
    emit_u2(name_index);             // u2 name_index
    emit_u2(descriptor_index);       // u2 descriptor_index
    increment_cp_counter();
}

/**
    @brief Marks the end of the constant pool
    
*/
void constant_pool_end() {
    // constant_pool_count = constant_pool_counter
    patch_u2(cp_count_offset, constant_pool_counter);
    // restruc directives are not applicable in C (purge and re-structure are no-ops)
}

// ------------------------
// interfaces macros
// ------------------------

// macro interfaces {
/**
 * @brief Do not modify
 * 
 */
static size_t interfaces_count_offset = 0;
/**
 * @brief Keeps track of how many interfaces there are
 * 
 */
static uint16_t interfaces_counter = 0;

/**
    @brief Marks the start of the list of interfaces the class is using
    
*/
void interfaces_start() {
    // u2 interfaces_count
    interfaces_count_offset = current_offset();
    emit_u2(0); // placeholder for interfaces_count
    interfaces_counter = 0;
}

/**
    @brief Adds a new interface to the list
    @param interface_val No idea
    
*/
void interface_entry(uint16_t interface_val) {
    // interfaces_counter = interfaces_counter + 1
    interfaces_counter++;
    // u2 interface
    emit_u2(interface_val);
}

/**
    @brief Marks the end of the list of interfaces
    
*/
void interfaces_end() {
    patch_u2(interfaces_count_offset, interfaces_counter);
    // purge interface (no-op)
}

// ------------------------
// attributes macros
// ------------------------

/**
 * @brief Do not modify
 * 
 */
static size_t attributes_count_offset = 0;
/**
 * @brief Tracks how many attributes there are
 * 
 */
static uint16_t attributes_counter = 0;

/**
    @brief Marks the start of the list of attributes the class has
    
*/
void attributes_start() {
    // u2 attributes_count
    attributes_count_offset = current_offset();
    emit_u2(0); // placeholder for attributes_count
    attributes_counter = 0;
}

size_t attribute_start_offset = 0; // used to hold starting offset of attribute

/**
    @brief Marks the start of a new attribute
    @param attribute_name_index Constant pool index of the name of the attribute
    
*/
void attribute_start(uint16_t attribute_name_index) {
    attributes_counter++;
    // u2 attribute_name_index
    emit_u2(attribute_name_index);
    // local start: record current offset for attribute_length calculation
    attribute_start_offset = current_offset();
    // u4 attribute_length placeholder
    emit_u4(0);
}

/**
    @brief Marks the end of an attribute
    
*/
void attribute_end() {
    size_t end_offset = current_offset();
    uint32_t length = (uint32_t)(end_offset - attribute_start_offset - 4);
    patch_u4(attribute_start_offset, length);
    // restore coordinate values (no-op)
}

/**
    @brief Marks the start of the attributes section
    
*/
void attributes_end() {
    patch_u2(attributes_count_offset, attributes_counter);
    // restore attributes_count, attributes_counter and purge attribute (all no-ops in C)
}

// ------------------------
// fields macros
// ------------------------

static size_t fields_count_offset = 0;
/**
 * @brief Counts the amount of fields in the class
 * 
 */
static uint16_t fields_counter = 0;

/**
    @brief Marks the start of the fields section
    
*/
void fields_start() {
    // u2 fields_count
    fields_count_offset = current_offset();
    emit_u2(0); // placeholder for fields_count
    fields_counter = 0;
}

/**
    @brief Creates a new field and begins it's attributes section
    @param access_flags The JVM flags for the field
    @param name_index Constant pool index of the name of the field
    @param descriptor_index The constant pool index of the decriptor for the field
    
*/
void field_info(uint16_t access_flags, uint16_t name_index, uint16_t descriptor_index) {
    fields_counter++;
    emit_u2(access_flags);
    emit_u2(name_index);
    emit_u2(descriptor_index);
    // attributes for field
    attributes_start();
}

/**
    @brief Ends the current fields attributes section and the field itself
    
*/
void end_field_info() {
    // end_attributes for field
    attributes_end();
}

/**
    @brief Marks the end of the fields section
    
*/
void fields_end() {
    patch_u2(fields_count_offset, fields_counter);
    // purge field_info, end_field_info (no-op)
}

// ------------------------
// methods macros
// ------------------------

static size_t methods_count_offset = 0;
/**
 * @brief Counts the amount of methods in the class
 * 
 */
static uint16_t methods_counter = 0;

/**
    @brief Marks the start of the methods section
    
*/
void methods_start() {
    // u2 methods_count
    methods_count_offset = current_offset();
    emit_u2(0); // placeholder for methods_count
    methods_counter = 0;
}

/**
    @brief Creates a new method and begins it's attributes section
    @param access_flags The JVM flags for the method
    @param name_index Constant pool index of the name of the method
    @param descriptor_index The constant pool index of the decriptor for the method
    
*/
void method_info(uint16_t access_flags, uint16_t name_index, uint16_t descriptor_index) {
    methods_counter++;
    emit_u2(access_flags);
    emit_u2(name_index);
    emit_u2(descriptor_index);
    // attributes for method
    attributes_start();
}

/**
    @brief Ends the current methods attributes section and the method itself
    
*/
void end_method_info() {
    // end_attributes for method
    attributes_end();
}

/**
    @brief Marks the end of the methods section
    
*/
void methods_end() {
    patch_u2(methods_count_offset, methods_counter);
    // purge method_info, end_method_info (no-op)
}

// ------------------------
// bytecode macros
// ------------------------

static size_t bytecode_length_offset = 0;
static size_t bytecode_offset = 0;

/**
    @brief Marks the start of a bytecode section
    
*/
void bytecode_start() {
    // local length; bytecode_length equ length
    bytecode_length_offset = current_offset();
    // u4 bytecode_length placeholder
    emit_u4(0);
    bytecode_offset = current_offset();
    // org 0 is ignored in C
}

/**
    @brief Marks the end of a bytecode section
    
*/
void bytecode_end() {
    size_t end_offset = current_offset();
    uint32_t length = (uint32_t)(end_offset - bytecode_offset);
    patch_u4(bytecode_length_offset, length);
    // org bytecode_offset+bytecode_length, restore bytecode_length are ignored
}

// ------------------------
// exceptions macros
// ------------------------

static size_t exception_table_length_offset = 0;
static uint16_t exception_counter = 0;

/**
    @brief Marks the start of an exceptions section
    
*/
void exceptions_start() {
    // local length; exception_table_length equ length
    exception_table_length_offset = current_offset();
    // u2 exception_table_length placeholder
    emit_u2(0);
    exception_counter = 0;
}

void exception_entry(uint16_t start_pc, uint16_t end_pc, uint16_t handler_pc, uint16_t catch_type) {
    exception_counter++;
    emit_u2(start_pc);
    emit_u2(end_pc);
    emit_u2(handler_pc);
    emit_u2(catch_type);
}

/**
    @brief Marks the end of a bytecode section
    
*/
void exceptions_end() {
    patch_u2(exception_table_length_offset, exception_counter);
    // restore exception_table_length (no-op)
}

// ------------------------
// BYTECODE constants
// ------------------------

/**
 * @brief The magic numbers for primitive types in java
 * 
 */
enum {
    T_BOOLEAN = 4,
    T_CHAR    = 5,
    T_FLOAT   = 6,
    T_DOUBLE  = 7,
    T_BYTE    = 8,
    T_SHORT   = 9,
    T_INT     = 10,
    T_LONG    = 11
};

// ------------------------
// Bytecode instruction macros as functions
// ------------------------

void aaload() {
    // macro aaload { db 0x32 }
    emit_u1(0x32);
}

void aastore() {
    // macro aastore { db 0x53 }
    emit_u1(0x53);
}

void aconst_null() {
    // macro aconst_null { db 0x01 }
    emit_u1(0x01);
}

void aload(uint16_t index) {
    // macro aload index { if index>=0 & index<=3 ... }
    if (index <= 3) {
        emit_u1(0x2a + index);
    } else if (index < 0x100) {
        emit_u1(0x19);
        emit_u1((uint8_t)index);
    } else {
        emit_u1(0xc4);
        emit_u1(0x19);
        emit_u2(index);
    }
}

void anewarray(uint16_t class_index) {
    // macro anewarray class { db 0xbd,(class) shr 8,(class) and 0FFh }
    emit_u1(0xbd);
    emit_u2(class_index);
}

void areturn() {
    // macro areturn { db 0xb0 }
    emit_u1(0xb0);
}

void arraylength() {
    // macro arraylength { db 0xbe }
    emit_u1(0xbe);
}

void astore(uint16_t index) {
    // macro astore index { if index>=0 & index<=3 ... }
    if (index <= 3) {
        emit_u1(0x4b + index);
    } else if (index < 0x100) {
        emit_u1(0x3a);
        emit_u1((uint8_t)index);
    } else {
        emit_u1(0xc4);
        emit_u1(0x3a);
        emit_u2(index);
    }
}

void athrow() {
    // macro athrow { db 0xbf }
    emit_u1(0xbf);
}

void baload() {
    // macro baload { db 0x33 }
    emit_u1(0x33);
}

void bastore() {
    // macro bastore { db 0x54 }
    emit_u1(0x54);
}

void bipush(int8_t byte_val) {
    // macro bipush byte { if byte>-1 & byte<=5 ... }
    if (byte_val >= -1 && byte_val <= 5) {
        emit_u1(0x03 + byte_val);
    } else {
        emit_u1(0x10);
        emit_u1((uint8_t)byte_val);
    }
}

void caload() {
    // macro caload { db 0x34 }
    emit_u1(0x34);
}

void castore() {
    // macro castore { db 0x55 }
    emit_u1(0x55);
}

void checkcast(uint16_t class_index) {
    // macro checkcast class { db 0xc0,(class) shr 8,(class) and 0FFh }
    emit_u1(0xc0);
    emit_u2(class_index);
}

void d2f() {
    // macro d2f { db 0x90 }
    emit_u1(0x90);
}

void d2i() {
    // macro d2i { db 0x8e }
    emit_u1(0x8e);
}

void d2l() {
    // macro d2l { db 0x8f }
    emit_u1(0x8f);
}

void dadd() {
    // macro dadd { db 0x63 }
    emit_u1(0x63);
}

void daload() {
    // macro daload { db 0x31 }
    emit_u1(0x31);
}

void dastore() {
    // macro dastore { db 0x52 }
    emit_u1(0x52);
}

void dcmpg() {
    // macro dcmpg { db 0x98 }
    emit_u1(0x98);
}

void dcmpl() {
    // macro dcmpl { db 0x97 }
    emit_u1(0x97);
}

void dconst_0() {
    // macro dconst_0 { db 0x0e }
    emit_u1(0x0e);
}

void dconst_1() {
    // macro dconst_1 { db 0x0f }
    emit_u1(0x0f);
}

void ddiv() {
    // macro ddiv { db 0x6f }
    emit_u1(0x6f);
}

void dload(uint16_t index) {
    // macro dload index { if index>=0 & index<=3 ... }
    if (index <= 3) {
        emit_u1(0x26 + index);
    } else if (index < 0x100) {
        emit_u1(0x18);
        emit_u1((uint8_t)index);
    } else {
        emit_u1(0xc4);
        emit_u1(0x18);
        emit_u2(index);
    }
}

void dmul() {
    // macro dmul { db 0x6b }
    emit_u1(0x6b);
}

void dneg() {
    // macro dneg { db 0x77 }
    emit_u1(0x77);
}

void j_drem() {
    // macro drem { db 0x73 }
    emit_u1(0x73);
}

void dreturn() {
    // macro dreturn { db 0xaf }
    emit_u1(0xaf);
}

void dstore(uint16_t index) {
    // macro dstore index { if index>=0 & index<=3 ... }
    if (index <= 3) {
        emit_u1(0x47 + index);
    } else if (index < 0x100) {
        emit_u1(0x39);
        emit_u1((uint8_t)index);
    } else {
        emit_u1(0xc4);
        emit_u1(0x39);
        emit_u2(index);
    }
}

void dsub() {
    // macro dsub { db 0x67 }
    emit_u1(0x67);
}

void dup() {
    // macro dup { db 0x59 }
    emit_u1(0x59);
}

void dup_x1() {
    // macro dup_x1 { db 0x5a }
    emit_u1(0x5a);
}

void dup_x2() {
    // macro dup_x2 { db 0x5b }
    emit_u1(0x5b);
}

void dup2() {
    // macro dup2 { db 0x5c }
    emit_u1(0x5c);
}

void dup2_x1() {
    // macro dup2_x1 { db 0x5d }
    emit_u1(0x5d);
}

void dup2_x2() {
    // macro dup2_x2 { db 0x5e }
    emit_u1(0x5e);
}

void f2d() {
    // macro f2d { db 0x8d }
    emit_u1(0x8d);
}

void f2i() {
    // macro f2i { db 0x8b }
    emit_u1(0x8b);
}

void f2l() {
    // macro f2l { db 0x8c }
    emit_u1(0x8c);
}

void fadd() {
    // macro fadd { db 0x62 }
    emit_u1(0x62);
}

void faload() {
    // macro faload { db 0x30 }
    emit_u1(0x30);
}

void fastore() {
    // macro fastore { db 0x51 }
    emit_u1(0x51);
}

void fcmpg() {
    // macro fcmpg { db 0x96 }
    emit_u1(0x96);
}

void fcmpl() {
    // macro fcmpl { db 0x95 }
    emit_u1(0x95);
}

void fconst_0() {
    // macro fconst_0 { db 0x0b }
    emit_u1(0x0b);
}

void fconst_1() {
    // macro fconst_1 { db 0x0c }
    emit_u1(0x0c);
}

void fconst_2() {
    // macro fconst_2 { db 0x0d }
    emit_u1(0x0d);
}

void fdiv() {
    // macro fdiv { db 0x6e }
    emit_u1(0x6e);
}

void fload(uint16_t index) {
    // macro fload index { if index>=0 & index<=3 ... }
    if (index <= 3) {
        emit_u1(0x22 + index);
    } else if (index < 0x100) {
        emit_u1(0x17);
        emit_u1((uint8_t)index);
    } else {
        emit_u1(0xc4);
        emit_u1(0x17);
        emit_u2(index);
    }
}

void fmul() {
    // macro fmul { db 0x6a }
    emit_u1(0x6a);
}

void fneg() {
    // macro fneg { db 0x76 }
    emit_u1(0x76);
}

void frem() {
    // macro frem { db 0x72 }
    emit_u1(0x72);
}

void freturn() {
    // macro freturn { db 0xae }
    emit_u1(0xae);
}

void fstore(uint16_t index) {
    // macro fstore index { if index>=0 & index<=3 ... }
    if (index <= 3) {
        emit_u1(0x43 + index);
    } else if (index < 0x100) {
        emit_u1(0x38);
        emit_u1((uint8_t)index);
    } else {
        emit_u1(0xc4);
        emit_u1(0x38);
        emit_u2(index);
    }
}

void fsub() {
    // macro fsub { db 0x66 }
    emit_u1(0x66);
}

void getfield(uint16_t index) {
    // macro getfield index { db 0xb4,(index) shr 8,(index) and 0FFh }
    emit_u1(0xb4);
    emit_u2(index);
}

void getstatic(uint16_t index) {
    // macro getstatic index { db 0xb2,(index) shr 8,(index) and 0FFh }
    emit_u1(0xb2);
    emit_u2(index);
}

void goto_inst(size_t branch_target) {
    // macro goto branch { if branch-$>=-8000h & branch-$<8000h ... }
    int32_t offset = (int32_t)(branch_target - current_offset());
    if (offset >= (int32_t)0xFFFF8000 && offset < 0x8000) {
        int16_t word_offset = (int16_t)offset;
        emit_u1(0xa7);
        emit_u2((uint16_t)word_offset);
    } else {
        int32_t dword_offset = offset;
        emit_u1(0xc8);
        emit_u4((uint32_t)dword_offset);
    }
}

void goto_w_inst(size_t branch_target) {
    // macro goto_w branch { offset = dword branch-$; db 0xc8, ... }
    int32_t offset = (int32_t)(branch_target - current_offset());
    emit_u1(0xc8);
    emit_u4((uint32_t)offset);
}

void i2b() {
    // macro i2b { db 0x91 }
    emit_u1(0x91);
}

void i2c() {
    // macro i2c { db 0x92 }
    emit_u1(0x92);
}

void i2d() {
    // macro i2d { db 0x87 }
    emit_u1(0x87);
}

void i2f() {
    // macro i2f { db 0x86 }
    emit_u1(0x86);
}

void i2l() {
    // macro i2l { db 0x85 }
    emit_u1(0x85);
}

void i2s() {
    // macro i2s { db 0x93 }
    emit_u1(0x93);
}

void iadd() {
    // macro iadd { db 0x60 }
    emit_u1(0x60);
}

void iaload() {
    // macro iaload { db 0x2e }
    emit_u1(0x2e);
}

void iand() {
    // macro iand { db 0x7e }
    emit_u1(0x7e);
}

void iastore() {
    // macro iastore { db 0x4f }
    emit_u1(0x4f);
}

void iconst_m1() {
    // macro iconst_m1 { db 0x02 }
    emit_u1(0x02);
}

void iconst_0() {
    // macro iconst_0 { db 0x03 }
    emit_u1(0x03);
}

void iconst_1() {
    // macro iconst_1 { db 0x04 }
    emit_u1(0x04);
}

void iconst_2() {
    // macro iconst_2 { db 0x05 }
    emit_u1(0x05);
}

void iconst_3() {
    // macro iconst_3 { db 0x06 }
    emit_u1(0x06);
}

void iconst_4() {
    // macro iconst_4 { db 0x07 }
    emit_u1(0x07);
}

void iconst_5() {
    // macro iconst_5 { db 0x08 }
    emit_u1(0x08);
}

void idiv() {
    // macro idiv { db 0x6c }
    emit_u1(0x6c);
}

void if_acmpeq(size_t branch_target) {
    // macro if_acmpeq branch { offset = word branch-$; db 0xa5, ... }
    int16_t offset = (int16_t)(branch_target - current_offset());
    emit_u1(0xa5);
    emit_u2((uint16_t)offset);
}

void if_acmpne(size_t branch_target) {
    int16_t offset = (int16_t)(branch_target - current_offset());
    emit_u1(0xa6);
    emit_u2((uint16_t)offset);
}

void if_icmpeq(size_t branch_target) {
    int16_t offset = (int16_t)(branch_target - current_offset());
    emit_u1(0x9f);
    emit_u2((uint16_t)offset);
}

void if_icmpne(size_t branch_target) {
    int16_t offset = (int16_t)(branch_target - current_offset());
    emit_u1(0xa0);
    emit_u2((uint16_t)offset);
}

void if_icmplt(size_t branch_target) {
    int16_t offset = (int16_t)(branch_target - current_offset());
    emit_u1(0xa1);
    emit_u2((uint16_t)offset);
}

void if_icmpge(size_t branch_target) {
    int16_t offset = (int16_t)(branch_target - current_offset());
    emit_u1(0xa2);
    emit_u2((uint16_t)offset);
}

void if_icmpgt(size_t branch_target) {
    int16_t offset = (int16_t)(branch_target - current_offset());
    emit_u1(0xa3);
    emit_u2((uint16_t)offset);
}

void if_icmple(size_t branch_target) {
    int16_t offset = (int16_t)(branch_target - current_offset());
    emit_u1(0xa4);
    emit_u2((uint16_t)offset);
}

void ifeq(size_t branch_target) {
    int16_t offset = (int16_t)(branch_target - current_offset());
    emit_u1(0x99);
    emit_u2((uint16_t)offset);
}

void ifne(size_t branch_target) {
    int16_t offset = (int16_t)(branch_target - current_offset());
    emit_u1(0x9a);
    emit_u2((uint16_t)offset);
}

void iflt(size_t branch_target) {
    int16_t offset = (int16_t)(branch_target - current_offset());
    emit_u1(0x9b);
    emit_u2((uint16_t)offset);
}

void ifge(size_t branch_target) {
    int16_t offset = (int16_t)(branch_target - current_offset());
    emit_u1(0x9c);
    emit_u2((uint16_t)offset);
}

void ifgt(size_t branch_target) {
    int16_t offset = (int16_t)(branch_target - current_offset());
    emit_u1(0x9d);
    emit_u2((uint16_t)offset);
}

void ifle(size_t branch_target) {
    int16_t offset = (int16_t)(branch_target - current_offset());
    emit_u1(0x9e);
    emit_u2((uint16_t)offset);
}

void ifnonnull(size_t branch_target) {
    int16_t offset = (int16_t)(branch_target - current_offset());
    emit_u1(0xc7);
    emit_u2((uint16_t)offset);
}

void ifnull(size_t branch_target) {
    int16_t offset = (int16_t)(branch_target - current_offset());
    emit_u1(0xc6);
    emit_u2((uint16_t)offset);
}

void iinc(uint16_t index, int16_t constant_val) {
    // macro iinc index, const { if index < 100h & const < 80h & const >= -80h ... }
    if (index < 0x100 && constant_val < 0x80 && constant_val >= -0x80) {
        emit_u1(0x84);
        emit_u1((uint8_t)index);
        emit_u1((uint8_t)constant_val);
    } else {
        emit_u1(0xc4);
        emit_u1(0x84);
        emit_u2(index);
        emit_u2((uint16_t)constant_val);
    }
}

void iload(uint16_t index) {
    // macro iload index { if index>=0 & index<=3 ... }
    if (index <= 3) {
        emit_u1(0x1a + index);
    } else if (index < 0x100) {
        emit_u1(0x15);
        emit_u1((uint8_t)index);
    } else {
        emit_u1(0xc4);
        emit_u1(0x15);
        emit_u2(index);
    }
}

void imul() {
    // macro imul { db 0x68 }
    emit_u1(0x68);
}

void ineg() {
    // macro ineg { db 0x74 }
    emit_u1(0x74);
}

void instanceof(uint16_t index) {
    // macro instanceof index { db 0xc1,(index) shr 8,(index) and 0FFh }
    emit_u1(0xc1);
    emit_u2(index);
}

void invokedynamic(uint16_t index) {
    // macro invokedynamic index { db 0xba,(index) shr 8,(index) and 0FFh,0,0 }
    emit_u1(0xba);
    emit_u2(index);
    emit_u1(0x00);
    emit_u1(0x00);
}

void invokeinterface(uint16_t index, uint8_t count) {
    // macro invokeinterface index,count { db 0xb9,(index) shr 8,(index) and 0FFh,count }
    emit_u1(0xb9);
    emit_u2(index);
    emit_u1(count);
}

void invokespecial(uint16_t index) {
    // macro invokespecial index { db 0xb7,(index) shr 8,(index) and 0FFh }
    emit_u1(0xb7);
    emit_u2(index);
}

void invokestatic(uint16_t index) {
    // macro invokestatic index { db 0xb8,(index) shr 8,(index) and 0FFh }
    emit_u1(0xb8);
    emit_u2(index);
}

void invokevirtual(uint16_t index) {
    // macro invokevirtual index { db 0xb6,(index) shr 8,(index) and 0FFh }
    emit_u1(0xb6);
    emit_u2(index);
}

void ior() {
    // macro ior { db 0x80 }
    emit_u1(0x80);
}

void irem() {
    // macro irem { db 0x70 }
    emit_u1(0x70);
}

void ireturn() {
    // macro ireturn { db 0xac }
    emit_u1(0xac);
}

void ishl() {
    // macro ishl { db 0x78 }
    emit_u1(0x78);
}

void ishr() {
    // macro ishr { db 0x7a }
    emit_u1(0x7a);
}

void istore(uint16_t index) {
    // macro istore index { if index>=0 & index<=3 ... }
    if (index <= 3) {
        emit_u1(0x3b + index);
    } else if (index < 0x100) {
        emit_u1(0x36);
        emit_u1((uint8_t)index);
    } else {
        emit_u1(0xc4);
        emit_u1(0x36);
        emit_u2(index);
    }
}

void isub() {
    // macro isub { db 0x64 }
    emit_u1(0x64);
}

void iushr() {
    // macro iushr { db 0x7c }
    emit_u1(0x7c);
}

void ixor() {
    // macro ixor { db 0x82 }
    emit_u1(0x82);
}

void jsr_inst(size_t branch_target) {
    // macro jsr branch { if branch-$>=-8000h & branch-$<8000h ... }
    int32_t offset = (int32_t)(branch_target - current_offset());
    if (offset >= (int32_t)0xFFFF8000 && offset < 0x8000) {
        int16_t word_offset = (int16_t)offset;
        emit_u1(0xa8);
        emit_u2((uint16_t)word_offset);
    } else {
        int32_t dword_offset = offset;
        emit_u1(0xc9);
        emit_u4((uint32_t)dword_offset);
    }
}

void jsr_w_inst(size_t branch_target) {
    // macro jsr_w branch { offset = dword branch-$; db 0xc9, ... }
    int32_t offset = (int32_t)(branch_target - current_offset());
    emit_u1(0xc9);
    emit_u4((uint32_t)offset);
}

void l2d() {
    // macro l2d { db 0x8a }
    emit_u1(0x8a);
}

void l2f() {
    // macro l2f { db 0x89 }
    emit_u1(0x89);
}

void l2i() {
    // macro l2i { db 0x88 }
    emit_u1(0x88);
}

void ladd() {
    // macro ladd { db 0x61 }
    emit_u1(0x61);
}

void laload() {
    // macro laload { db 0x2f }
    emit_u1(0x2f);
}

void land() {
    // macro land { db 0x7f }
    emit_u1(0x7f);
}

void lastore() {
    // macro lastore { db 0x50 }
    emit_u1(0x50);
}

void lcmp() {
    // macro lcmp { db 0x94 }
    emit_u1(0x94);
}

void lconst_0() {
    // macro lconst_0 { db 0x09 }
    emit_u1(0x09);
}

void lconst_1() {
    // macro lconst_1 { db 0x0a }
    emit_u1(0x0a);
}

void ldc(uint16_t index) {
    // macro ldc index { if index<100h ... }
    if (index < 0x100) {
        emit_u1(0x12);
        emit_u1((uint8_t)index);
    } else {
        emit_u1(0x13);
        emit_u2(index);
    }
}

void ldc_w(uint16_t index) {
    // macro ldc_w index { db 0x13,(index) shr 8,(index) and 0FFh }
    emit_u1(0x13);
    emit_u2(index);
}

void ldc2_w(uint16_t index) {
    // macro ldc2_w index { db 0x14,(index) shr 8,(index) and 0FFh }
    emit_u1(0x14);
    emit_u2(index);
}

void j_ldiv() {
    // macro ldiv { db 0x6d }
    emit_u1(0x6d);
}

void lload(uint16_t index) {
    // macro lload index { if index>=0 & index<=3 ... }
    if (index <= 3) {
        emit_u1(0x1e + index);
    } else if (index < 0x100) {
        emit_u1(0x16);
        emit_u1((uint8_t)index);
    } else {
        emit_u1(0xc4);
        emit_u1(0x16);
        emit_u2(index);
    }
}

void lmul() {
    // macro lmul { db 0x69 }
    emit_u1(0x69);
}

void lneg() {
    // macro lneg { db 0x75 }
    emit_u1(0x75);
}

// macro lookupswitch is commented out

void lor() {
    // macro lor { db 0x81 }
    emit_u1(0x81);
}

void lrem() {
    // macro lrem { db 0x71 }
    emit_u1(0x71);
}

void lreturn() {
    // macro lreturn { db 0xad }
    emit_u1(0xad);
}

void lshl() {
    // macro lshl { db 0x79 }
    emit_u1(0x79);
}

void lshr() {
    // macro lshr { db 0x7b }
    emit_u1(0x7b);
}

void lstore(uint16_t index) {
    // macro lstore index { if index>=0 & index<=3 ... }
    if (index <= 3) {
        emit_u1(0x3f + index);
    } else if (index < 0x100) {
        emit_u1(0x37);
        emit_u1((uint8_t)index);
    } else {
        emit_u1(0xc4);
        emit_u1(0x37);
        emit_u2(index);
    }
}

void lsub() {
    // macro lsub { db 0x65 }
    emit_u1(0x65);
}

void lushr() {
    // macro lushr { db 0x7d }
    emit_u1(0x7d);
}

void lxor() {
    // macro lxor { db 0x83 }
    emit_u1(0x83);
}

void monitorenter() {
    // macro monitorenter { db 0xc2 }
    emit_u1(0xc2);
}

void monitorexit() {
    // macro monitorexit { db 0xc3 }
    emit_u1(0xc3);
}

void multianewarray(uint16_t index, uint8_t dimensions) {
    // macro multianewarray index,dimensions { db 0xc5,(index) shr 8,(index) and 0FFh,dimensions }
    emit_u1(0xc5);
    emit_u2(index);
    emit_u1(dimensions);
}

void new_inst(uint16_t index) {
    // macro new index { db 0xbb,(index) shr 8,(index) and 0FFh }
    emit_u1(0xbb);
    emit_u2(index);
}

void newarray(uint8_t atype) {
    // macro newarray atype { db 0xbc,atype }
    emit_u1(0xbc);
    emit_u1(atype);
}

void nop() {
    // macro nop { db 0x00 }
    emit_u1(0x00);
}

void pop_inst() {
    // macro pop { db 0x57 }
    emit_u1(0x57);
}

void pop2() {
    // macro pop2 { db 0x58 }
    emit_u1(0x58);
}

void putfield(uint16_t index) {
    // macro putfield index { db 0xb5,(index) shr 8,(index) and 0FFh }
    emit_u1(0xb5);
    emit_u2(index);
}

void putstatic(uint16_t index) {
    // macro putstatic index { db 0xb3,(index) shr 8,(index) and 0FFh }
    emit_u1(0xb3);
    emit_u2(index);
}

void ret_inst(uint16_t index) {
    // macro ret index { if index<100h ... }
    if (index < 0x100) {
        emit_u1(0xa9);
        emit_u1((uint8_t)index);
    } else {
        emit_u1(0xc4);
        emit_u1(0xa9);
        emit_u2(index);
    }
}

void return_inst() {
    // macro return { db 0xb1 }
    emit_u1(0xb1);
}

void saload() {
    // macro saload { db 0x35 }
    emit_u1(0x35);
}

void sastore() {
    // macro sastore { db 0x56 }
    emit_u1(0x56);
}

void sipush(uint16_t value) {
    // macro sipush short { db 0x11,(short) shr 8,(short) and 0FFh }
    emit_u1(0x11);
    emit_u2(value);
}

void swap() {
    // macro swap { db 0x5f }
    emit_u1(0x5f);
}

// macro tableswitch is commented out

void breakpoint() {
    // macro breakpoint { db 0xca }
    emit_u1(0xca);
}

void impdep1() {
    // macro impdep1 { db 0xfe }
    emit_u1(0xfe);
}

void impdep2() {
    // macro impdep2 { db 0xff }
    emit_u1(0xff);
}
void impdep2_dup() {
    // duplicate definition as in original code
    emit_u1(0xff);
}

static void emit_class_header() {
    // Magic number
    emit_u4(0xCAFEBABE);
    
    // Version: Java 8
    emit_u2(0);     // minor_version
    emit_u2(52);    // major_version
}

static void emit_class_footer(uint16_t this_class, uint8_t this_class_flags, uint16_t super_class) {
    emit_u2(this_class_flags);
    
    // Class references
    emit_u2(this_class);     // this_class
    emit_u2(super_class);    // super_class
}

// Enhanced Code attribute handling
void code_attribute_start(uint16_t name_index, uint16_t max_stack, uint16_t max_locals) {
    attribute_start(name_index);
    emit_u2(max_stack);
    emit_u2(max_locals);
    bytecode_start();  // Starts code emission
}

void code_attribute_end() {
    bytecode_end();     // Patches code length
    // Add exception table (empty) and attributes (none)
    emit_u2(0);        // exception_table_length
    emit_u2(0);        // attributes_count
    attribute_end();
}
