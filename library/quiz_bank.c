#include "quiz_bank.h"
#include <stddef.h>


static const char *Q1_OPTS[]  = { "Read user input",
                                  "Print formatted output to stdout",
                                  "Allocate memory" };

static const char *Q2_OPTS[]  = { "var", "let", "const", "final" };

static const char *Q3_OPTS[]  = { "An empty string",
                                  "The number zero",
                                  "A pointer that doesn't point to a valid memory location" };

static const char *Q4_OPTS[]  = { "*  (asterisk)",
                                  "&  (ampersand)",
                                  "-> (arrow)",
                                  ".  (dot)" };

static const char *Q5_OPTS[]  = { "<stdio.h>", "<stdlib.h>",
                                  "<string.h>", "<malloc.h>" };

static const char *Q6_OPTS[]  = { "The array length",
                                  "The number of bytes of its operand",
                                  "Always 4",
                                  "Size of the pointer only" };

static const char *Q7_OPTS[]  = { "for loop", "while loop",
                                  "do-while loop", "Infinite loop" };

static const char *Q8_OPTS[]  = { "\\r", "\\n", "\\t", "\\0" };

static const char *Q9_OPTS[]  = { "strcmp()", "strcpy()",
                                  "strcat()", "strlen()" };


static const char *Q11_OPTS[] = { "continue", "break",
                                  "exit", "return" };

static const char *Q12_OPTS[] = { "9", "10", "8", "11" };

static const char *Q13_OPTS[] = { "double pointer",
                                  "function pointer",
                                  "void pointer (void*)",
                                  "null pointer" };

static const char *Q14_OPTS[] = { "Segmentation fault",
                                  "Buffer overflow",
                                  "Dangling pointer",
                                  "Memory leak" };

static const char *Q15_OPTS[] = { "gcc -o prog prog.c",
                                  "gcc prog.c -o prog",
                                  "Both A and B",
                                  "clang++ prog.c" };

static const char *Q16_OPTS[] = { "#define MAX 100;",
                                  "int MAX = 100;",
                                  "const int MAX = 100;",
                                  "static const MAX = 100;" };


static const char *Q18_OPTS[] = { "0x0", "0", "NULL", "All of the above" };

static const char *Q19_OPTS[] = { "fopen()", "fclose()",
                                  "fread()", "fwrite()" };

static const char *Q20_OPTS[] = { "== compares content, = assigns",
                                  "= compares, == assigns",
                                  "Both compare", "Both assign" };


const quiz_question_t QUIZ_BANK[] = {
    { "What does 'printf' primarily do in C?",
      3, (char **)Q1_OPTS, 1, "assets/fonts/Cascadia.ttf" },

    { "Which keyword is used to define a constant in C?",
      4, (char **)Q2_OPTS, 2, "assets/fonts/Cascadia.ttf" },

    { "What is 'NULL' typically used to represent?",
      3, (char **)Q3_OPTS, 2, "assets/fonts/Cascadia.ttf" },

    { "Which operator is used to access the value pointed to by a pointer?",
      4, (char **)Q4_OPTS, 0, "assets/fonts/Cascadia.ttf" },

    { "Which header file must be included to use malloc()?",
      4, (char **)Q5_OPTS, 1, "assets/fonts/Cascadia.ttf" },

    { "What does the sizeof operator return?",
      4, (char **)Q6_OPTS, 1, "assets/fonts/Cascadia.ttf" },

    { "Which loop in C is guaranteed to execute at least once?",
      4, (char **)Q7_OPTS, 2, "assets/fonts/Cascadia.ttf" },

    { "Which escape sequence represents a newline?",
      4, (char **)Q8_OPTS, 1, "assets/fonts/Cascadia.ttf" },

    { "Which standard function compares two strings?",
      4, (char **)Q9_OPTS, 0, "assets/fonts/Cascadia.ttf" },

    { "Which keyword immediately exits a switch statement?",
      4, (char **)Q11_OPTS, 1, "assets/fonts/Cascadia.ttf" },

    { "For int arr[10]; the largest valid index is…",
      4, (char **)Q12_OPTS, 0, "assets/fonts/Cascadia.ttf" },

    { "Which pointer type can hold the address of any data type?",
      4, (char **)Q13_OPTS, 2, "assets/fonts/Cascadia.ttf" },

    { "Forgetting to free dynamically allocated memory causes a…",
      4, (char **)Q14_OPTS, 3, "assets/fonts/Cascadia.ttf" },

    { "Which command correctly compiles prog.c with GCC?",
      4, (char **)Q15_OPTS, 2, "assets/fonts/Cascadia.ttf" },

    { "Which line correctly defines an integer constant named MAX?",
      4, (char **)Q16_OPTS, 2, "assets/fonts/Cascadia.ttf" },


    { "Which of the following can represent a null pointer literal in C?",
      4, (char **)Q18_OPTS, 3, "assets/fonts/Cascadia.ttf" },

    { "Which function closes a file opened with fopen()?",
      4, (char **)Q19_OPTS, 1, "assets/fonts/Cascadia.ttf" },

    { "In C, what is the difference between '=' and '=='?",
      4, (char **)Q20_OPTS, 0, "assets/fonts/Cascadia.ttf" }
};


const size_t QUIZ_BANK_LEN = sizeof(QUIZ_BANK) / sizeof(QUIZ_BANK[0]);
