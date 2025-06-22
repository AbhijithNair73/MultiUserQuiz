
// QuizClientUI.cpp (Continued from where it was cut off)
#include "QuizClientUI.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <thread>
#include <chrono>

QuizClientUI::QuizClientUI ()
    : connection_mgr (std::make_unique<ClientConnectionManager> ()),
    is_running (false)
{

    // Set up callbacks
    auto & controller = connection_mgr->GetQuizController ();

    controller.SetStatusUpdateCallback ([this] (const std::string & status) {
        OnStatusUpdate (status);
                                        });

    controller.SetQuestionReceivedCallback ([this] (const json & question) {
        OnQuestionReceived (question);
                                            });

    controller.SetResultReceivedCallback ([this] (const json & result) {
        OnResultReceived (result);
                                          });

    controller.SetErrorCallback ([this] (const std::string & error) {
        OnError (error);
                                 });
}

QuizClientUI::~QuizClientUI ()
{
    Stop ();
}

void QuizClientUI::Run ()
{
    std::cout << "=== Welcome to Quiz Client ===" << std::endl;

    // Connect to server
    if (!connection_mgr->ConnectToServer ()) {
        std::cout << "Failed to connect to server!" << std::endl;
        return;
    }

    // Wait for connection
    std::this_thread::sleep_for (std::chrono::milliseconds (1000));

    if (!connection_mgr->IsConnected ()) {
        std::cout << "Connection timeout!" << std::endl;
        return;
    }

    is_running = true;
    HandleUserInput ();
}

void QuizClientUI::Stop ()
{
    is_running = false;
    if (connection_mgr) {
        connection_mgr->Disconnect ();
    }
}

void QuizClientUI::HandleUserInput ()
{
    auto & controller = connection_mgr->GetQuizController ();

    while (is_running && connection_mgr->IsConnected ()) {
        if (!controller.IsLoggedIn ()) {
            ShowMainMenu ();
        } else if (controller.IsQuizActive ()) {
            ShowQuizMenu ();
        } else {
            // Logged in but no active quiz
            std::cout << "\n=== Quiz Menu ===" << std::endl;
            std::cout << "1. Start Quiz" << std::endl;
            std::cout << "2. Continue Quiz" << std::endl;
            std::cout << "3. End Quiz (Get Results)" << std::endl;
            std::cout << "4. Logout" << std::endl;
            std::cout << "5. Exit" << std::endl;
            std::cout << "Status: " << controller.GetStatusString () << std::endl;

            int choice = GetIntInput ("Enter choice: ");

            switch (choice) {
                case 1:
                    controller.StartQuiz ();
                    break;
                case 2:
                    controller.ContinueQuiz ();
                    break;
                case 3:
                    controller.EndQuiz ();
                    break;
                case 4:
                    controller.Logout ();
                    break;
                case 5:
                    is_running = false;
                    break;
                default:
                    std::cout << "Invalid choice!" << std::endl;
                    break;
            }
        }
    }
}

void QuizClientUI::ShowMainMenu ()
{
    auto & controller = connection_mgr->GetQuizController ();

    std::cout << "\n=== Main Menu ===" << std::endl;
    std::cout << "1. Login" << std::endl;
    std::cout << "2. Exit" << std::endl;
    std::cout << "Status: " << controller.GetStatusString () << std::endl;

    int choice = GetIntInput ("Enter choice: ");

    switch (choice) {
        case 1:
        {
            std::string username = GetStringInput ("Username: ");
            std::string password = GetStringInput ("Password (default: 1234): ");
            if (password.empty ()) {
                password = "1234";
            }
            controller.Login (username, password);
            break;
        }
        case 2:
            is_running = false;
            break;
        default:
            std::cout << "Invalid choice!" << std::endl;
            break;
    }
}

void QuizClientUI::ShowQuizMenu ()
{
    auto & controller = connection_mgr->GetQuizController ();
    auto & session = ClientSessionManager::GetInstance ();

    std::cout << "\n=== Active Quiz ===" << std::endl;
    std::cout << "1. Fetch Question by ID" << std::endl;
    std::cout << "2. Fetch Unattempted Questions" << std::endl;
    std::cout << "3. Submit Answer for Current Question" << std::endl;
    std::cout << "4. End Quiz" << std::endl;
    std::cout << "5. Logout" << std::endl;

    std::cout << "\nQuiz Info:" << std::endl;
    std::cout << "Total Questions: " << session.GetTotalQuestions () << std::endl;
    std::cout << "Current Score: " << session.GetCurrentScore () << std::endl;
    std::cout << "Elapsed Time: " << session.GetElapsedTime () << "ms" << std::endl;
    std::cout << "Unattempted Questions: " << session.GetUnattemptedQuestions ().size () << std::endl;

    int choice = GetIntInput ("Enter choice: ");

    // TODO: Below logic of displaying question and submitting answer to change
    switch (choice) {
        case 1:
        {
            unsigned int qid = GetIntInput ("Enter Question ID: ");
            controller.FetchQuestion (qid);
            break;
        }
        case 2:
            controller.FetchUnattemptedQuestions ();
            break;
        case 3:
        {
            unsigned int qid = session.GetCurrentQuestion ();
            if (qid == 0) {
                qid = GetIntInput ("Enter Question ID to answer: ");
            }

            std::cout << "Enter selected options (space-separated, 0-based indexing): ";
            std::vector<int> options = GetMultipleChoiceInput (10); // Assume max 10 options

            long long time_taken = GetIntInput ("Enter time taken (ms): ");

            controller.SubmitAnswer (qid, options, time_taken);
            break;
        }
        case 4:
            controller.EndQuiz ();
            break;
        case 5:
            controller.Logout ();
            break;
        default:
            std::cout << "Invalid choice!" << std::endl;
            break;
    }
}

