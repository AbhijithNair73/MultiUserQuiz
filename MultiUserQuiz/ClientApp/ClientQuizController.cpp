// ClientQuizController.cpp  
#include "ClientQuizController.hpp"
#include <iostream>
#include <chrono>

ClientQuizController::ClientQuizController ()
    : session_mgr (ClientSessionManager::GetInstance ())
{ }

void ClientQuizController::SetSendMessageCallback (std::function<void (const json &)> callback)
{
    send_message_callback = callback;
}

void ClientQuizController::SetStatusUpdateCallback (std::function<void (const std::string &)> callback)
{
    on_status_update = callback;
}

void ClientQuizController::SetQuestionReceivedCallback (std::function<void (const json &)> callback)
{
    on_question_received = callback;
}

void ClientQuizController::SetResultReceivedCallback (std::function<void (const json &)> callback)
{
    on_result_received = callback;
}

void ClientQuizController::SetErrorCallback (std::function<void (const std::string &)> callback)
{
    on_error = callback;
}

void ClientQuizController::Login (const std::string & username, const std::string & password)
{
    json request = {
        {"type", "LOGIN"},
        {"username", username},
        {"password", password}
    };

    if (send_message_callback) {
        send_message_callback (request);
    }
}

void ClientQuizController::StartQuiz ()
{
    json request = {{"type", "START_QUIZ"}};

    if (send_message_callback) {
        send_message_callback (request);
    }
}

void ClientQuizController::ContinueQuiz ()
{
    json request = {{"type", "CONTINUE_QUIZ"}};

    if (send_message_callback) {
        send_message_callback (request);
    }
}

void ClientQuizController::EndQuiz ()
{
    json request = {{"type", "END_QUIZ"}};

    if (send_message_callback) {
        send_message_callback (request);
    }
}

void ClientQuizController::FetchQuestion (unsigned int question_id)
{
    json request = {
        {"type", "FETCH_QUESTION"},
        {"question_id", question_id}
    };

    session_mgr.SetCurrentQuestion (question_id);

    if (send_message_callback) {
        send_message_callback (request);
    }
}

void ClientQuizController::FetchUnattemptedQuestions ()
{
    json request = {{"type", "FETCH_UNATTEMPTED"}};

    if (send_message_callback) {
        send_message_callback (request);
    }
}

void ClientQuizController::SubmitAnswer (unsigned int question_id, const std::vector<int> & selected_options, long long time_to_attempt)
{
    json request = {
        {"type", "SUBMIT_ANSWER"},
        {"question_id", question_id},
        {"selected_options", selected_options},
        {"time_to_attempt_in_ms", time_to_attempt}
    };

    if (send_message_callback) {
        send_message_callback (request);
    }
}

void ClientQuizController::Logout ()
{
    json request = {{"type", "LOGOUT"}};

    if (send_message_callback) {
        send_message_callback (request);
    }
}

void ClientQuizController::ProcessResponse (const json & response)
{
    try {
        std::string type = response.value ("type", "");

        if (type == "LOGIN_OK") {
            HandleLoginResponse (response);
        } else if (type == "LOGIN_FAIL") {
            HandleError (response);
        } else if (type == "QUIZ_STARTED") {
            HandleQuizStarted (response);
        } else if (type == "QUIZ_RESTARTED") {
            HandleQuizRestarted (response);
        } else if (type == "QUESTION") {
            HandleQuestionResponse (response);
        } else if (type == "ANSWER_SUBMITTED") {
            HandleAnswerSubmitted (response);
        } else if (type == "QUIZ_RESULT") {
            HandleQuizResult (response);
        } else if (type == "QUIZ_ENDED") {
            HandleQuizEnded (response);
        } else if (type == "LOGOUT_OK") {
            HandleLogoutResponse (response);
        } else if (type == "ERROR") {
            HandleError (response);
        } else {
            std::cerr << "Unknown response type: " << type << std::endl;
        }
    } catch (const std::exception & e) {
        if (on_error) {
            on_error ("Failed to process response: " + std::string (e.what ()));
        }
    }
}

