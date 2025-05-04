#include "QuizConfig.h"
#include <string>
#include "../External/ini/ini.h"

const char * CONFIG_FILE_NAME = "quiz_config.ini";

int QuizConfig::Parser (void * user, const char * section, const char * name, const char * value)
{
    QuizConfig * self = static_cast<QuizConfig *>(user);

    if (strcmp (section, "Quiz") == 0) {

        if (strcmp (name, "CorrectScore") == 0) {

            self->posMarking = std::stod (value);
        } else if (strcmp (name, "IncorrectPenalty") == 0) {

            self->negMarking = std::stod (value);
        } else if (strcmp (name, "PartialScore") == 0) {

            self->paritalMarking = std::stod (value);
        } else {

            std::cerr << "Unknown key: " << name << " in section: " << section << "\n";
            return 0;  // Stop parsing if unknown key
        }

    } else {

        std::cerr << "Unknown section: " << section << "\n";
        return 0;  // Stop parsing if unknown section
    }
    return 1;  // Success, continue
}

QuizConfig & QuizConfig::GetInstance ()
{
    static QuizConfig instance;
    return instance;
}

QuizConfig::QuizConfig ()
{
    // load default values
    quizMode = BULLET_TIMER_MODE;
    isSingleUser = true;
    isMultiOptionSelect = false;

    isNegMarking = false;
    negMarking = 0;
    posMarking = paritalMarking = 1;

    timeAllowed.timePerQues = 15;
    isKBCMode = false;
    displayOnlyScore = false;
}

QuizConfig::~QuizConfig ()
{
    // nothing to do
}

bool QuizConfig::LoadConfigFromFile ()
{
    if (ini_parse (CONFIG_FILE_NAME, Parser, this) < 0) {

        std::cerr << "Error loading config file: " << CONFIG_FILE_NAME << std::endl;
        return false;
    }

    return true;
}

bool QuizConfig::RefreshConfig ()
{
    return true;
}

double QuizConfig::GetCorrectAnsReward () const
{
    return posMarking;
}

double QuizConfig::GetIncorrectAnsPenalty () const
{
    return (isNegMarking ? negMarking : 0);
}

double QuizConfig::GetPartialAnsReward () const
{
    return paritalMarking;
}