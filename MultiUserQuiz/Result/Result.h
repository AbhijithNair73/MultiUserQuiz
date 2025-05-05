#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include "../Answer/Answer.h"
#include "../QuizDefs.h"

using namespace std;

/*
* Result class serves as storage of the result and also keeps score
* and the attempted answers gets recorded here.
* 
* Every User will have its own copy of result object.
* 
* 
**/
class Result {
 
public:
                                Result                  ();
                                ~Result                 ();

    eQuesAttemptStatus          AddAnswer               (Answer & ans);
    double                      GetCurrentScore         () const;

    void                        PrintFinalResult        (bool pShowDetailedResult) const;

    void                        SetTimeLimit            (unsigned int timeLimit);
    void                        UpdateTimeElapsed       (unsigned int pTimeElapsedMs);
    unsigned int                GetTimeElapsedInQuiz    () const;

private:

    // Store attempted answer and map question id to selected answer
    unordered_map <unsigned int, Answer> attempt_map;
    // Store question id and its status
    unordered_map <unsigned int, eQuesAttemptStatus> tracker_map;

    // score
    double vCurrScore = 0;

    const double correctReward      = 1.0;          // reward for correct answer
    const double incorrectPenalty   = 0.25;         // penalty for incorrect answer
    const double partialReward      = 0.5;          // reward for partially correct answer

    // mode
    unsigned int vTotalTimeElapsed;  // Used in Mode 2
    unsigned int vTotalTimeLimit;    // Used in Mode 2

};