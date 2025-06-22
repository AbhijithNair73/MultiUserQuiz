#include "User.h"

User::User (const std::string & pUserName)
{
    vResultPtr = std::make_unique<Result> ();
    vUserName = pUserName;
    ResetLastActivityTimeInMs ();
}

User::~User ()
{
    // nothing to do
}

void User::SetStartTimeInMs (long long pStartTime)
{
    vStartTime = pStartTime;
}

void User::SetEndTimeInMs (long long pTime)
{
    vEndTime = pTime;
}

long long User::GetStartTimeInMs () const
{ 
    return vStartTime;
}

long long User::GetEndTimeInMs () const
{
    return vEndTime;
}

void User::SetLastActivityTimeInMs ()
{
    vLastActivityTime = QuizHelper::get_current_time_in_ms ();
}

void User::ResetLastActivityTimeInMs ()
{
    vLastActivityTime = 0;
}

long long User::GetlastActivityTimeInMs () const
{
    return vLastActivityTime;
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