#include "ConnectionMgr.hpp"
#include "../QuizMgr.h"

std::map<connection_hdl, std::string, std::owner_less<connection_hdl>> hdl_to_username;
std::unordered_map<std::string, std::shared_ptr<User>> username_to_user;
std::shared_mutex session_mutex;

std::shared_ptr<asio::ssl::context> on_tls_init (websocketpp::connection_hdl hdl)
{
    auto ctx = std::make_shared<asio::ssl::context> (asio::ssl::context::tlsv12);
    try {
        ctx->set_options (
            asio::ssl::context::default_workarounds |
            asio::ssl::context::no_sslv2 |
            asio::ssl::context::single_dh_use);

        ctx->use_certificate_chain_file ("server.crt");
        ctx->use_private_key_file ("server.key", asio::ssl::context::pem);

        std::cout << "TLS init succeeded." << std::endl;
    } catch (std::exception & e) {
        std::cout << "TLS init failed: " << e.what () << std::endl;
    }
    return ctx;
}

// TODO: timer based check till which point the user will be allowed to login/relogin
json handle_login_request (connection_hdl hdl, string password, string username)
{
        json response;

    if (password != "1234") {
        return {{"type", "LOGIN_FAIL"}, {"reason", "Invalid credentials"}};
    }

    {
        std::unique_lock lock (session_mutex);

        // Check if already logged in from another connection - some fake user trying to connect or trying to make 2 connections for same user.
        for (const auto & [existing_hdl, existing_user] : hdl_to_username) {
            if (existing_user == username) {
                return {{"type", "LOGIN_FAIL"}, {"reason", "User already logged in"}};
            }
        }

        // Allow re-login only if quiz already started (user object exists)
        if (username_to_user.find (username) != username_to_user.end ()) {
            hdl_to_username[hdl] = username;
            return {{"type", "LOGIN_OK"}, {"welcome", username}, {"note", "Reconnected"}};
        }

        // Fresh login only if user hasn't already started quiz
        hdl_to_username[hdl] = username;
    }

    return {{"type", "LOGIN_OK"}, {"welcome", username}};
}

json handle_start_quiz (connection_hdl hdl)
{
        std::string uname;
        unsigned int ques_count = QuestionBank::GetInstance ().TotalQuestionCount ();
        QuizConfig & cfg = QuizConfig::GetInstance ();
        bool is_bullet_mode = (cfg.GetQuizMode () == BULLET_TIMER_MODE);
        long long time_allowed = cfg.GetTimeAllowedBasedOnQuizMode ();

    {
        std::shared_lock lock (session_mutex);
        auto it = hdl_to_username.find (hdl);
        if (it == hdl_to_username.end ()) {
            return {{"type", "ERROR"}, {"message", "Please login first"}};
        }
        uname = it->second;

        if (username_to_user.find (uname) != username_to_user.end ()) {

            return {{"type", "ERROR"}, {"message", "Quiz already started"}};
        }
    }

    {
        std::unique_lock lock (session_mutex);
        username_to_user[uname] = std::make_shared<User> (uname);

        if (!is_bullet_mode) {
            // time is managed by server in timebound mode and by client in bullet mode.
            username_to_user[uname]->SetTotalTimeLimit (time_allowed);
        } else {
            // this is for tracking and for fastest finger first result - tracking time in bullet mode.
            username_to_user[uname]->SetTotalTimeLimit (time_allowed * ques_count);
        }
    }


    return {
        {"type", "QUIZ_STARTED"}, 
        {"total_questions", ques_count},
        {"is_bullet_mode", is_bullet_mode},
        {"is_multioption_allowed", cfg.IsMultiOptionSelect ()},
        {"is_kbc_mode", cfg.IsKBCMode ()},
        {"time_allowed", cfg.GetTimeAllowedBasedOnQuizMode ()}
    };
}

