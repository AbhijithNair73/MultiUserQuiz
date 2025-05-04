#include "QuizDefs.h"

// Global variables


// TODO check how to use namespace correctly
namespace QuizHelper
{
    eQuesAttemptStatus ValidateUserAnswer (const Answer & ans)
    {
            QuestionBank & qb                       = QuestionBank::GetInstance ();
            std::unordered_set<int> correct_opts    = qb.GetCorrectOptionsById (ans.GetQuestionId ());
            std::array<bool, 4> selected_ans        = ans.GetSelectedOp ();
            bool has_correct                        = false;
            bool has_incorrect                      = false;

        if (selected_ans.empty ()) {
            return UNATTEMPTED;  // No answer selected
        }

        for (int option_idx : correct_opts) {

            if (selected_ans[option_idx]) {
                has_correct = true;
            } else {
                has_incorrect = true;
            }
        }

        if (has_correct && has_incorrect) return PARTIALLY_CORRECT;
        if (has_correct) return CORRECT;
        return INCORRECT;
    }

    std::shared_ptr<Question> MakeQuestionFromExcelRow (const std::string & question_number_str,
                                                        const std::string & question_text,
                                                        const std::string & option_a,
                                                        const std::string & option_b,
                                                        const std::string & option_c,
                                                        const std::string & option_d,
                                                        const std::string & correct_options_str)
    {
        // Validate mandatory fields
        if (question_text.empty () ||
            option_a.empty () || option_b.empty () || option_c.empty () || option_d.empty () ||
            correct_options_str.empty ()) {

            // TODO: add log here
            return nullptr; // Mandatory fields missing
        }

        // Parse question number
        unsigned int question_id = std::stoul (question_number_str);

        // Options
        std::vector<std::string> options = {option_a, option_b, option_c, option_d};

        // Parse correct options - supports both "A,C" and "1,3"
        std::unordered_set<int> correct_options;
        std::istringstream iss (correct_options_str);
        std::string token;

        while (std::getline (iss, token, ',')) {

            if (token.empty ()) continue;

            // Remove whitespace
            token.erase (remove_if (token.begin (), token.end (), ::isspace), token.end ());

            int index = -1;
            // Accept ABCD or 1234
            if (std::isdigit (token[0])) {
                index = std::stoi (token) - 1;
            } else if (std::isalpha (token[0])) {
                char ch = std::toupper (token[0]);
                if (ch >= 'A' && ch <= 'D') {
                    index = ch - 'A';
                }
            }

            // Check index range
            if (index < 0 || index > 3) {
                return nullptr; // Invalid correct option
            }
            correct_options.insert (index);
        }

        return std::make_shared<Question> (question_id, question_text, options, correct_options);
    }

}
