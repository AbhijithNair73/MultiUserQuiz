// QuizController.cpp
#include "QuizController.hpp"
#include "../QuizMgr.h"

QuizController::QuizController ()
    : session_mgr (SessionManager::GetInstance ()),
    state_mgr (QuizStateManager::GetInstance ())
{ }

CommandType QuizController::ParseCommandType (const std::string & type) const
{
    static const std::unordered_map<std::string, CommandType> command_map = {
        {"LOGIN", CommandType::LOGIN},
        {"START_QUIZ", CommandType::START_QUIZ},
        {"CONTINUE_QUIZ", CommandType::CONTINUE_QUIZ},
        {"END_QUIZ", CommandType::END_QUIZ},
        {"FETCH_QUESTION", CommandType::FETCH_QUESTION},
        {"FETCH_UNATTEMPTED", CommandType::FETCH_UNATTEMPTED},
        {"SUBMIT_ANSWER", CommandType::SUBMIT_ANSWER},
        {"LOGOUT", CommandType::LOGOUT}
    };

    auto it = command_map.find (type);
    return (it != command_map.end ()) ? it->second : CommandType::UNKNOWN;
}

bool QuizController::IsCommandAllowed (CommandType cmd, const std::string & quiz_id) const
{
    QuizState state = state_mgr.GetQuizState (quiz_id);

    // Commands allowed after quiz end
    if (state == QuizState::ENDED_TIMEOUT ||
        state == QuizState::ENDED_FORCE_STOPPED ||
        state == QuizState::ENDED_COMPLETED) {
        return cmd == CommandType::LOGIN ||
            cmd == CommandType::END_QUIZ ||
            cmd == CommandType::LOGOUT;
    }

    // All commands allowed during active quiz
    return state == QuizState::IN_PROGRESS || cmd == CommandType::LOGIN;
}

json QuizController::ProcessRequest (connection_hdl hdl, const json & request)
{
    try {
        std::string type_str = request.value ("type", "");
        CommandType cmd = ParseCommandType (type_str);

        // Get quiz ID (assuming global quiz for now)
        std::string quiz_id = "global_quiz";

        // Check if command is allowed based on quiz state
        if (!IsCommandAllowed (cmd, quiz_id) && cmd != CommandType::LOGIN) {
            return CreateErrorResponse ("Quiz has ended. Only result checking is allowed.");
        }

        switch (cmd) {
            case CommandType::LOGIN:
                return HandleLogin (hdl, request);
            case CommandType::START_QUIZ:
                return HandleStartQuiz (hdl, request);
            case CommandType::CONTINUE_QUIZ:
                return HandleContinueQuiz (hdl, request);
            case CommandType::END_QUIZ:
                return HandleEndQuiz (hdl, request);
            case CommandType::FETCH_QUESTION:
                return HandleFetchQuestion (hdl, request);
            case CommandType::FETCH_UNATTEMPTED:
                return HandleFetchUnattempted (hdl, request);
            case CommandType::SUBMIT_ANSWER:
                return HandleSubmitAnswer (hdl, request);
            case CommandType::LOGOUT:
                return HandleLogout (hdl, request);
            default:
                return CreateErrorResponse ("Unknown command");
        }
    } catch (const std::exception & e) {
        return CreateErrorResponse ("Request processing failed: " + std::string (e.what ()));
    }
}

json QuizController::HandleLogin (connection_hdl hdl, const json & request)
{
    std::string username = request.value ("username", "");
    std::string password = request.value ("password", "");

    if (password != "1234") {
        return {{"type", "LOGIN_FAIL"}, {"reason", "Invalid credentials"}};
    }

    if (session_mgr.IsUserLoggedIn (username)) {
        return {{"type", "LOGIN_FAIL"}, {"reason", "User already logged in"}};
    }

    // Check if reconnection
    bool is_reconnection = session_mgr.GetUser (username) != nullptr;

    if (!session_mgr.AddSession (hdl, username)) {
        return {{"type", "LOGIN_FAIL"}, {"reason", "Session creation failed"}};
    }

    json response = {{"type", "LOGIN_OK"}, {"welcome", username}};
    if (is_reconnection) {
        response["note"] = "Reconnected";
    }

    return response;
}

