#pragma once
#include <iostream>
#include "../Result/Result.h"
/*
* Manages a user
* Should be responsible for login and also initiating connection to server
* TODO: ConnectionMgr class will come.
* This will call a function of QuizMgr - StartQuizClient - which will accordingly fetch the questions from the server
* and then display the questions to the user.
* and take input from user
* It stores the user name and the result object for this user
*/

class User {
    public:

                                User                        ();
                                ~User                       ();

        void                    SetTotalTimeLimit           (unsigned int pTotaltime);
        void                    UpdateElapsedTimeInQuiz     (unsigned int pTimeElapsed);
        eQuesAttemptStatus      SetAndValidateUserAnswer    (Answer & pAns);
        double                  GetUserCurrentScore         ();
        void                    ShowFinalScore              (bool pShowIncorrectAttempts);
    private:

        std::unique_ptr<Result> vResultPtr;                     // Result object for this user
};