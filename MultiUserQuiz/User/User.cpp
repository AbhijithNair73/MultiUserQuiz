#include "User.h"

User::User (const std::string & pUserName)
{
    vResultPtr = std::make_unique<Result> ();
    vUserName = pUserName;
}

User::~User ()
{
    // nothing to do
}

void User::SetStartTime (long long pStartTime)
{
    vStartTime = pStartTime;
}

long long User::GetStartTime () const
{ 
    return vStartTime;
}

void User::SetTotalTimeLimit (long long pTotaltime)
{
    vResultPtr->SetTotalTimeLimit (pTotaltime);
}

long long User::GetTotalTimeLimit () const
{
    return vResultPtr->GetTotalTimeLimit ();
}

void User::UpdateElapsedTimeInQuiz (long long pTimeElapsed)
{
    vResultPtr->UpdateTimeElapsed (pTimeElapsed);
}

void User::AddToElapsedTimeInQuiz (long long pTimeElapsed)
{
    vResultPtr->AddToElapsedTime (pTimeElapsed);
}

long long User::GetElapsedTime ()
{
    return vResultPtr->GetTimeElapsedInQuiz ();
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

std::vector<unsigned int> User::GetUnattemptedQuestionIds () const
{
    return vResultPtr->GetUnattemptedQuestionIds ();
}