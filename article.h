/* 
 * Copyright (c) 2022 - present, GitHub: @cubter
 * 
 * See COPYING.txt in the project root for license information.
 */

// TODO: passing params as references instead of moving them?

#ifndef ARTICLE_H
    #define ARTICLE_H
#endif

#include <iostream>
#include <string>
#include <vector>
#include <functional>

using str_vec   = std::vector<std::string>;
using string    = std::string;

namespace metasci
{
class Author
{
    int32_t         id_;
    // From here, the max_id_ param. is required to track the 
    // currently assigned IDs. Every time an instance of Author is 
    // created, max_id_ is incremented.
    static int32_t  max_id_; 
    string          orcid;
    bool            is_auth_orcid;
    string          first_name;
    string          family_name;
    str_vec         affiliations;

public:
    string  get_first_name() const { return first_name; }
    string  get_family_name() const { return family_name; }
    str_vec get_affiliations() const { return affiliations; }
    void    add_affiliation(string &&aff) { affiliations.push_back(std::move(aff)); }

    Author() { };
    Author(string &&first_name, string &&family_name);
    Author(string &&first_name, 
        string &&family_name, 
        string &&orcid, 
        bool is_authenticated_orcid); 
    ~Author() { };
};

Author::Author(string &&first_name, 
    string &&family_name): 
    first_name(std::move(first_name)), 
    family_name(std::move(family_name)) 
{ };

Author::Author(
    string &&first_name, 
    string &&family_name, 
    string &&orcid, 
    bool   is_authenticated_orcid): 
    first_name(std::move(first_name)), 
    family_name(std::move(family_name)),
    orcid(std::move(orcid)),
    is_auth_orcid(is_authenticated_orcid) 
{ };

class Publisher
{
private:
    int32_t         id_;
    static int32_t  max_id_;
    string          title;
public:
    string  get_title() const { return title; };

    Publisher();
    Publisher(string &&title);
    virtual ~Publisher() { };
};

Publisher::Publisher(string &&title): 
    title(std::move(title)) 
{ 
    id_ = ++max_id_; 
};

class Journal: public Publisher
{
private:
    int32_t         id_;
    static  int32_t max_id_;
    string          title;
public:
    string get_title() const            { return title; }
    string get_publisher_title() const  { return(Publisher::get_title()); }

    Journal() { };
    Journal(string &&title, string &&publisher_title);

    virtual ~Journal() { };
};

Journal::Journal(string &&title, string &&publisher_title):
    title(std::move(title)), 
    Publisher(std::move(publisher_title)) 
{ 
    id_ = ++max_id_; 
};

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

struct Date
{
    int32_t year;
    int32_t month;
    int32_t day;
};

using date_vec = std::vector<Date>;

class Subject
{
private:
    string          title;
    int32_t         id_;
    static int32_t  max_id_;
    
public:
    string get_title() const { return title; }

    Subject() {}; 
    Subject(string title); 
    ~Subject() {};
};

Subject::Subject(string title): 
    title(title)
{ 
    id_ = ++max_id_; 
}; 

struct Subject_hasher
{
    size_t operator()(const Subject &s) const noexcept
    {
        return std::hash<string>()(s.get_title());
    }
};
struct Subject_comparator
{
    bool operator()(const Subject &s1, const Subject &s2) const noexcept
    {
        return (s1.get_title() == s2.get_title());
    }
};

template<typename T>
using cref_vec = std::vector<std::reference_wrapper<const T>>;

class Article
{
private:
    int32_t         id_;
    static int32_t  max_id_;
    string          doi;
    string          title;          // article's title
    string          type;           // journal article etc. 
    date_vec        published;      // either online (pref.) or print
    double          score;
    date_vec        issued;         // date of issue
    string          volume;
    string          issue;  
    // It doesn't make sense to make NCT IDs std::ref, since almost 
    // never two articles refer to the same ID.        
    str_vec         ct_numbers; 
    int32_t         ref_num;
    int32_t         ref_by_num; 
    std::vector<Author>         authors;
    mutable cref_vec<Journal>   journals; 
    mutable cref_vec<Subject>   subjects;
    str_vec                     references;
public:
    string                  get_title() const { return title; }
    std::vector<Journal>    get_journals() const;
    std::vector<Subject>    get_subjects() const;
    std::vector<Author>     get_authors() const { return authors; };

    // I'm aware that it's a non-standard solution to implement
    // a builder class. However, in this case I've decided to stick to it,
    // since I don't really see any need for the Builder's inheritance here.
    class Builder
    {
    public:
        string      doi_b;
        string      title_b;         
        string      type_b;        
        date_vec    published_b;   
        double      score_b;
        date_vec    issued_b;      
        string      volume_b;
        string      issue_b;
        str_vec     ct_numbers_b;   
        int32_t     ref_num_b;
        int32_t     ref_by_num_b;
        mutable cref_vec<Journal>   journals_b;
        mutable cref_vec<Subject>   subjects_b;
        std::vector<Author>         authors_b;
        std::vector<string>         references_b;

        Article build() { return Article(*this); }

        Builder(string &&title, 
            string &&doi, 
            cref_vec<Journal> &&journals,
            std::vector<Author> &&authors);
    };
    
    // Article &operator =(const Article &) = delete;
    // Article(const Article &other) = delete;
    Article(Builder b);

    ~Article() { };
};

std::vector<Journal> Article::get_journals() const 
{  
    std::vector<Journal> journals_tmp;

    for (auto &jcref : journals)
    {
        auto j = jcref.get();
        journals_tmp.emplace_back(j);
    }

    return journals_tmp;
};

std::vector<Subject> Article::get_subjects() const
{
    std::vector<Subject> subjects_tmp;

    for (auto &subjcref : subjects)
    {
        auto s = subjcref.get();
        subjects_tmp.emplace_back(s);
    }
    return subjects_tmp;
};

Article::Builder::Builder(string &&title, 
    string &&doi, 
    cref_vec<Journal> &&journals,
    std::vector<Author> &&authors):
    doi_b(std::move(doi)), 
    title_b(std::move(title)), 
    journals_b(std::move(journals)),
    authors_b(std::move(authors))
{ };

Article::Article(Builder b): 
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
    subjects(std::move(b.subjects_b)),
    references(std::move(b.references_b))
{ 
    id_ = ++max_id_;
}

}