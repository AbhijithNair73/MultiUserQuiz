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

    void                        SetTotalTimeLimit       (long long timeLimit);
    long long                   GetTotalTimeLimit       () const;
    void                        UpdateTimeElapsed       (long long pTimeElapsedMs);
    void                        AddToElapsedTime        (long long pTimeElapsedMs);
    long long                   GetTimeElapsedInQuiz    () const;

    std::vector<unsigned int>   GetUnattemptedQuestionIds () const;

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
    long long vTotalTimeElapsed;  // Used in Mode 2
    long long vTotalTimeLimit;    // Used in Mode 2

};