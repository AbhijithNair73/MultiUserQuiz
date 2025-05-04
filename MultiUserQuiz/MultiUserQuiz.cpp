// MultiUserQuiz.cpp : Defines the entry point for the application.
//
#include "MultiUserQuiz.h"
#include "QuizMgr.h"

using namespace std;

const std::string gFilename = "QuizBank.xlsx";

int main()
{
    QuizMgr quiz;
    quiz.StartQuiz (gFilename);
    return 0;
}