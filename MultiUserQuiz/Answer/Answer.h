#pragma once
#include <iostream>
#include <array>

using namespace std;

/*
* This class stores the option selected by the user
* also stores the quiestion id against which the user 
* has selected this option.
* 
* @Note: This object should be light-weight as every user
* will have the copy of the answer object selected for each attempted question by that user.
* And similarly for each user the attempted answer will be stored in memory.
* 
* option selected should be all false for non-attempted questions OR no answer object at all.
* 
* we have choosen array<bool, 4> for option selected because the question object will have correct options
* which is an unordered_set and will contain the option selected and is 0 based index.
* so based on correct option index, lets say 0 & 3(i.e option A & D) is the answer then we need to check
* against that index of opt_selected. e.g: opt_selected[0] && opt_selected[3] both should be true for a correct response.
* 
* For unattempted answers the answer object will be created but SetSelectedOps will not be called and hence the selectedop will be empty.
*/
class Answer {

    std::array<bool, 4> selectedOp {};
    unsigned int        quesId;                     // stores the question Id for which option is selected

public:
                    Answer          (unsigned int pQId);
                    // default construction is intentionally deleted as we dont want this object to be constructed without a relevant question id.
                    Answer          ();
                    ~Answer         ();

    bool            SetSelectedOp   (int optIndex);

    array<bool, 4>  GetSelectedOp   () const;
    unsigned int    GetQuestionId   () const;

    void            SetQuestionId   (unsigned int pId);

};