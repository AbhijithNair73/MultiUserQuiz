#include "Result.h"
#include "../Config/QuizConfig.h"


Result::Result () : vTotalTimeElapsed (0), 
                    vTotalTimeLimit (0),
                    vCurrScore (0),
                    correctReward       (QuizConfig::GetInstance().GetCorrectAnsReward()),
                    incorrectPenalty    (QuizConfig::GetInstance ().GetIncorrectAnsPenalty()),
                    partialReward       (QuizConfig::GetInstance ().GetPartialAnsReward ())
{ 
    // nothing
}

Result::~Result ()
{
    // nothing to do.
}

// This is only used for timebound mode
void Result::SetTimeLimit (unsigned int timeLimit)
{
    vTotalTimeLimit = timeLimit;
}

void Result::UpdateTimeElapsed (unsigned int pTimeElapsedMs)
{
    vTotalTimeElapsed = pTimeElapsedMs;
}

unsigned int Result::GetTimeElapsedInQuiz () const
{
    return vTotalTimeElapsed;
}


eQuesAttemptStatus Result::AddAnswer (Answer & ans)
{
        unsigned int quesId = ans.GetQuestionId ();
        eQuesAttemptStatus newStatus = QuizHelper::ValidateUserAnswer (ans);

    auto it = tracker_map.find (quesId);

    // If already attempted, revert previous score
    if (it != tracker_map.end ()) {

        if (it->second == newStatus) {

            // case: same response selected - nothing to do.
            return newStatus;
        }

        switch (it->second) {
            case CORRECT:            vCurrScore -= correctReward; break;
            case INCORRECT:          vCurrScore -= incorrectPenalty; break;
            case PARTIALLY_CORRECT:  vCurrScore -= partialReward; break;
            default: break;
        }
    }

    // Apply new score based on new status
    switch (newStatus) {
        case CORRECT:            vCurrScore += correctReward; break;
        case INCORRECT:          vCurrScore += incorrectPenalty; break;
        case PARTIALLY_CORRECT:  vCurrScore += partialReward; break;
        case UNATTEMPTED:        break;
    }

    // If it's attempted, store the answer
    if (newStatus != UNATTEMPTED) {
        attempt_map[quesId] = std::move (ans);
    }

    // Always record status
    tracker_map[quesId] = newStatus;

    return newStatus;
}

double Result::GetCurrentScore () const
{
    return vCurrScore;
}

void Result::PrintFinalResult (bool pShowDetailResult) const
{
    std::cout << "\n======= Quiz Summary =======\n";
    std::cout << "Final Score: " << vCurrScore << "\n";

    if (vTotalTimeLimit > 0) {

        std::cout << "Time Taken: " << vTotalTimeElapsed / 1000.0 << " seconds";
        std::cout << " / Time Limit: " << vTotalTimeLimit / 1000.0 << " seconds";
    }

    std::cout << "\n-----------------------------\n";

    if (pShowDetailResult) {

        QuestionBank & qb = QuestionBank::GetInstance ();
        bool hasIncorrect = false;

        for (const auto & [quesId, status] : tracker_map) {

            if (status == CORRECT) {
                // Only print Incorrect answers
                continue;
            }

            shared_ptr<const Question> question = qb.GetQuestionById (quesId);
            if (!question) {
                std::cerr << "[ERROR] Could not retrieve question ID: " << quesId << "\n";
                continue;
            }

            std::cout << "\nQ" << quesId << ": ";
            question->DisplayQuestion ();

            switch (status) {
                case INCORRECT:
                    std::cout << "Status: Incorrect\n";
                    break;
                case PARTIALLY_CORRECT:
                    std::cout << "Status: Partially Correct\n";
                    break;
                case UNATTEMPTED:
                    std::cout << "Status: Unattempted\n";
                    break;
                default:
                    break;
            }

            std::cout << "Correct Answer(s):\n";
            question->DisplayCorrectAnswers ();
            hasIncorrect = true;
        }

        if (!hasIncorrect)
            std::cout << "\nCongratulations! All questions answered correctly.\n";

        std::cout << "\n=============================\n";
    }

}
