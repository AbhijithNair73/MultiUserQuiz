#include "QuizMgr.h"

QuizMgr::QuizMgr ()
{
        QuizConfig & cfg        = QuizConfig::GetInstance ();
    vIsSingleUser               = cfg.IsSingleUser();
    vIsMultipleAnswersAllowed   = cfg.IsMultiOptionSelect();
    vIsKBCMode                  = true;
    vQuizMode                   = cfg.GetQuizMode();
    vUser                       = nullptr;
    vQuizTimeLimit              = cfg.GetTimeAllowedBasedOnQuizMode ();
}

QuizMgr::~QuizMgr ()
{
    // Do nothing
}

/*
    QuizMgr class will be the entry point for client side app
    and also for single user app
    User will get created as part of this
    Login and all if required should be performed
*/
void QuizMgr::CreateNewUser ()
{
    vUser = new User();
    if (!vUser) {
        std::cerr << "Error: Could not create user." << std::endl;
        return;
    }
}

void QuizMgr::OnTimerExpired ()
{
    std::cerr << "[TIMER] Time expired for ";
    if (vQuizMode == TIME_BOUND_MODE)
        std::cerr << "entire quiz.";
    else if (vQuizMode == BULLET_TIMER_MODE)
        std::cerr << "this question.";

    std::cerr << std::endl;

    vTimedOut.store (true);
}

bool QuizMgr::StartQuiz ()
{
        QuestionBank &                  qb = QuestionBank::GetInstance ();
        bool                            isBulletMode;
        std::unique_ptr<QuestionTimer>  quizTimer;

    if (!qb.IsQuestionBankInitialized () || qb.IsQuestionBankEmpty ()) {

        std::cerr << "QB is not initialized or is empty" << std::endl;
        return false;
    }

    if (vUser == nullptr) {
        //user not created yet
        std::cerr << "User not yet created. Create user first." << std::endl;
        return false;
    }

    // Initialize quiz timer & set the timer callback
    vTimedOut.store (false);

    quizTimer = std::make_unique<QuestionTimer> (
        vQuizTimeLimit,
        std::bind (&QuizMgr::OnTimerExpired, this)
    );

    // Initialize quiz timer according to the quizMode
    if (vQuizMode == BULLET_TIMER_MODE) {

        isBulletMode = true;
    } else{

        // Timebound mode - start the quiz timer
        isBulletMode = false;
        vUser->SetTotalTimeLimit (vQuizTimeLimit);
        quizTimer->Start ();
    }

    const unsigned int totalQuestions = qb.TotalQuestionCount ();

    // Question Bank stores question from 1 till N (till size coz we are starting from 1 and not 0).
    for (unsigned int i = 1; i <= totalQuestions; ++i) {

            shared_ptr<const Question> question = qb.GetQuestionById (i);
            eQuesAttemptStatus status;

        if (!question) {
            std::cerr << "Error: Could not retrieve question ID " << i << ".\n";
            return false;
        }

        if (isBulletMode) {

            // case: start the timer for each question in bullet mode
            vTimedOut.store (false);
            quizTimer->Start ();
        }

        question->DisplayQuestion ();

        Answer answer = WaitForUserAnswer (question->GetQuestionID ());

        if (isBulletMode) {

            // stop the timer
            quizTimer->Stop ();

            if (vTimedOut.load ()) {
                // Donot Set the answer - as it will be considered unattempted as timeout happened
                // skip to next question.
                continue; 
            }

        } else {

            // update the time elapsed in Timebound mode after every attempt
            vUser->UpdateElapsedTimeInQuiz (quizTimer->GetElapsedTimeMillis ());
        }

        status = vUser->SetAndValidateUserAnswer (answer);

        // TIMEBOUND Mode and quiz timedout - indicating end of quiz
        if (!isBulletMode && vTimedOut) {

            std::cout << "Time is up! Quiz Ends." << std::endl;
            break;
        }

        if (vIsKBCMode) {

            switch (status) {
                
                case CORRECT:
                    std::cout << "Correct answer!" << std::endl;
                    break;

                case INCORRECT:
                    std::cout << "Incorrect answer!" << std::endl;
                    question->DisplayCorrectAnswers ();
                    break;

                case PARTIALLY_CORRECT:
                    std::cout << "Partially correct answer!" << std::endl;
                    question->DisplayCorrectAnswers ();
                    break;

                case UNATTEMPTED:
                    std::cout << "Question unattempted!" << std::endl;
                    question->DisplayCorrectAnswers ();
                    break;
            }
        }
    }
    // End of quiz
    if (!isBulletMode) {

        // stop the quiz timer for time bound mode
        quizTimer->Stop ();
    }

    // Show result score 
    // Detailed score should be shown for KBC mode - as already correct answers are displayed after each wrong attempt
    vUser->ShowFinalScore (!vIsKBCMode);

    return true;
}

