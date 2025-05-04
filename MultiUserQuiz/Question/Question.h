#pragma once
#include <iostream>
#include <unordered_set>
#include <vector>
#include <string>
using namespace std;

class Question {

public:
                        // Constructor and Destructor
                        Question            () = default;
                        Question            (const Question &) = default;
                        Question            (unsigned int questionID, string questionText, vector<string> options, unordered_set<int> correctOptions);
                        ~Question           ();

                        // Getters
    unsigned int        GetQuestionID       ();
    string              GetQuestionText     ();
    vector<string>      GetQuestionOptions  ();
    unordered_set<int>  GetCorrectOptions   ();

                        // Setters  
    void                SetQuestionID       (unsigned int questionID);
    void                SetQuestionText     (string & questionText);
    void                SetQuestionOptions  (vector<string> options);
    void                SetCorrectOptions   (unordered_set<int> correctOptions);

    void                AddQuestionOption   (string option);

    bool                IsCorrect           (string option);
    bool                IsCorrect           (int index);

                        // Diplay methods
    void                DisplayQuestion         ();
    void                DisplayCorrectAnswers   ();

private:

    unsigned int        questionID;             // Unique ID for the question and also serves as question number.
    unsigned int        exclquesID;             // Question number in excel.
    string              questionText;

    //TODO: Should option be made set? so that only unique options are allowed. But this should lead to parsing failure.
    // We will keep it as vector only as 4 elements are only there and our use case would be to access at index at max, so should be fine.
    // As far as detecting duplicate options while parsing , we can handle it while parsing only.

    vector<string>      quesOptions;            // stores the options A,B,C,D string
    unordered_set<int>  correctOptions;         // stores the index of quesOptions index - which are correct answers - this will be zero based index. 
                                                // lets say if A & C are correct answers then it will store 0 and 2 in the set (correctOptions).
};