void ClientQuizController::HandleLoginResponse (const json & response)
{
    std::string username = response.value ("welcome", "");
    session_mgr.SetUsername (username);
    session_mgr.SetState (ClientState::LOGGED_IN);

    if (response.contains ("note") && response["note"] == "Reconnected") {
        session_mgr.SetReconnection (true);
        if (on_status_update) {
            on_status_update ("Reconnected as " + username);
        }
    } else {
        if (on_status_update) {
            on_status_update ("Logged in as " + username);
        }
    }
}

void ClientQuizController::HandleQuizStarted (const json & response)
{
    session_mgr.SetState (ClientState::QUIZ_ACTIVE);
    session_mgr.UpdateQuizConfig (response);

    if (on_status_update) {
        on_status_update ("Quiz started! Total questions: " + std::to_string (session_mgr.GetTotalQuestions ()));
    }
}

void ClientQuizController::HandleQuizRestarted (const json & response)
{
    session_mgr.SetState (ClientState::QUIZ_ACTIVE);
    session_mgr.UpdateQuizConfig (response);

    if (response.contains ("question_ids")) {
        std::vector<unsigned int> unattempted = response["question_ids"];
        session_mgr.UpdateUnattemptedQuestions (unattempted);
    }

    if (on_status_update) {
        on_status_update ("Quiz resumed! Remaining questions: " + std::to_string (session_mgr.GetUnattemptedQuestions ().size ()));
    }
}

void ClientQuizController::HandleQuestionResponse (const json & response)
{
    session_mgr.UpdateQuizProgress (response);

    if (on_question_received) {
        on_question_received (response);
    }
}

void ClientQuizController::HandleAnswerSubmitted (const json & response)
{
    session_mgr.UpdateQuizProgress (response);

    int status = response.value ("status", -1);
    std::string status_msg;

    switch (status) {
        case 0: status_msg = "Correct!"; break;
        case 1: status_msg = "Incorrect"; break;
        case 2: status_msg = "Partially Correct"; break;
        default: status_msg = "Unknown"; break;
    }

    if (on_status_update) {
        on_status_update ("Answer submitted: " + status_msg + " | Score: " + std::to_string (session_mgr.GetCurrentScore ()));
    }
}

void ClientQuizController::HandleQuizResult (const json & response)
{
    session_mgr.SetState (ClientState::QUIZ_ENDED);

    if (on_result_received) {
        on_result_received (response);
    }
}

void ClientQuizController::HandleQuizEnded (const json & response)
{
    session_mgr.SetState (ClientState::QUIZ_ENDED);

    if (on_status_update) {
        on_status_update ("Quiz ended!");
    }
}

void ClientQuizController::HandleLogoutResponse (const json & response)
{
    session_mgr.Reset ();
    session_mgr.SetState (ClientState::CONNECTED);

    if (on_status_update) {
        on_status_update ("Logged out successfully");
    }
}

void ClientQuizController::HandleError (const json & response)
{
    std::string message = response.value ("message", response.value ("reason", "Unknown error"));

    if (on_error) {
        on_error (message);
    }
}

bool ClientQuizController::IsConnected () const
{
    return session_mgr.GetState () != ClientState::DISCONNECTED;
}

bool ClientQuizController::IsLoggedIn () const
{
    ClientState state = session_mgr.GetState ();
    return state == ClientState::LOGGED_IN || state == ClientState::QUIZ_ACTIVE || state == ClientState::QUIZ_ENDED;
}

bool ClientQuizController::IsQuizActive () const
{
    return session_mgr.GetState () == ClientState::QUIZ_ACTIVE;
}

std::string ClientQuizController::GetStatusString () const
{
    switch (session_mgr.GetState ()) {
        case ClientState::DISCONNECTED: return "Disconnected";
        case ClientState::CONNECTED: return "Connected";
        case ClientState::LOGGED_IN: return "Logged in as " + session_mgr.GetUsername ();
        case ClientState::QUIZ_ACTIVE: return "Quiz active - " + session_mgr.GetUsername ();
        case ClientState::QUIZ_ENDED: return "Quiz ended - " + session_mgr.GetUsername ();
        default: return "Unknown state";
    }
}