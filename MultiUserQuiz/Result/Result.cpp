#include "Result.h"
#include "../Config/QuizConfig.h"


Result::Result (int timeLimit) :totalTimeElapsed (0), 
                                totalTimeLimit (timeLimit),
                                curr_score (0),
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

void Result::UpdateTimeElapsed (int time)
{
    totalTimeElapsed += time;
}

void Result::AddAnswer (Answer && ans)
{
        unsigned int quesId = ans.GetQuestionId ();
        eQuesAttemptStatus newStatus = QuizHelper::ValidateUserAnswer (ans);

    auto it = tracker_map.find (quesId);

    // If already attempted, revert previous score
    if (it != tracker_map.end ()) {

        if (it->second == newStatus) {

            // case: same response selected - nothing to do.
            return;
        }

        switch (it->second) {
            case CORRECT:            curr_score -= correctReward; break;
            case INCORRECT:          curr_score -= incorrectPenalty; break;
            case PARTIALLY_CORRECT:  curr_score -= partialReward; break;
            default: break;
        }
    }

    // Apply new score based on new status
    switch (newStatus) {
        case CORRECT:            curr_score += correctReward; break;
        case INCORRECT:          curr_score += incorrectPenalty; break;
        case PARTIALLY_CORRECT:  curr_score += partialReward; break;
        case UNATTEMPTED:        break;
    }

    // If it's attempted, store the answer
    if (newStatus != UNATTEMPTED) {
        attempt_map[quesId] = std::move (ans);
    }

    // Always record status
    tracker_map[quesId] = newStatus;
}