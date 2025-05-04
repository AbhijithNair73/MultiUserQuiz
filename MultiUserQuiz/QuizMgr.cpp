#include "QuizMgr.h"

QuizMgr::QuizMgr ()
{
    // Do nothing
}

QuizMgr::~QuizMgr ()
{
    // Do nothing
}

bool QuizMgr::StartQuiz (const std::string & pFileName)
{
    return InitializeQuestionBank (pFileName);
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
                std::cout << std::endl << "===============" << std::endl;
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
                    ques->DisplayQuestion ();
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

    return true;
}

