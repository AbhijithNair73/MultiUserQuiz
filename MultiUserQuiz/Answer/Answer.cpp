#include "Answer.h"

Answer::Answer (unsigned int pQId): quesId(pQId)
{
    // nothing to do
}

Answer::Answer ()
{
    quesId = 0;
}

Answer::~Answer ()
{
    // nothing to do.
}

bool Answer::SetSelectedOp (int optIndex)
{
    if (optIndex < 0 || optIndex >= 4) {
        // selected wrong index to set. !!! something wrong
        return false;
    }

    selectedOp[optIndex] = true;
    return true;
}

void Answer::SetQuestionId (unsigned int pId)
{
    quesId = pId;
}

array<bool, 4> Answer::GetSelectedOp () const
{
    return selectedOp;
}

unsigned int Answer::GetQuestionId () const
{
    return quesId;
}