json QuizController::HandleStartQuiz (connection_hdl hdl, const json & request)
{
    std::string error_msg;
    if (!session_mgr.ValidateSession (hdl, error_msg)) {
        return CreateErrorResponse (error_msg);
    }

    std::string username = session_mgr.GetUsername (hdl);
    if (session_mgr.GetUser (username) != nullptr) {
        return CreateErrorResponse ("Quiz already started");
    }

    // Create user and initialize quiz
    if (!session_mgr.CreateUser (username)) {
        return CreateErrorResponse ("Failed to create user session");
    }

    auto user = session_mgr.GetUser (username);
    QuizConfig & cfg = QuizConfig::GetInstance ();
    eQuizMode quiz_mode = cfg.GetQuizMode ();
    unsigned int ques_count = QuestionBank::GetInstance ().TotalQuestionCount ();
    long long time_allowed_in_ms = cfg.GetTimeAllowedBasedOnQuizMode () * 1000;

    // Configure user based on quiz mode
    if (quiz_mode != BULLET_TIMER_MODE) {

        user->SetTotalTimeLimit (time_allowed_in_ms);

        if (quiz_mode == STRICT_TIME_BOUND_MODE) {
            user->SetStartTimeInMs (QuizHelper::get_current_time_in_ms ());
            user->SetEndTimeInMs (user->GetStartTimeInMs () + time_allowed_in_ms);

            // Start global quiz timer
            std::string quiz_id = "global_quiz";

            if (!state_mgr.IsQuizActive (quiz_id)) {

                state_mgr.StartQuiz (quiz_id, time_allowed_in_ms);

                // Register notification callback
                state_mgr.RegisterClient (quiz_id, [this] (const std::string & message) {
                    session_mgr.NotifyAllUsers (message);
                                          });
            }
        }
    } else {
        user->SetTotalTimeLimit (time_allowed_in_ms * ques_count);
    }

    return {
        {"type", "QUIZ_STARTED"},
        {"total_questions", ques_count},
        {"quiz_mode", quiz_mode},
        {"is_multioption_allowed", cfg.IsMultiOptionSelect ()},
        {"is_kbc_mode", cfg.IsKBCMode ()},
        {"total_time", user->GetTotalTimeLimit ()},
        {"end_time", user->GetEndTimeInMs ()}
    };
}

json QuizController::HandleContinueQuiz (connection_hdl hdl, const json & request)
{
    std::string error_msg;
    if (!session_mgr.ValidateSession (hdl, error_msg)) {
        return CreateErrorResponse (error_msg);
    }

    std::string username = session_mgr.GetUsername (hdl);
    auto user = session_mgr.GetUser (username);

    if (!user) {
        return CreateErrorResponse ("Quiz was not started");
    }

    QuizConfig & cfg = QuizConfig::GetInstance ();
    eQuizMode quiz_mode = cfg.GetQuizMode ();

    // Check if quiz time has elapsed
    if (CheckTimeElapsed (user, quiz_mode)) {
        return CreateErrorResponse ("Quiz time has elapsed");
    }

    // Get unattempted questions
    const std::vector<unsigned int> unattempted = user->GetUnattemptedQuestionIds ();

    if (unattempted.empty ()) {
        return {{"type", "QUIZ_ENDED"}};
    }

    return {
        {"type", "QUIZ_RESTARTED"},
        {"total_questions", QuestionBank::GetInstance ().TotalQuestionCount ()},
        {"quiz_mode", quiz_mode},
        {"is_multioption_allowed", cfg.IsMultiOptionSelect ()},
        {"is_kbc_mode", cfg.IsKBCMode ()},
        {"total_time", user->GetTotalTimeLimit ()},
        {"updated_elapsed_time", user->GetElapsedTime ()},
        {"end_time", user->GetEndTimeInMs ()},
        {"question_ids", unattempted}
    };
}

json QuizController::HandleEndQuiz (connection_hdl hdl, const json & request)
{
    std::string error_msg;
    if (!session_mgr.ValidateSession (hdl, error_msg)) {
        return CreateErrorResponse (error_msg);
    }

    auto user = session_mgr.GetUserByHandle (hdl);
    if (!user) {
        return CreateErrorResponse ("No active quiz found");
    }

    // TODO: Generate and return quiz results
    return {
        {"type", "QUIZ_RESULT"},
        {"score", user->GetUserCurrentScore ()},
        {"total_time", user->GetTotalTimeLimit ()},
        {"elapsed_time", user->GetElapsedTime ()}
        // Add more result details as needed
    };
}

json QuizController::HandleFetchQuestion (connection_hdl hdl, const json & request)
{
    std::string error_msg;
    if (!session_mgr.ValidateSession (hdl, error_msg)) {
        return CreateErrorResponse (error_msg);
    }

    auto user = session_mgr.GetUserByHandle (hdl);
    if (!user) {
        return CreateErrorResponse ("Start the quiz first");
    }

    unsigned int qid = request.value ("question_id", 0);
    QuestionBank & qb = QuestionBank::GetInstance ();

    if (qid <= 0 || qid > qb.TotalQuestionCount ()) {
        return CreateErrorResponse ("Invalid question ID");
    }

    if (CheckTimeElapsed (user, QuizConfig::GetInstance ().GetQuizMode ())) {
        return CreateErrorResponse ("Quiz time has elapsed");
    }

    user->SetLastActivityTimeInMs ();

    auto question = qb.GetQuestionById (qid);

    return {
        {"type", "QUESTION"},
        {"id", question->GetQuestionID ()},
        {"text", question->GetQuestionText ()},
        {"options", question->GetQuestionOptions ()},
        {"total_time", user->GetTotalTimeLimit ()},
        {"updated_elapsed_time", user->GetElapsedTime ()},
        {"question_timer", CalculateQuestionTimer (user)}
    };
}

