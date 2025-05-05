#pragma once
#include <iostream>

#include "../QuizDefs.h"

using namespace std;

/*
* QuizConfig class is a singleton class which stores the configurations
* configs like negative scoring, mode of the test, etc.
* This class will be accessible to all - i.e server and client both - why? need to think through really needed or not?
*/
class QuizConfig {

public:

    static QuizConfig &         GetInstance             ();

            bool                LoadConfigFromFile      ();
            bool                RefreshConfig           ();

            double              GetCorrectAnsReward     () const;
            double              GetIncorrectAnsPenalty  () const;
            double              GetPartialAnsReward     () const;

            unsigned int        GetTimeAllowedBasedOnQuizMode () const;

            bool                IsSingleUser () const;
            bool                IsMultiOptionSelect () const;
            eQuizMode           GetQuizMode () const;

private:
                                // Ctor and Dtors
                                QuizConfig              ();
                                ~QuizConfig             ();

                                QuizConfig              (const QuizConfig &) = delete;
                                QuizConfig & operator=  (const QuizConfig &) = delete;

    static  int                 Parser                  (void * user, const char * section, const char * name, const char * value);

            // config variables.
            eQuizMode           vQuizMode;
            bool                vIsSingleUser;
            bool                vIsMultiOptionSelect;

            // scoring related
            bool                vIsNegMarking;
            double              negMarking;
            double              posMarking;
            double              paritalMarking;

            // time related
            union {
                unsigned int    vTimePerQues;            // allowed time to attempt each question - bullet mode
                unsigned int    vTotalQuizTime;          // total time to finish the test - competitive/time bound mode.
            } vTimeAllowed;

            // result related
            bool                vIsKBCMode;             // In KBC mode it will show the correct answer and score after each question.
                                                        // KBC Mode should not be allowed with TIMERBOUND quiz
};