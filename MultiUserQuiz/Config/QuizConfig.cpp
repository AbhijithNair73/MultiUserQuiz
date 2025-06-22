#include "QuizConfig.h"
#include <algorithm>
#include <string>
#include <ini/ini.h>

const char * CONFIG_FILE_NAME = "quiz_config.ini";

namespace {

    // Helper to normalize boolean strings
    bool ParseBool (const std::string & value, bool & outBool)
    {
        std::string val = value;
        std::transform (val.begin (), val.end (), val.begin (), ::tolower);

        if (val == "true" || val == "1" || val == "yes") {
            outBool = true;
            return true;
        } else if (val == "false" || val == "0" || val == "no") {
            outBool = false;
            return true;
        }
        return false;
    }

    // Helper to parse numeric values
    template <typename T>
    bool ParseNumber (const std::string & value, T & outVal)
    {
        try {
            if constexpr (std::is_same_v<T, int>)
                outVal = std::stoi (value);
            else if constexpr (std::is_same_v<T, long long>)
                outVal = std::stoll (value);
            else if constexpr (std::is_same_v<T, double>)
                outVal = std::stod (value);
            else
                return false;
            return true;
        } catch (...) {
            return false;
        }
    }

    // Helper to parse quiz mode
    bool ParseQuizMode (const std::string & modeStr, eQuizMode & mode)
    {
        if (modeStr == "BULLET_TIMER_MODE") {
            mode = BULLET_TIMER_MODE;
        } else if (modeStr == "TIME_BOUND_MODE") {
            mode = TIME_BOUND_MODE;
        } else if (modeStr == "STRICT_TIME_BOUND_MODE") {
            mode = STRICT_TIME_BOUND_MODE;
        } else {
            return false;
        }
        return true;
    }

} // anonymous namespace

// --- Main Parser Function ---
int QuizConfig::Parser (void * user, const char * section, const char * name, const char * value)
{
    QuizConfig * self = static_cast<QuizConfig *>(user);

    if (strcmp (section, "Quiz") != 0) {
        std::cerr << "Unknown section: " << section << "\n";
        return 0;
    }

    std::string key (name);
    std::string valStr (value);

    if (key == "CorrectScore") {
        if (!ParseNumber (valStr, self->posMarking)) {
            std::cerr << "Invalid CorrectScore value: " << valStr << "\n";
            return 0;
        }

    } else if (key == "IncorrectPenalty") {
        if (!ParseNumber (valStr, self->negMarking)) {
            std::cerr << "Invalid IncorrectPenalty value: " << valStr << "\n";
            return 0;
        }
        self->vIsNegMarking = true;

    } else if (key == "PartialScore") {
        if (!ParseNumber (valStr, self->paritalMarking)) {
            std::cerr << "Invalid PartialScore value: " << valStr << "\n";
            return 0;
        }

    } else if (key == "QuizMode") {
        if (!ParseQuizMode (valStr, self->vQuizMode)) {
            std::cerr << "Invalid QuizMode: " << valStr << "\n";
            return 0;
        }

    } else if (key == "TimeAllowed") {
        long long timeSecs;
        if (!ParseNumber (valStr, timeSecs) || timeSecs <= 0 || timeSecs > 43200) {
            std::cerr << "Invalid TimeAllowed. Must be > 0 and <= 43200 seconds. Got: " << valStr << "\n";
            return 0;
        }

        if (self->vQuizMode == BULLET_TIMER_MODE) {
            self->vTimeAllowed.vTimePerQues = timeSecs;
        } else {
            self->vTimeAllowed.vTotalQuizTime = timeSecs;
        }

    } else if (key == "IsSingleUser") {
        if (!ParseBool (valStr, self->vIsSingleUser)) {
            std::cerr << "Invalid IsSingleUser value: " << valStr << "\n";
            return 0;
        }

    } else if (key == "IsMultiOptionSelect") {
        if (!ParseBool (valStr, self->vIsMultiOptionSelect)) {
            std::cerr << "Invalid IsMultiOptionSelect value: " << valStr << "\n";
            return 0;
        }

        if (self->vIsMultiOptionSelect) {
            self->paritalMarking = 0.5 * self->posMarking;
        }

    } else if (key == "IsKBCMode") {
        if (self->vQuizMode != BULLET_TIMER_MODE) {
            std::cerr << "KBC Mode is only allowed in BULLET_TIMER_MODE.\n";
            return 0;
        }

        if (!ParseBool (valStr, self->vIsKBCMode)) {
            std::cerr << "Invalid IsKBCMode value: " << valStr << "\n";
            return 0;
        }

    } else {
        std::cerr << "Unknown key: " << name << " in section: " << section << "\n";
        return 0;
    }

    return 1; // Success
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

long long QuizConfig::GetTimeAllowedBasedOnQuizMode () const
{
    return (vQuizMode == BULLET_TIMER_MODE) ? vTimeAllowed.vTimePerQues : vTimeAllowed.vTotalQuizTime;
}

bool QuizConfig::IsSingleUser () const
{
    return vIsSingleUser;
}

bool QuizConfig::IsKBCMode () const
{
    return vIsKBCMode;
}

bool QuizConfig::IsMultiOptionSelect () const
{
    return vIsMultiOptionSelect;
}

eQuizMode QuizConfig::GetQuizMode () const
{
    return vQuizMode;
}