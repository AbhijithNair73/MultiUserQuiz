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
                                User                        (const std::string & pUserName);
                                ~User                       ();

        void                    SetStartTime                (long long pTotaltime);
        long long               GetStartTime                () const;

        void                    SetTotalTimeLimit           (long long pTotaltime);
        long long               GetTotalTimeLimit           () const;
        void                    UpdateElapsedTimeInQuiz     (long long pTimeElapsed);
        void                    AddToElapsedTimeInQuiz      (long long pTimeElapsed);
        long long               GetElapsedTime              ();

        eQuesAttemptStatus      SetAndValidateUserAnswer    (Answer & pAns);
        double                  GetUserCurrentScore         ();
        void                    ShowFinalScore              (bool pShowIncorrectAttempts);

        std::vector<unsigned int> GetUnattemptedQuestionIds () const;

    private:

        long long               vStartTime;                 // stores the julian time when the quiz is started. will be resetted 
                                                            // when the quiz is restarted. This parameter is not needed for bullet mode as the time is managed on client
                                                            // and is needed in time-bound mode for calculation of time remaining in case of reconnection or relogin.

        std::unique_ptr<Result> vResultPtr;                 // Result object for this user
        std::string             vUserName;                  // User name
};