void QuizClientUI::ShowQuestion (const json & question)
{
    std::cout << "\n=== QUESTION ===" << std::endl;

    if (question.contains ("question_id")) {
        std::cout << "Question ID: " << question["question_id"] << std::endl;
    }

    if (question.contains ("question_text")) {
        std::cout << "Question: " << question["question_text"] << std::endl;
    } else if (question.contains ("question")) {
        std::cout << "Question: " << question["question"] << std::endl;
    }

    if (question.contains ("options")) {
        std::cout << "Options:" << std::endl;
        auto options = question["options"];
        for (size_t i = 0; i < options.size (); ++i) {
            std::cout << "  " << i << ". " << options[i] << std::endl;
        }
    }

    if (question.contains ("type")) {
        std::cout << "Type: " << question["type"] << std::endl;
    }

    if (question.contains ("difficulty")) {
        std::cout << "Difficulty: " << question["difficulty"] << std::endl;
    }

    if (question.contains ("score")) {
        std::cout << "Points: " << question["score"] << std::endl;
    }

    std::cout << "=================" << std::endl;
}

void QuizClientUI::ShowResult (const json & result)
{
    std::cout << "\n=== QUIZ RESULTS ===" << std::endl;

    if (result.contains ("final_score")) {
        std::cout << "Final Score: " << result["final_score"] << std::endl;
    }

    if (result.contains ("total_questions")) {
        std::cout << "Total Questions: " << result["total_questions"] << std::endl;
    }

    if (result.contains ("attempted_questions")) {
        std::cout << "Attempted Questions: " << result["attempted_questions"] << std::endl;
    }

    if (result.contains ("correct_answers")) {
        std::cout << "Correct Answers: " << result["correct_answers"] << std::endl;
    }

    if (result.contains ("incorrect_answers")) {
        std::cout << "Incorrect Answers: " << result["incorrect_answers"] << std::endl;
    }

    if (result.contains ("percentage")) {
        std::cout << "Percentage: " << result["percentage"] << "%" << std::endl;
    }

    if (result.contains ("time_taken")) {
        std::cout << "Total Time Taken: " << result["time_taken"] << "ms" << std::endl;
    }

    if (result.contains ("rank")) {
        std::cout << "Rank: " << result["rank"] << std::endl;
    }

    std::cout << "====================" << std::endl;
}

std::string QuizClientUI::GetStringInput (const std::string & prompt)
{
    std::cout << prompt;
    std::string input;
    std::getline (std::cin, input);
    return input;
}

int QuizClientUI::GetIntInput (const std::string & prompt)
{
    std::string input;
    int value;

    while (true) {
        std::cout << prompt;
        std::getline (std::cin, input);

        try {
            value = std::stoi (input);
            break;
        } catch (const std::exception &) {
            std::cout << "Invalid input. Please enter a number." << std::endl;
        }
    }

    return value;
}

std::vector<int> QuizClientUI::GetMultipleChoiceInput (int max_options)
{
    std::string input;
    std::getline (std::cin, input);

    std::vector<int> options;
    std::istringstream iss (input);
    std::string token;

    while (iss >> token) {
        try {
            int option = std::stoi (token);
            if (option >= 0 && option < max_options) {
                options.push_back (option);
            }
        } catch (const std::exception &) {
            // Ignore invalid tokens
        }
    }

    return options;
}

void QuizClientUI::OnStatusUpdate (const std::string & status)
{
    std::cout << "[STATUS] " << status << std::endl;
}

void QuizClientUI::OnQuestionReceived (const json & question)
{
    ShowQuestion (question);
}

void QuizClientUI::OnResultReceived (const json & result)
{
    ShowResult (result);
}

void QuizClientUI::OnError (const std::string & error)
{
    std::cout << "[ERROR] " << error << std::endl;
}
