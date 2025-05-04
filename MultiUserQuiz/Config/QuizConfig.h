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

private:
                                // Ctor and Dtors
                                QuizConfig              ();
                                ~QuizConfig             ();

                                QuizConfig              (const QuizConfig &) = delete;
                                QuizConfig & operator=  (const QuizConfig &) = delete;

    static  int                 Parser                  (void * user, const char * section, const char * name, const char * value);

            // config variables.
            eQuizMode           quizMode;
            bool                isSingleUser;
            bool                isMultiOptionSelect;

            // scoring related
            bool                isNegMarking;
            double              negMarking;
            double              posMarking;
            double              paritalMarking;

            // time related
            union {
                unsigned int    timePerQues;            // allowed time to attempt each question - bullet mode
                unsigned int    totalTime;              // total time to finish the test - competitive/time bound mode.
            } timeAllowed;

            // result related
            bool                isKBCMode;              // KBC mode.
            bool                displayOnlyScore;       // only score should be displayed.

};