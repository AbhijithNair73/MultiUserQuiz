#pragma once

#include <string>
#include <vector>
#include <memory>
#include "ClientConnectionManager.hpp"

class QuizClientUI {
    private:
    std::unique_ptr<ClientConnectionManager> connection_mgr;
    bool is_running;

    // UI helper methods
    void ShowMainMenu ();
    void ShowQuizMenu ();
    void ShowQuestion (const json & question);
    void ShowResult (const json & result);
    void HandleUserInput ();

    // Input helpers
    std::string GetStringInput (const std::string & prompt);
    int GetIntInput (const std::string & prompt);
    std::vector<int> GetMultipleChoiceInput (int max_options);

    // Callbacks
    void OnStatusUpdate (const std::string & status);
    void OnQuestionReceived (const json & question);
    void OnResultReceived (const json & result);
    void OnError (const std::string & error);

    public:
    QuizClientUI ();
    ~QuizClientUI ();

    void Run ();
    void Stop ();
};

