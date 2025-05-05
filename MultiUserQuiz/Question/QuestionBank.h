#pragma once
#include <unordered_map>
#include <shared_mutex>
#include "Question.h"

using quesmap_citr = std::unordered_map<unsigned int, std::shared_ptr<Question>>::const_iterator;
using quesmap_itr = std::unordered_map<unsigned int, std::shared_ptr<Question>>::iterator;

/**
* QuestionBank will be a singleton class holding all the questions, and 
* it will be the single point of contact for all the questions.
* Server and client amy have its own question bank object.
* It will be initialized/instantiated at startup.
* 
* This class will hold all the questions and will be object and it will return reference or pointer 
* of the Question object when requested.
* 
* Question bank will update the question id of the question object as per the map count or number of items added till now,
* this also means the question object will come with empty question id to QuestionBank.
* 
*/
class QuestionBank {
public:
    static  QuestionBank &                      GetInstance         ();

            // the actual object should be moved or should it be pointer, or should we allocate question object dynamically and let the quesmap hold the pointer?
            bool                                AddQuestionToBank           (std::shared_ptr<Question> ques);
            bool                                UpdateQuestionById          (unsigned int id, std::shared_ptr<Question> ques);
            bool                                RemoveQuestionById          (unsigned int id);

            std::shared_ptr<const Question>     GetQuestionById             (unsigned int id);

            unordered_set<int>                  GetCorrectOptionsById       (unsigned int id);

            unsigned int                        TotalQuestionCount          () const;
            void                                ResetQuestionBank           ();

            bool                                IsQuestionBankEmpty         () const;
            bool                                IsQuestionBankInitialized   () const;
            void                                SetQuestionBankInitialized  (bool pIsInitialized);
private:
                                                QuestionBank                ();
                                                ~QuestionBank               ();

                                                QuestionBank                (const QuestionBank &) = delete;
            QuestionBank &                      operator =                  (const QuestionBank &)  = delete;

            // we have chosen shared_ptr as we wanted to return the question outside this class using GetQuestionById function.
            // where now it is returning shared_ptr outside, but after adding constness. This will ensure the object is not updated outside the class.
            // But on using shared_ptr we have the flexibility on the life of shared_ptr(question) - because it has reference count internally.
            // so we will update or remove this question only if its reference count is 1, which means this question is no where else reffered outside this class.
            // and lets say when question bank is destroyed, we will not delete the question object, as it will be deleted by the shared_ptr destructor. 
            // So if any of the question is referred outside and question bank gets destroyed, the externally referred question will be not be deleted, but the reference 
            // count will be decremented and when finally the external reference goes out of scope or is deleted, then the 
            // shared ptr ref count will go to zero and the question will be deleted.
            unordered_map <unsigned int, std::shared_ptr<Question>>  quesmap;                   //< Holds all the questions for the quiz
            mutable shared_mutex                mtx;
            bool                                vIsInitialized;                                 //< Flag to indicate if the question bank is initialized or not
};