Answer QuizMgr::WaitForUserAnswer (unsigned int pQuesId)
{
    Answer answer (pQuesId);
    const bool allowMultiple = vIsMultipleAnswersAllowed;
    const std::set<char> validOpts = {'A', 'B', 'C', 'D'};
    const std::set<char> noiseChars = {' ', ',', '-', '_'};
    std::set<char> selected;

    while (!vTimedOut.load ()) {

        selected.clear ();

        std::cout << "Enter your answer(s) [A, B, C, D]"
            << (allowMultiple ? " (Multiple allowed)" : " (Single only)")
            << ": ";

        std::string input;
        std::getline (std::cin, input);
        std::transform (input.begin (), input.end (), input.begin (), ::toupper);

        if (!allowMultiple) {
            // Single option mode: must be exactly one valid character
            if (input.size () != 1 || validOpts.count (input[0]) == 0) {
                std::cout << "Invalid input. Enter a single option: A, B, C, or D.\n";
                continue;
            }
            answer.SetSelectedOp (input[0] - 'A');
            return answer;
        }

        // Multi-option mode
        for (char ch : input) {
            if (noiseChars.count (ch)) {
                continue; // skip noise
            } else if (validOpts.count (ch)) {
                if (!selected.insert (ch).second) {
                    std::cout << "Duplicate option '" << ch << "' not allowed.\n";
                    selected.clear ();
                    break;
                }
            } else {
                // invalid non-noise character
                std::cout << "Invalid character '" << ch << "' in input.\n";
                selected.clear ();
                break;
            }
        }

        if (selected.empty ()) {
            std::cout << "Invalid input. Use only A, B, C, D without duplicates.\n";
            continue;
        }

        // in multi-option mode - there cannot be more than 3 options to be correct in any case
        if (selected.size () > MAX_NUMBER_OF_CORRECT_OPTION) {
            std::cout << "Too many options. Max allowed: " << MAX_NUMBER_OF_CORRECT_OPTION << ".\n";
            continue;
        }
    }

    for (char ch : selected) {

        answer.SetSelectedOp (ch - 'A');
    }

    return answer;
}

bool QuizMgr::InitializeQuizConfigs ()
{
        QuizConfig & config = QuizConfig::GetInstance ();

    // Load the config file
    if (!config.LoadConfigFromFile ()) {
        std::cerr << "Error loading config file." << std::endl;
        return false;
    }

    return true;
}

bool QuizMgr::InitializeQuestionBank (const std::string & excelFileName)
{
    xlnt::workbook  wb;
    QuestionBank &  qb = QuestionBank::GetInstance ();

    try {

        wb.load ("QuizBank.xlsx");

        xlnt::worksheet ws = wb.active_sheet ();

        bool is_header = true;
        for (auto row : ws.rows (false)) {
            if (is_header) {
                // Print headers nicely
                // std::cout << "=== Header ===" << std::endl;
                // for (auto cell : row)
                // {
                    // std::cout << cell.to_string() << " | ";
                // }
                // std::cout << std::endl << "===============" << std::endl;
                is_header = false;
                continue;
            }

            // For each question row
            auto cells = row.begin ();

            std::string question_number = (cells != row.end ()) ? (*cells++).to_string () : "";
            std::string question_text = (cells != row.end ()) ? (*cells++).to_string () : "";
            std::string option_a = (cells != row.end ()) ? (*cells++).to_string () : "";
            std::string option_b = (cells != row.end ()) ? (*cells++).to_string () : "";
            std::string option_c = (cells != row.end ()) ? (*cells++).to_string () : "";
            std::string option_d = (cells != row.end ()) ? (*cells++).to_string () : "";
            std::string correct_options = (cells != row.end ()) ? (*cells++).to_string () : "";

            {
                // Create Question
                shared_ptr<Question> ques = QuizHelper::MakeQuestionFromExcelRow (question_number, question_text,
                                                                                  option_a, option_b, option_c,
                                                                                  option_d, correct_options);

                if (ques) {
                    // Add Question to Question Bank
                    if (!qb.AddQuestionToBank (ques)) {
                        std::cerr << "Failed to add question to bank." << std::endl;
                        return false;
                    }

                } else {
                    std::cerr << "Failed to create question from row." << std::endl;
                    return false;
                }
            }
            // 'ques' is now out of scope, only the QuestionBank holds it
        }

    } catch (const std::exception & e) {

        std::cerr << "Error loading Excel file: " << e.what () << std::endl;

        return false;
    }

    // Mark Question Bank as initialized
    qb.SetQuestionBankInitialized (true);

    return true;
}

