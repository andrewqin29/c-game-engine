#ifndef QUIZ_BANK_H
#define QUIZ_BANK_H

#include <stddef.h>   // for size_t

/**
 * One multiple-choice quiz question.
 *
 * `question_text`        - UTF-8 question string.
 * `num_options`          - number of answer choices in `answer_choices`.
 * `answer_choices`       - array of C-strings (NOT owned by the struct).
 * `correct_option_index` - 0-based index into `answer_choices`.
 * `font_path`            -  TTF font to render this question/answers with.
 */
typedef struct {
    char  *question_text;
    size_t       num_options;
    char **answer_choices;
    int          correct_option_index;   // 0 num_options-1
    char  *font_path;
} quiz_question_t;


extern const quiz_question_t QUIZ_BANK[];   /* full array of questions  */
extern const size_t          QUIZ_BANK_LEN; /* number of questions (20) */

#endif /* QUIZ_BANK_H */