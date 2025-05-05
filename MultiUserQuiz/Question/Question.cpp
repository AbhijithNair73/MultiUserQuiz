#include "Question.h"

Question::Question (unsigned int questionID, string questionText, vector<string> options, unordered_set<int> correctOptions)
{
    this->exclquesID = questionID;
    this->questionText = questionText;
    this->quesOptions = options;
    this->correctOptions = correctOptions;
}

Question::~Question ()
{

}

unsigned int Question::GetQuestionID () const
{
    return questionID;
}

string Question::GetQuestionText () const
{
    return questionText;
}

vector<string> Question::GetQuestionOptions () const
{
    return quesOptions;
}

unordered_set<int> Question::GetCorrectOptions () const
{
    // TODO: debug check for empty correct option for a question
#ifdef DEBUG
    if (correctOptions.empty ()) 
    {
        // something is wrong with this question
    }
#endif

    return correctOptions;
}

void Question::SetQuestionID (unsigned int questionID)
{
    this->questionID = questionID;
}

void Question::SetQuestionText (string & questionText)
{
    this->questionText = questionText;
}

void Question::SetQuestionOptions (vector<string> options)
{
    // TODO make sure it is moved instead of copied.
    this->quesOptions = options;
}

void Question::SetCorrectOptions (unordered_set<int> correctOptions)
{
    this->correctOptions = correctOptions;
}

void Question::AddQuestionOption (string option)
{
    if (quesOptions.size () > 4) {

        // log here - TODO: add your logger class and add here.
    }
    quesOptions.push_back (option);
}

void Question::DisplayQuestion () const
{
    std::cout << "\nQuestion " << questionID << ": " << questionText << std::endl;
    std::cout << "A) " << quesOptions[0] << std::endl;
    std::cout << "B) " << quesOptions[1] << std::endl;
    std::cout << "C) " << quesOptions[2] << std::endl;
    std::cout << "D) " << quesOptions[3] << std::endl;
}

void Question::DisplayCorrectAnswers () const
{
    std::cout << "Correct Option(s): ";
    for (int i : correctOptions) {
        std::cout << static_cast<char>('A' + i) << " ";
    }
    std::cout << std::endl;
}

bool Question::IsCorrect (string option) const
{
    for (int i = 0; i < quesOptions.size (); i++) {

        if (quesOptions[i] == option) {

            return IsCorrect (i);
        }
    }
    // option is not part of the question , some bug
    // TODO - add log and debug break.
    return false;
}

/*
*  Method to check the answer based on the index selected.
* @param:   index - index of the quesoption
* 
* Checks the question option index is part of the correct options. This can be used to check
* if single answer and not multioption answer.
* 
*/
bool Question::IsCorrect (int index) const
{
    return correctOptions.find (index) != correctOptions.end ();
}
