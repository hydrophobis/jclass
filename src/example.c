#include "jclass.c"

int main() {
    J_CLASS_BEGIN

    // Start emitting class header
    emit_class_header();

    // Start constant pool
    constant_pool_start();
    constant_utf8("TestClass");                // Index 1
    constant_utf8("java/lang/Object");         // Index 2
    constant_utf8("Code");                     // Index 3
    constant_utf8("()V");                      // Index 4
    constant_utf8("([Ljava/lang/String;)V");   // Index 5
    constant_utf8("<init>");                   // Index 6
    constant_utf8("main");                     // Index 7
    constant_class(1);                         // Index 8: TestClass
    constant_class(2);                         // Index 9: java/lang/Object
    constant_nameandtype(6, 4);                // Index 10: <init> ()V
    constant_nameandtype(7, 5);                // Index 11: main ([Ljava/lang/String;)V
    constant_methodref(9, 10);                 // Index 12: java/lang/Object.<init> ()V
    constant_utf8("Hello World");              // Index 13
    constant_utf8("java/lang/System");         // Index 14
    constant_class(14);                        // Index 15: java/lang/System
    constant_utf8("out");                      // Index 16
    constant_utf8("Ljava/io/PrintStream;");    // Index 17

    constant_nameandtype(16, 17);              // Index 18: System.out
    constant_fieldref(15, 18);                 // Index 19: java/lang/System.out
    constant_utf8("(Ljava/lang/String;)V");    // Index 20: println descriptor
    constant_utf8("println");                  // Index 21
    constant_nameandtype(21, 20);              // Index 22: println name and type

    constant_string(13);                       // Index 23: Hello World string
    constant_utf8("java/io/PrintStream");      // Index 24: PrintStream
    constant_class(24);                        // Index 25: java/io/Printstream
    constant_methodref(25, 22);                // Index 26: java/io/PrintStream.println
    constant_pool_end();

    emit_class_footer(8, ACC_PUBLIC, 9); // TestClass extends Object

    interfaces_start();
    interfaces_end();

    fields_start();
    fields_end();

    methods_start();

    // Constructor method
    method_info(ACC_PUBLIC, 6, 4); // <init> ()V
    code_attribute_start(3, 1, 1); // max_stack = 1, max_locals = 1
    aload(0); // Load "this"
    invokespecial(12); // Call java/lang/Object.<init>
    return_inst(); // Return
    code_attribute_end();
    end_method_info();

    // Main method
    method_info(ACC_PUBLIC | ACC_STATIC, 7, 5); // main ([Ljava/lang/String;)V
    code_attribute_start(3, 2, 1); // max_stack = 2, max_locals = 1
    getstatic(19); // Get java/lang/System.out
    ldc(23); // Load "Hello World"
    invokevirtual(26); // Call println
    return_inst(); // Return
    code_attribute_end();
    end_method_info();

    methods_end();

    attributes_start();
    attributes_end();

    J_CLASS_END

    // Output the class bytecode to a file
    FILE *file = fopen("TestClass.class", "wb");
    if (file) {
        fwrite(outputBuffer, 1, outputIndex, file);
        fclose(file);
    } else {
        fprintf(stderr, "Failed to open output file\n");
        return 1;
    }

    return 0;
}