json handle_continue_quiz (connection_hdl hdl)
{

    unsigned int ques_count = QuestionBank::GetInstance ().TotalQuestionCount ();
    QuizConfig & cfg = QuizConfig::GetInstance ();
    bool is_bullet_mode = (cfg.GetQuizMode () == BULLET_TIMER_MODE);

    std::shared_ptr<User> user;
    {
        std::shared_lock lock (session_mutex);
        auto it = hdl_to_username.find (hdl);
        if (it == hdl_to_username.end ()) {
            return {{"type", "ERROR"}, {"message", "Please login first"}};
        }

        auto uit = username_to_user.find (it->second);
        if (uit == username_to_user.end ()) {

            return {{"type", "ERROR"}, {"message", "Quiz was not started"}};
        }
        user = uit->second;

    }

    if (!is_bullet_mode) {

        // this is a reconnection or relogin for an already running quiz in flexible time mode.
        // so reconnection time loss should not be accounted in quiz time and user should get the same time as before reconnection.
        long long new_time_limit = user->GetTotalTimeLimit() - user->GetElapsedTime ();
        if (new_time_limit <= 0) {
            // If the time limit is already over, we should not allow the user to continue.
            return {{"type", "ERROR"}, {"message", "Time limit already over"}};
        }

        // start time should be updated to current time.
        user->SetStartTime (QuizHelper::get_current_time_in_ms ());

        // update the total time - bcoz now again disconnection can happen and then the reconnection time loss calculating will be difficult.
        user->SetTotalTimeLimit (new_time_limit);

        user->UpdateElapsedTimeInQuiz (0);

        // In strict time mode: start time will remain same and time loss should be considered in case of disconnection hence.
        // new_time_limit = QuizHelper::get_current_time_in_ms () - user->GetStartTime ()
        // // current time - start time - indicates the time elapsed. and total time will remain same.
        // user->UpdateElapsedTimeInQuiz (new_time_limit);
    }

    // fetch unattempted questions.
    const std::vector<unsigned int> unattempted = user->GetUnattemptedQuestionIds ();

    return {
        {"type", "QUIZ_RESTARTED"},
        {"total_questions", ques_count},
        {"is_bullet_mode", is_bullet_mode},
        {"is_multioption_allowed", cfg.IsMultiOptionSelect ()},
        {"is_kbc_mode", cfg.IsKBCMode ()},
        {"time_allowed", cfg.GetTimeAllowedBasedOnQuizMode ()},
        {"updated_total_time", user->GetTotalTimeLimit ()},
        {"updated_elapsed_time", user->GetElapsedTime ()},
        {"question_ids", unattempted}
    };
}


// returning the user outside the mutex lock is fine bcz we are sure that same username cannot exist from 2 handles.
// coz the client is synchronous it will wait for the response of this request before proceeding to any other request.
std::shared_ptr<User> check_and_return_user (connection_hdl hdl, json & pOutResp)
{
    std::shared_lock lock (session_mutex);
    auto it = hdl_to_username.find (hdl);
    if (it == hdl_to_username.end ()) {
        pOutResp = {{"type", "ERROR"}, {"message", "Please login first"}};
        return nullptr;
    }

    auto uit = username_to_user.find (it->second);
    if (uit == username_to_user.end ()) {

        pOutResp = {{"type", "ERROR"}, {"message", "Start the quiz first"}};
        return nullptr;
    }

    return uit->second;
}

string perform_logout (connection_hdl hdl) 
{
    std::string uname;
    {
        std::unique_lock lock (session_mutex);
        auto it = hdl_to_username.find (hdl);
        if (it != hdl_to_username.end ()) {
            uname = it->second;
            hdl_to_username.erase (it);
        }
    }
    return uname;
}

json handle_fetch_unattempted_ques (connection_hdl hdl)
{
    json response;

    std::shared_ptr<User> user = check_and_return_user (hdl, response);

    if (user == nullptr) {
        return response;
    }

    const std::vector<unsigned int> unattempted = user->GetUnattemptedQuestionIds ();

    response = {
        {"type", "UNATTEMPTED_QUESTIONS"},
        {"question_ids", unattempted}
    };

    return response;
}

