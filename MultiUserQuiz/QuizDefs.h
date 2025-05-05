#pragma once
#include "Question/QuestionBank.h"
#include "Answer/Answer.h"
#include <sstream>

#define MAX_NUMBER_OF_CORRECT_OPTION        3

enum eQuizMode {
    BULLET_TIMER_MODE,       // User has limited time per question
    TIME_BOUND_MODE,         // User has limited total time to finish the quiz
    MODE_MAX,
};

enum eQuesAttemptStatus {

    CORRECT,
    INCORRECT,
    PARTIALLY_CORRECT,
    UNATTEMPTED,
};

namespace QuizHelper {

    eQuesAttemptStatus ValidateUserAnswer (const Answer & ans);

    std::shared_ptr<Question> MakeQuestionFromExcelRow (const std::string & question_number_str,
                                                        const std::string & question_text, 
                                                        const std::string & option_a, 
                                                        const std::string & option_b, 
                                                        const std::string & option_c, 
                                                        const std::string & option_d, 
                                                        const std::string & correct_options_str);
}