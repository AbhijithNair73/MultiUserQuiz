// MultiUserQuiz.cpp : Defines the entry point for the application.
//
#include "MultiUserQuiz.h"
#include "QuizMgr.h"

using namespace std;

const std::string gFilename = "QuizBank.xlsx";

int main()
{
        QuizMgr quiz;

    if (quiz.InitializeQuizConfigs () == false) {
        std::cerr << "error loading config file" << std::endl;
        return -1;
    }

    if (quiz.InitializeQuestionBank (gFilename) == false) {
        std::cerr << "error parsing question bank" << std::endl;
        return -1;
    }

    quiz.CreateNewUser ();

    if (quiz.StartQuiz () == false) {
        return -1;
    }

    return 0;
}