json process_request (connection_hdl hdl, json & req)
{
        json            response;
        std::string     type = req["type"];
        QuestionBank &  qb = QuestionBank::GetInstance ();

    if (type == "LOGIN") {

        const std::string username = req.value ("username", "");
        const std::string password = req.value ("password", "");

        return  handle_login_request (hdl, password, username);
    }

    else if (type == "START_QUIZ") {

        return handle_start_quiz (hdl);
    }
    else if (type == "CONTINUE_QUIZ") {

        // should be called after reconnection
        return handle_continue_quiz (hdl);
    }
    else if (type == "END_QUIZ") {

        // show result. this will be called in case of abort.
        std::shared_ptr<User> user = check_and_return_user (hdl, response);

        if (user == nullptr) {
            return response;
        }


    }
    else if (type == "FETCH_QUESTION") {

        std::shared_ptr<User> user = check_and_return_user (hdl, response);

        if (user == nullptr) {
            return response;
        }

        unsigned int qid = req.value ("question_id", 0);
        if (qid <= 0 || qid > qb.TotalQuestionCount ()) {

            return {{"type", "ERROR"}, {"message", "Invalid question ID"}};
        }

        // Check if it is allowed or not and update the time elapsed.


        std::shared_ptr<const Question> q = qb.GetQuestionById (qid);

        response = {
            {"type",    "QUESTION"},
            {"id",      q->GetQuestionID ()},
            {"text",    q->GetQuestionText ()},
            {"options", q->GetQuestionOptions ()},
            {"updated_total_time", user->GetTotalTimeLimit ()},
            {"updated_elapsed_time", user->GetElapsedTime ()},
        };
    }
    else if (type == "FETCH_UNATTEMPTED") {

        return handle_fetch_unattempted_ques (hdl);

    }
    else if (type == "SUBMIT_ANSWER") {

        std::shared_ptr<User> user = check_and_return_user (hdl, response);

        if (user == nullptr) {
            return response;
        }

        unsigned int qid = req.value ("question_id", 0);
        if (qid <= 0 || qid > qb.TotalQuestionCount ()) {

            return {{"type", "ERROR"}, {"message", "Invalid question ID"}};
        }

        Answer ans (qid);
        for (int op : req.value ("selected_options", std::vector<int>{})) {

            ans.SetSelectedOp (op);
        }


        long long time_to_attempt = req.value ("time_to_attempt", 0);
        user->AddToElapsedTimeInQuiz (time_to_attempt);

        // check if the answer submission is allowed or not or the timer has expired.

        // TODO: status and score should be sent optionally based on KBC mode
        eQuesAttemptStatus status = user->SetAndValidateUserAnswer (ans);
        double score = user->GetUserCurrentScore ();

        long long time_elapsed = user->GetElapsedTime ();

        
        response = {
            {"type", "ANSWER_SUBMITTED"},
            {"question_id", qid},
            {"status",  static_cast<int>(status)},
            {"score", score},
            {"updated_total_time", user->GetTotalTimeLimit ()},
            {"updated_elapsed_time", user->GetElapsedTime ()},
        };

    }
    else if (type == "LOGOUT") {

        string uname = perform_logout (hdl);
        response = {{"type", "LOGOUT_OK"}, {"Bye", uname}};

    }
    else {
        // Unknown command
        response = {{"type", "ERROR"}, {"message", "Unknown command"}};
    }

    return response;
}

void on_message (server * s, connection_hdl hdl, server::message_ptr msg)
{
    try {

#ifdef DEBUG
        server::connection_ptr con = s->get_con_from_hdl (hdl);
        std::string client_info = con->get_remote_endpoint ();
        std::cout << "[MESSAGE] From " << client_info << ": " << msg->get_payload () << std::endl;
#endif

        json req = json::parse (msg->get_payload ());
        json response = process_request (hdl, req);

        s->send (hdl, response.dump (), websocketpp::frame::opcode::text);

    } catch (const std::exception & e) {

        std::cerr << "Exception: " << e.what () << std::endl;
    }
}

void on_open (server * s, connection_hdl hdl)
{
    auto con = s->get_con_from_hdl (hdl);
    std::string remote = con->get_remote_endpoint ();
    std::cout << "[CONNECTED] " << remote << std::endl;
}

void on_close (server * s, connection_hdl hdl)
{
    perform_logout (hdl);
}

void ConnectionMgr::StartServer ()
{
    server ws_server;

    try {

        ws_server.set_access_channels (websocketpp::log::alevel::none);
        ws_server.init_asio ();
        ws_server.set_tls_init_handler (bind (&on_tls_init, _1));
        ws_server.set_message_handler (bind (&on_message, &ws_server, _1, _2));
        ws_server.listen (9002);
        ws_server.start_accept ();

        // --- THREAD POOL START ---
        const int num_threads = std::thread::hardware_concurrency (); // or a fixed number

        std::vector<std::thread> threads;

        for (int i = 0; i < num_threads; ++i) {

            threads.emplace_back ([&ws_server] () {
                ws_server.run (); // each thread runs the IO loop
            });
        }

        // Join threads
        for (auto & t : threads) {
            t.join ();
        }

        // --- THREAD POOL END ---

    } catch (const std::exception & e) {
        std::cerr << "Error: " << e.what () << std::endl;
    }
}