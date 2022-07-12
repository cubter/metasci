/* 
 * Copyright (c) 2022 - present, GitHub: @cubter
 * 
 * See COPYING.txt in the project root for license information.
 */
#ifndef ARTICLE_H
    #define ARTICLE_H
#endif

#include "author.h"
#include "journal.h"

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace metasci
{
using str_vec   = std::vector<std::string>;
using string    = std::string;
using journal   = metasci::journal;
using author    = metasci::author;

// Crossref stores dates as integers, and so do I.
struct date
{
    uint16_t    year;
    uint8_t     month;
    uint8_t     day;
};
using date_vec      = std::vector<date>;
using pub_type_id   = int8_t;
// Publication type. As of 2021, there are 29 publication types.
class publication_type
{
public:
    pub_type_id get_id() const { return id; };
    
    inline bool      operator==(const publication_type &other); 

    publication_type() {};
    publication_type(string crossref_id); 
    publication_type(const publication_type &other) = default; 
    publication_type(publication_type &&other)      = default; 

private:
    string              crossref_id;    // journal-article etc.
    pub_type_id         id;             // my own id
    static pub_type_id  max_id_;
};

publication_type::publication_type(string crossref_id) :
    crossref_id(crossref_id) 
{
    // Each time a new instance of the class is created, the class' max 
    // id is incremeted, and the instance receives an ID.
    id = ++max_id_;
};
inline bool publication_type::operator==(const publication_type &other)
{
    return crossref_id == other.crossref_id;
};

using subject_id = int16_t;
// article's subject (physics etc.).
class subject
{
public:
    subject_id get_id() const { return id; };
    
    inline bool operator==(const subject &other);
    subject     &operator=(const subject &other)    = default; 
    subject     &operator=(subject &&other)         = default;

    subject() {};
    subject(string title);
    subject(const subject &other) = default; 
    subject(subject &&other)      = default; 

private:
    subject_id          id;  // my own id
    static subject_id   max_id_;
    string              title;
};

inline bool subject::operator==(const subject &other)
{
    return title == other.title;
};
subject::subject(string title) :
    title(title)  // titles are short, so no need to `move` them.
{
    // Each time a new instance of the class is created, the class' max 
    // id is incremeted, and the instance receives an ID.
    id = ++max_id_;
};

template<typename T>
using cref_vec = std::vector<std::reference_wrapper<const T>>;

class article
{
public:    
    // Builder's class.
    // I'm aware that it's a non-standard solution to implement
    // a builder class. However, in this case I've decided to stick to it,
    // since I've got a rel. simple inheritance in general and don't really see 
    // the need for the builder's inheritance here.
    class builder
    {
    public:
        string      doi_b;
        string      title_b;         
        pub_type_id type_b;        
        date_vec    published_b;   
        int32_t     score_b;
        date_vec    issued_b;      
        string      volume_b;
        string      issue_b;
        str_vec     ct_numbers_b;   
        int32_t     ref_num_b;
        int32_t     ref_by_num_b;
        str_vec     references_b;
        mutable cref_vec<journal>   journals_b;
        std::vector<subject_id>     subjects_ids_b;
        std::vector<author>         authors_b;

        article build();

        // builder's constructors
        builder();
        builder(string title, 
            string doi, 
            cref_vec<journal> &&journals, 
            std::vector<author> &&authors);
        builder(string title, 
            string doi, 
            const cref_vec<journal> &journals,
            const std::vector<author> &authors);
        ~builder() {};
    };

    string       get_title() const  { return title; };
    string       get_doi() const    { return doi; };
    pub_type_id  get_type() const   { return type; };
    std::vector<author>     get_authors() const         { return authors; };
    std::vector<subject_id> get_subjects_ids() const    { return subjects_ids; };
    inline std::vector<journal> get_journals() const;

    article &operator=(article &&other)      = default;
    article &operator=(const article &other) = delete;
    
    article(builder &b);
    article(article &&other)        = default;
    article(const article &other)   = delete;
    article()                       = delete;
    ~article() {};

private:
    int32_t         id;          // my own id
    static int32_t  max_id_;
    string          doi;         // DOI -- a unique article's ID
    string          title;          
    pub_type_id     type;           
    date_vec        published;   // date of publication (online (pref.)/print)  
    int32_t         score;
    date_vec        issued;      // date of issue
    string          volume;      // volume's number
    string          issue;       // issue's number        
    str_vec         ct_numbers;  // NCT IDs associated with the publication
    int32_t         ref_num;     // number of references
    int32_t         ref_by_num;  // No of times the article has been referenced
    str_vec         references;  // list of references
    // The use of subjects' IDs instead of references to subjects is
    // exclusively due to the reason of space optimization: unlike journals, 
    // there are not so many subjects out there.
    std::vector<subject_id> subjects_ids;
    // article may have several authors. Unfortunately, it's impossible to say
    // whether two authors with the same full name are indeed the same person, 
    // unless the author has ORCID, which is far not always the case. Hence, 
    // a vector with classes instead of references is used here.
    std::vector<author>     authors;
    cref_vec<journal>       journals;  // references to journals
};

// Gets the names of the journals the article has been published in.
inline std::vector<journal> article::get_journals() const 
{  
    std::vector<journal> out;

    for (auto &jcref : this->journals)
    {
        out.emplace_back(jcref.get());
    }

    return out;
};

// Builds an article from builder, calling builder's destructor afterwards. 
article article::builder::build()
{ 
    article a(*this); 
    this->~builder(); 

    return a; 
};
// builder's ctor with vectors as rvalues.
article::builder::builder(string title, 
    string doi, 
    cref_vec<journal> &&journals,
    std::vector<author> &&authors) : 
    doi_b(std::move(doi)), 
    title_b(std::move(title)), 
    journals_b(std::move(journals)),
    authors_b(std::move(authors))
{};
// builder's ctor with vectors passed by reference.
article::builder::builder(string title, 
    string doi, 
    const cref_vec<journal> &journals,
    const std::vector<author> &authors) : 
    doi_b(std::move(doi)), 
    title_b(std::move(title)), 
    journals_b(journals),
    authors_b(authors)
{};
// article's ctor.
article::article(builder &b) : 
    doi(std::move(b.doi_b)), 
    title(std::move(b.title_b)), 
    type(b.type_b), 
    published(std::move(b.published_b)),
    score(b.score_b), 
    issued(std::move(b.issued_b)), 
    volume(b.volume_b), 
    issue(b.issue_b), 
    ct_numbers(std::move(b.ct_numbers_b)), 
    ref_num(b.ref_num_b),
    ref_by_num(b.ref_by_num_b), 
    journals(std::move(b.journals_b)),
    authors(std::move(b.authors_b)),
    subjects_ids(std::move(b.subjects_ids_b)),
    references(std::move(b.references_b))
{ 
    // Each time a new instance of the class is created, the class' max 
    // id is incremeted, and the instance receives a new ID.
    id = ++max_id_;
}
}