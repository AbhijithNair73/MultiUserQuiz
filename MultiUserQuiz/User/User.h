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

        void                    SetStartTimeInMs                (long long pTotaltime);
        long long               GetStartTimeInMs                () const;

        void                    SetEndTimeInMs                  (long long pTotaltime);
        long long               GetEndTimeInMs                  () const;

        void                    SetLastActivityTimeInMs         ();
        void                    ResetLastActivityTimeInMs       ();
        long long               GetlastActivityTimeInMs         () const;

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

        long long               vStartTime;                 // stores the julian time when the quiz is started, used in strict mode to know when the quiz started

        long long               vEndTime;                   // stores the julian time when the quiz will end - used in strict time bound mode.

        long long               vLastActivityTime;          // This stores when the last request came from client to fetch question or submit answer. This will help in calculating 
                                                            // the elapsed time in case of disconnection happens after long duration of inactivity at client.

        std::unique_ptr<Result> vResultPtr;                 // Result object for this user
        std::string             vUserName;                  // User name
};

