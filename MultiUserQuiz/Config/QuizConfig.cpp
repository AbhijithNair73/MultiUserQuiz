#include "QuizConfig.h"
#include <algorithm>
#include <string>
#include "../External/ini/ini.h"

const char * CONFIG_FILE_NAME = "quiz_config.ini";

int QuizConfig::Parser (void * user, const char * section, const char * name, const char * value)
{
    QuizConfig * self = static_cast<QuizConfig *>(user);
    std::string boolValStr;

    if (strcmp (section, "Quiz") == 0) {

        if (strcmp (name, "CorrectScore") == 0) {

            self->posMarking = std::stod (value);

        } else if (strcmp (name, "IncorrectPenalty") == 0) {

            self->negMarking = std::stod (value);
            self->vIsNegMarking = true;

        } else if (strcmp (name, "PartialScore") == 0) {

            self->paritalMarking = std::stod (value);

        } else if (strcmp (name, "QuizMode") == 0) {

            // TODO quizMode
        } else if (strcmp (name, "TimeAllowed") == 0) {

            // TODO Time limit based on the quiz Mode

        } else if (strcmp (name, "IsSingleUser") == 0) {

            boolValStr = value;
            std::transform (boolValStr.begin (), boolValStr.end (), boolValStr.begin (), ::tolower); // normalize

            if (boolValStr == "true" || boolValStr == "1" || boolValStr == "yes") {
                self->vIsSingleUser = true;
            } else if (boolValStr == "false" || boolValStr == "0" || boolValStr == "no") {
                self->vIsSingleUser = false;
            }

        } else if (strcmp (name, "IsMultiOptionSelect") == 0) {

            boolValStr = value;
            std::transform (boolValStr.begin (), boolValStr.end (), boolValStr.begin (), ::tolower); // normalize

            if (boolValStr == "true" || boolValStr == "1" || boolValStr == "yes") {
                self->vIsMultiOptionSelect = true;
            } else if (boolValStr == "false" || boolValStr == "0" || boolValStr == "no") {
                self->vIsMultiOptionSelect = false;
            }

        } else if (strcmp (name, "IsKBCMode") == 0) {

            // Restrict KBCMode with TimerBound Quiz only allowed in Bullet mode.
            self->vIsKBCMode = (strcmp (value, "true") == 0);

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
    vQuizMode = BULLET_TIMER_MODE;
    vIsSingleUser = true;
    vIsMultiOptionSelect = false;

    vIsNegMarking = false;
    negMarking = 0;
    posMarking = paritalMarking = 1;

    vTimeAllowed.vTimePerQues = 15 * 1000;          // 15 seconds
    vIsKBCMode = false;
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
    return (vIsNegMarking ? negMarking : 0);
}

double QuizConfig::GetPartialAnsReward () const
{
    return paritalMarking;
}

unsigned int QuizConfig::GetTimeAllowedBasedOnQuizMode () const
{
    return (vQuizMode == BULLET_TIMER_MODE) ? vTimeAllowed.vTimePerQues : vTimeAllowed.vTotalQuizTime;
}

bool QuizConfig::IsSingleUser () const
{
    return vIsSingleUser;
}

bool QuizConfig::IsMultiOptionSelect () const
{
    return vIsMultiOptionSelect;
}

eQuizMode QuizConfig::GetQuizMode () const
{
    return vQuizMode;
}