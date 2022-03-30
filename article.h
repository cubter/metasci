/* 
 * Copyright (c) 2022 - present, GitHub: @cubter
 * 
 * See COPYING.txt in the project root for license information.
 */
#ifndef ARTICLE_H
    #define ARTICLE_H
#endif

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace metasci
{
using str_vec       = std::vector<std::string>;
using string        = std::string;
using pub_type_id   = int8_t;
using subject_id    = int16_t;

// Author. 
class Author
{
public:
    string  inline get_first_name() const { return first_name; }
    string  inline get_family_name() const { return family_name; }
    str_vec inline get_affiliations() const { return affiliations; }
    void    inline add_affiliation(string aff);
    void    inline set_affiliations(str_vec &&aff);
    void    inline set_affiliations(const str_vec &aff);

    Author() {};
    Author(string first_name, string family_name);
    Author(string first_name, 
        string family_name, 
        string orcid, 
        bool is_authenticated_orcid); 
    ~Author() {};

private:
    // id is my own internal ID. It has nothing to do with Crossref and is 
    // assigned automatically every time a new instance of Author is created. 
    // Same applies to the other classes.  
    int32_t         id; 
    // From here on, the max_id_ param. is to track the currently assigned IDs. 
    // Every time an instance of Author is created, max_id_ is incremented.
    static int32_t  max_id_; 
    string          orcid;  // unique author's ID; many authors lack it.
    bool            is_auth_orcid;  // is the ORCID authenticated
    string          first_name;     
    string          family_name;
    str_vec         affiliations;   // list of affiliations
};

// Adds an afiiliation to the list.
void Author::add_affiliation(string aff) 
{ 
    affiliations.emplace_back(std::move(aff)); 
};
// Sets Author's affilitations.
void Author::set_affiliations(str_vec &&aff)
{
    affiliations = std::move(aff);
};
void Author::set_affiliations(const str_vec &aff)
{
    affiliations = aff;
};
Author::Author(string first_name, 
    string family_name) : 
    first_name(std::move(first_name)), 
    family_name(std::move(family_name)) 
{};
Author::Author(string first_name, 
    string family_name, 
    string orcid, 
    bool   is_authenticated_orcid) : 
    first_name(std::move(first_name)), 
    family_name(std::move(family_name)),
    orcid(std::move(orcid)), 
    is_auth_orcid(is_authenticated_orcid) 
{};

// Publisher. Publisher is a parent of journal. Each journal always has 
// exactly one publisher.
class Publisher
{
public:
    string inline get_title() const { return title; };

    Publisher();
    Publisher(string title);
    virtual ~Publisher() {};

private:
    int32_t         id;  // my own id
    static int32_t  max_id_;
    string          title;
};

Publisher::Publisher(string title) : 
    title(std::move(title)) 
{ 
    // Each time a new instance of the class is created, the class' max 
    // id is incremeted, and the instance receives an ID.
    id = ++max_id_; 
};

// A Journal is a child of a Publisher. No Journal can have more than one 
// publisher.
class Journal : public Publisher
{
public:
    string inline get_title() const           { return title; }
    string inline get_publisher_title() const { return Publisher::get_title(); }

    Journal() {};
    Journal(string title, string publisher_title);
    virtual ~Journal() {};
private:
    int32_t         id; // my own id
    static int32_t  max_id_;
    string          title;
};

Journal::Journal(string title, string publisher_title) :
    title(std::move(title)), 
    Publisher(std::move(publisher_title)) 
{ 
    // Each time a new instance of the class is created, the class' max 
    // id is incremeted, and the instance receives an ID.
    id = ++max_id_; 
};
// Hasher & comparator to enable creation of unordered sets.
struct Journal_hasher
{
    size_t operator()(const Journal &j) const noexcept
    {
        return std::hash<string>()(j.get_title());
    }
};
struct Journal_comparator
{
    bool operator()(const Journal &j1, const Journal &j2) const noexcept
    {
        return (j1.get_title() == j2.get_title());
    }
};

// Crossref stores dates as integers, and so do I.
struct Date
{
    uint16_t    year;
    uint8_t     month;
    uint8_t     day;
};

using date_vec = std::vector<Date>;

// Publication type. As of 2021, there are 29 publication types.
class Publication_type
{
public:
    pub_type_id inline get_id() const { return id; };
    bool operator==(const Publication_type &other);
    Publication_type(string crossref_id);

private:
    string              crossref_id;    // journal-article etc.
    pub_type_id         id;             // my own id
    static pub_type_id  max_id_;
};

Publication_type::Publication_type(string crossref_id) :
    crossref_id(crossref_id) 
{
    // Each time a new instance of the class is created, the class' max 
    // id is incremeted, and the instance receives an ID.
    id = ++max_id_;
};
bool Publication_type::operator==(const Publication_type &other)
{
    return crossref_id == other.crossref_id;
};

// Article's subject (physics etc.).
class Subject
{
public:
    subject_id inline get_id() const { return id; };
    bool operator==(const Subject &other);
    Subject(string title);

private:
    subject_id          id; // my own id
    static subject_id   max_id_;
    string              title;
};

bool Subject::operator==(const Subject &other)
{
    return title == other.title;
};
Subject::Subject(string title) :
    title(title)  // titles are short, so no need to `move` them.
{
    // Each time a new instance of the class is created, the class' max 
    // id is incremeted, and the instance receives an ID.
    id = ++max_id_;
};

template<typename T>
using cref_vec = std::vector<std::reference_wrapper<const T>>;

class Article
{
public:
    string inline           get_title() const { return title; };
    string inline           get_doi() const { return doi; };
    pub_type_id inline      get_type() const { return type; };
    std::vector<Author>     inline get_authors() const { return authors; };
    std::vector<subject_id> inline get_subjects_ids() const { return subjects_ids; };
    std::vector<Journal>    inline get_journals() const;

    // I'm aware that it's a non-standard solution to implement
    // a builder class. However, in this case I've decided to stick to it,
    // since I've got a rel. simple inheritance in general and don't really see 
    // the need for the Builder's inheritance here.
    class Builder
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
        mutable cref_vec<Journal>   journals_b;
        std::vector<subject_id>     subjects_ids_b;
        std::vector<Author>         authors_b;

        Article build();

        // Builder's constructors
        Builder();
        Builder(string title, 
            string doi, 
            cref_vec<Journal> &&journals, 
            std::vector<Author> &&authors);
        Builder(string title, 
            string doi, 
            const cref_vec<Journal> &journals,
            const std::vector<Author> &authors);
        ~Builder() {};
    };
    
    Article(Builder &b);
    Article(Article &&other)                    = default;
    Article &operator=(Article &&other)         = default;
    Article(const Article &other)               = delete;
    Article &operator=(const Article &other)    = default;
    Article()                                   = delete;
    ~Article() {};

private:
    int32_t         id;         // my own id
    static int32_t  max_id_;
    string          doi;        // DOI -- a unique article's ID
    string          title;          
    pub_type_id     type;           
    date_vec        published;  // date of either online (pref.) or print publication 
    int32_t         score;
    date_vec        issued;     // date of issue
    string          volume;     // volume's number
    string          issue;      // issue's number        
    str_vec         ct_numbers; // NCT IDs associated with the publication
    int32_t         ref_num;    // number of references
    int32_t         ref_by_num; // num. of times the article has been referenced
    str_vec         references; // list of references
    // I've decided to use Subjects' IDs instead of references to Subjects
    // exclusively for the reason of space optimization: unlike Journals, there 
    // are not so many subjects out there.
    std::vector<subject_id>    subjects_ids;
    // Article may have several authors. Unfortunately, it's impossible to say
    // whether the authors with the same full name are indeed the same person, 
    // unless the author has ORCID, which is not always the case. Hence, I'm 
    // using a simple vector with classes instead of references here.
    std::vector<Author>     authors;
    cref_vec<Journal>       journals; 
};

// Gets a lost of journals the article has been published in.
std::vector<Journal> Article::get_journals() const 
{  
    std::vector<Journal> jret;

    for (auto &jcref : this->journals)
    {
        jret.emplace_back(jcref.get());
    }

    return jret;
};
// Builds an article from builder, calling builder's destructor afterwards. 
Article Article::Builder::build()
{ 
    Article a(*this); 
    this->~Builder(); 
    
    return a; 
};
// Builder's ctor with vectors as rvalues.
Article::Builder::Builder(string title, 
    string doi, 
    cref_vec<Journal> &&journals,
    std::vector<Author> &&authors) : 
    doi_b(std::move(doi)), 
    title_b(std::move(title)), 
    journals_b(std::move(journals)),
    authors_b(std::move(authors))
{};
// Builder's ctor with vectors passed by reference.
Article::Builder::Builder(string title, 
    string doi, 
    const cref_vec<Journal> &journals,
    const std::vector<Author> &authors) : 
    doi_b(std::move(doi)), 
    title_b(std::move(title)), 
    journals_b(journals),
    authors_b(authors)
{};
// Article's ctor.
Article::Article(Builder &b) : 
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