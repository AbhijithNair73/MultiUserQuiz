#include "User.h"

User::User ()
{
    vResultPtr = std::make_unique<Result> ();
}

User::~User ()
{
    // nothing to do
}

void User::SetTotalTimeLimit (unsigned int pTotaltime)
{
    vResultPtr->SetTimeLimit (pTotaltime);
}

void User::UpdateElapsedTimeInQuiz (unsigned int pTimeElapsed)
{
    vResultPtr->UpdateTimeElapsed (pTimeElapsed);
}

eQuesAttemptStatus User::SetAndValidateUserAnswer (Answer & pAns)
{
    return vResultPtr->AddAnswer (pAns);
}

double User::GetUserCurrentScore ()
{
    return vResultPtr->GetCurrentScore ();
}

void User::ShowFinalScore (bool pShowIncorrectAttempts)
{
    vResultPtr->PrintFinalResult (pShowIncorrectAttempts);
}