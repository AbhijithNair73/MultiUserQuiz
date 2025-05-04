#pragma once
#include <iostream>
#include <string>
#include "External/xlnt/xlnt.hpp"
#include "Question/QuestionBank.h"
#include "Question/Question.h"
#include "QuizDefs.h"

/*
* Currently the usage and sope of this class is not yet clear.
* This class will be primarily used to manage the quiz - i.e start the quiz, stop the quiz, get the results, etc.
* Mananges the entire quiz, including users.
* Design Doubts:
* 1. Currently not sure if this class will be used to display/push the questions to each client.
* 2. TODO: There should be a connection manager class for managing multiple client connections and then there there should be 2 exe.
*
*/
class QuizMgr {

    public:
                QuizMgr                 ();
                ~QuizMgr                ();

    bool        StartQuiz               (const std::string & pFileName);

    private:

    bool        InitializeQuestionBank  (const std::string & excelFileName);
};