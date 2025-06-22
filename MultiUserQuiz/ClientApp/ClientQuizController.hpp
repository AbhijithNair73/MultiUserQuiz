#pragma once

#include <nlohmann/json.hpp>
#include <functional>
#include <string>
#include <vector>
#include "ClientSessionManager.hpp"

using json = nlohmann::json;

class ClientQuizController {

private:
    ClientSessionManager & session_mgr;
    std::function<void (const json &)> send_message_callback;

    // Response handlers
    void HandleLoginResponse (const json & response);
    void HandleQuizStarted (const json & response);
    void HandleQuizRestarted (const json & response);
    void HandleQuestionResponse (const json & response);
    void HandleAnswerSubmitted (const json & response);
    void HandleQuizResult (const json & response);
    void HandleQuizEnded (const json & response);
    void HandleLogoutResponse (const json & response);
    void HandleError (const json & response);

    // UI callbacks
    std::function<void (const std::string &)> on_status_update;
    std::function<void (const json &)> on_question_received;
    std::function<void (const json &)> on_result_received;
    std::function<void (const std::string &)> on_error;

public:
    ClientQuizController ();

    // Set callbacks
    void SetSendMessageCallback (std::function<void (const json &)> callback);
    void SetStatusUpdateCallback (std::function<void (const std::string &)> callback);
    void SetQuestionReceivedCallback (std::function<void (const json &)> callback);
    void SetResultReceivedCallback (std::function<void (const json &)> callback);
    void SetErrorCallback (std::function<void (const std::string &)> callback);

    // Command methods
    void Login (const std::string & username, const std::string & password = "1234");
    void StartQuiz ();
    void ContinueQuiz ();
    void EndQuiz ();
    void FetchQuestion (unsigned int question_id);
    void FetchUnattemptedQuestions ();
    void SubmitAnswer (unsigned int question_id, const std::vector<int> & selected_options, long long time_to_attempt);
    void Logout ();

    // Response processor
    void ProcessResponse (const json & response);

    // Utility methods
    bool IsConnected () const;
    bool IsLoggedIn () const;
    bool IsQuizActive () const;
    std::string GetStatusString () const;
};

