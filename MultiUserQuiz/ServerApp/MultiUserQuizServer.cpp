// MultiUserQuizServer.cpp : Defines the entry point for the application.

#include "ConnectionManager.hpp"
#include "../QuizMgr.h"

const std::string gFilename = "QuizBank.xlsx";

int main ()
{
    try {

        QuizMgr quiz;

        if (quiz.InitializeQuizConfigs () == false) {
            std::cerr << "error loading config file" << std::endl;
            return -1;
        }

        if (quiz.InitializeQuestionBank (gFilename) == false) {
            std::cerr << "error parsing question bank" << std::endl;
            return -1;
        }

        ConnectionManager server;
        server.StartServer (9002);

    } catch (const std::exception & e) {

        std::cerr << "Application error: " << e.what () << std::endl;
        return 1;
    }

    return 0;
}
