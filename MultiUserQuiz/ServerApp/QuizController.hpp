#pragma once
#include <nlohmann/json.hpp>
#include "SessionManager.hpp"
#include "QuizStateManager.hpp"
#include "QuizConfig.h"
#include "QuestionBank.h"

using json = nlohmann::json;
using connection_hdl = websocketpp::connection_hdl;

enum class CommandType {
    LOGIN,
    START_QUIZ,
    CONTINUE_QUIZ,
    END_QUIZ,
    FETCH_QUESTION,
    FETCH_UNATTEMPTED,
    SUBMIT_ANSWER,
    LOGOUT,
    UNKNOWN
};

class QuizController {
    private:
    SessionManager & session_mgr;
    QuizStateManager & state_mgr;

    // Command handlers
    json HandleLogin (connection_hdl hdl, const json & request);
    json HandleStartQuiz (connection_hdl hdl, const json & request);
    json HandleContinueQuiz (connection_hdl hdl, const json & request);
    json HandleEndQuiz (connection_hdl hdl, const json & request);
    json HandleFetchQuestion (connection_hdl hdl, const json & request);
    json HandleFetchUnattempted (connection_hdl hdl, const json & request);
    json HandleSubmitAnswer (connection_hdl hdl, const json & request);
    json HandleLogout (connection_hdl hdl, const json & request);

    // Utility methods
    CommandType ParseCommandType (const std::string & type) const;
    bool IsCommandAllowed (CommandType cmd, const std::string & quiz_id) const;
    json CreateErrorResponse (const std::string & message) const;
    long long CalculateQuestionTimer (std::shared_ptr<User> user) const;
    bool CheckTimeElapsed (std::shared_ptr<User> user, eQuizMode mode) const;
    void CalculateElapsedTimeOnDisconnection (std::shared_ptr<User> user) const;

    public:
    QuizController ();

    // Main request processor
    json ProcessRequest (connection_hdl hdl, const json & request);

    // Connection lifecycle
    void OnConnect (connection_hdl hdl);
    void OnDisconnect (connection_hdl hdl);
};