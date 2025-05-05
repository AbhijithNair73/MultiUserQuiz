#include "QuestionBank.h"

QuestionBank::QuestionBank (): vIsInitialized(false)
{
    // nothing
}

QuestionBank::~QuestionBank ()
{
    ResetQuestionBank ();
}

QuestionBank & QuestionBank::GetInstance ()
{
    static QuestionBank instance;
    return instance;
}

bool QuestionBank::AddQuestionToBank (std::shared_ptr<Question> ques)
{
        std::unique_lock        ulock (mtx);
        unsigned int            newId;

    // Generate a new Question ID
    newId = static_cast<unsigned int>(quesmap.size ()) + 1;

    ques->SetQuestionID (newId);

    auto [it, inserted] = quesmap.emplace (newId, std::move (ques));

    return inserted;  // true if inserted successfully, false if an issue occurred
}

bool QuestionBank::UpdateQuestionById (unsigned int id, std::shared_ptr<Question> ques)
{
        std::unique_lock    ulock (mtx);
        quesmap_itr         itr;

    if (quesmap.empty ()) {
        // Question Bank is empty
        return false;
    }

    itr = quesmap.find (id);

    // the question is found and nobody else is referencing this question except the QuestionBank itself
    if (itr != quesmap.end () && itr->second.use_count () == 1) {

        ques->SetQuestionID (id);

        itr->second = std::move (ques);

        //itr->second = ques; // update the question in the map
        return true;
    }

    // question id not present to alter.
    return false;
}

bool QuestionBank::RemoveQuestionById (unsigned int id)
{
        std::unique_lock    ulock (mtx);
        quesmap_citr        itr;

    if (quesmap.empty ()) {
        // Question Bank is empty
        return false;
    }

    itr = quesmap.find (id);

    // the question is found and nobody else is referencing this question except the QuestionBank itself
    if (itr != quesmap.end () && itr->second.use_count () == 1) {

        quesmap.erase (itr);
        return true;
    }

    return false;
}

std::shared_ptr<const Question> QuestionBank::GetQuestionById (unsigned int id)
{
        std::shared_lock    lck (mtx);
        quesmap_citr        itr;

    if (quesmap.empty ()) {
        // Question Bank is empty
        return nullptr;
    }
    itr = quesmap.find (id);

    if (itr != quesmap.end ()) {

        std::shared_ptr<Question> sp = itr->second;
        if (sp == nullptr) {
            // the stored question has been deleted.
            return nullptr;
        }

        // adding constness to the shared pointer - so that the caller cannot modify the question object.
        return std::const_pointer_cast<const Question>(sp);
    }
    return nullptr;
}

unordered_set<int> QuestionBank::GetCorrectOptionsById (unsigned int id)
{
        std::shared_lock    lck (mtx);
        quesmap_citr        itr;

    if (quesmap.empty ()) {
        // Question Bank is empty
        goto err_end;
    }

     itr = quesmap.find (id);

    if (itr != quesmap.end()) {

        std::shared_ptr<Question> sp = itr->second;
        if (sp == nullptr) {
            // the stored question has been deleted.
            goto err_end;
        }
        return sp->GetCorrectOptions ();
    }

err_end: 
    return unordered_set<int> ();
}

unsigned int QuestionBank::TotalQuestionCount () const
{
    return static_cast<unsigned int>(quesmap.size());
}

void QuestionBank::ResetQuestionBank ()
{
        std::unique_lock    ulock (mtx);

    // empties the map and should reduce the reference count hence leading to destruction of questions.
    quesmap.clear ();
}

bool QuestionBank::IsQuestionBankEmpty () const
{
    return quesmap.empty();
}

bool QuestionBank::IsQuestionBankInitialized () const
{
    return vIsInitialized;
}

void QuestionBank::SetQuestionBankInitialized (bool pIsInitialized)
{
    vIsInitialized = pIsInitialized;
}