json QuizController::HandleFetchUnattempted (connection_hdl hdl, const json & request)
{
    std::string error_msg;
    if (!session_mgr.ValidateSession (hdl, error_msg)) {
        return CreateErrorResponse (error_msg);
    }

    auto user = session_mgr.GetUserByHandle (hdl);
    if (!user) {
        return CreateErrorResponse ("Start the quiz first");
    }

    if (CheckTimeElapsed (user, QuizConfig::GetInstance ().GetQuizMode ())) {
        return CreateErrorResponse ("Quiz time has elapsed");
    }

    // Get unattempted questions
    const std::vector<unsigned int> unattempted = user->GetUnattemptedQuestionIds ();

    if (unattempted.empty ()) {
        return {{"type", "QUIZ_ENDED"}};
    }

    return {
        {"type", "UNATTEMPTED_QUESTIONS"},
        {"question_ids", unattempted}
    };
}

json QuizController::HandleSubmitAnswer (connection_hdl hdl, const json & request)
{
    std::string error_msg;

    if (!session_mgr.ValidateSession (hdl, error_msg)) {
        return CreateErrorResponse (error_msg);
    }

    auto user = session_mgr.GetUserByHandle (hdl);
    if (!user) {
        return CreateErrorResponse ("Start the quiz first");
    }

    if (CheckTimeElapsed (user, QuizConfig::GetInstance ().GetQuizMode ())) {
        return CreateErrorResponse ("Quiz time has elapsed");
    }

    unsigned int qid = request.value ("question_id", 0);
    QuestionBank & qb = QuestionBank::GetInstance ();

    if (qid <= 0 || qid > qb.TotalQuestionCount ()) {
        return CreateErrorResponse ("Invalid question ID");
    }

    user->SetLastActivityTimeInMs ();

    Answer ans (qid);
    for (int op : request.value ("selected_options", std::vector<int>{})) {
        ans.SetSelectedOp (op);
    }

    long long time_to_attempt = request.value ("time_to_attempt_in_ms", 0);
    user->AddToElapsedTimeInQuiz (time_to_attempt);

    eQuesAttemptStatus status = user->SetAndValidateUserAnswer (ans);
    double score = user->GetUserCurrentScore ();

    return {
        {"type", "ANSWER_SUBMITTED"},
        {"question_id", qid},
        {"status", static_cast<int>(status)},
        {"score", score},
        {"total_time", user->GetTotalTimeLimit ()},
        {"updated_elapsed_time", user->GetElapsedTime ()}
    };
}

json QuizController::HandleLogout (connection_hdl hdl, const json & request)
{
    std::string username = session_mgr.GetUsername (hdl);
    auto user = session_mgr.GetUser (username);

    if (user) {
        CalculateElapsedTimeOnDisconnection (user);
    }

    session_mgr.RemoveSession (hdl);

    return {{"type", "LOGOUT_OK"}, {"Bye", username}};
}

void QuizController::OnConnect (connection_hdl hdl)
{
// Connection established - no action needed
}

void QuizController::OnDisconnect (connection_hdl hdl)
{
    std::string username = session_mgr.GetUsername (hdl);
    auto user = session_mgr.GetUser (username);

    if (user) {
        CalculateElapsedTimeOnDisconnection (user);
    }

    session_mgr.RemoveSession (hdl);
}

// Utility method implementations
json QuizController::CreateErrorResponse (const std::string & message) const
{
    return {{"type", "ERROR"}, {"message", message}};
}

bool QuizController::CheckTimeElapsed (std::shared_ptr<User> user, eQuizMode mode) const
{
    if (mode == BULLET_TIMER_MODE) {

        return false;
    } else if (mode == STRICT_TIME_BOUND_MODE) {

        long long current_time = QuizHelper::get_current_time_in_ms ();
        return current_time >= user->GetEndTimeInMs ();

    } else if (mode == TIME_BOUND_MODE) {

        return user->GetElapsedTime () >= user->GetTotalTimeLimit ();
    }
    return false;
}

long long QuizController::CalculateQuestionTimer (std::shared_ptr<User> user) const
{
    QuizConfig & cfg = QuizConfig::GetInstance ();
    eQuizMode quiz_mode = cfg.GetQuizMode ();

    if (quiz_mode == BULLET_TIMER_MODE) {
        return cfg.GetTimeAllowedBasedOnQuizMode () * 1000;
    } else if (quiz_mode == TIME_BOUND_MODE) {
        return user->GetTotalTimeLimit () - user->GetElapsedTime ();
    } else {
        return user->GetEndTimeInMs () - QuizHelper::get_current_time_in_ms ();
    }
}

void QuizController::CalculateElapsedTimeOnDisconnection (std::shared_ptr<User> user) const
{
    QuizConfig & cfg = QuizConfig::GetInstance ();
    eQuizMode quiz_mode = cfg.GetQuizMode ();
    long long last_activity_time = user->GetlastActivityTimeInMs ();

    if (last_activity_time == 0) {
        return; // No activity recorded
    }

    if (quiz_mode == TIME_BOUND_MODE) {
        long long current_time = QuizHelper::get_current_time_in_ms ();
        long long elapsed_time = current_time - last_activity_time;
        user->AddToElapsedTimeInQuiz (elapsed_time);
    }

    user->ResetLastActivityTimeInMs ();
}