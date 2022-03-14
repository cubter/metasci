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

using str_vec   = std::vector<std::string>;
using string    = std::string;


class Author
{
    int32_t         _id = -1;
    string          orcid;
    bool            is_auth_orcid;
    string          first_name;
    string          family_name;
    str_vec         affiliations;

public:
    string  get_first_name() const { return first_name; }
    string  get_family_name() const { return family_name; }
    str_vec get_affiliations() const { return affiliations; }
    string  add_affiliation(string aff) { affiliations.push_back(std::move(aff)); }

    Author() { };
    Author(string first_name, string family_name): 
        first_name(first_name), 
        family_name(family_name) 
    { };
    Author(
        string first_name, 
        string family_name, 
        string orcid, 
        bool   is_authenticated_orcid): 
        first_name(first_name), 
        family_name(family_name),
        orcid(orcid),
        is_auth_orcid(is_authenticated_orcid) 
    { };
    Author(string first_name, string family_name, str_vec affiliations): 
       first_name(first_name), 
       family_name(family_name), 
       affiliations(std::move(affiliations)) 
    { };
    Author(
        string first_name, 
        string family_name, 
        str_vec affiliations,
        string orcid, 
        bool   is_authenticated_orcid): 
        first_name(first_name), 
        family_name(family_name), 
        affiliations(std::move(affiliations)),
        orcid(orcid),
        is_auth_orcid(is_authenticated_orcid) 
    { };
    ~Author() { };
};

class Publisher
{
private:
    int32_t _id;
    string  title;
public:
    string get_title() const { return title; };

    Publisher() { };
    Publisher(string title): title{title} { };
    virtual ~Publisher() { };
};

class Journal: public Publisher
{
private:
    int32_t _id;
    string  title;
public:
    string get_title() const { return title; }
    string get_publisher_title() const { return(Publisher::get_title()); }

    Journal() { }
    Journal(string title, string publisher_title): 
        title(title), 
        Publisher(publisher_title) 
    { }

    virtual ~Journal() { };
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

struct Subject
{
    string  title;
    int32_t _id = -1;
};
struct Subject_hasher
{
    size_t operator()(const Subject &s) const noexcept
    {
        return std::hash<string>()(s.title);
    }
};
struct Subject_comparator
{
    bool operator()(const Subject &s1, const Subject &s2) const noexcept
    {
        return (s1.title == s2.title);
    }
};
// class Article_builder
// {
// private:
//     std::unique_ptr<Article> article;
// public:
//     Article *get_article() 
//     {
//         auto res = this->article.get();
//         return res;
//     };
//     void set_type(string type) { this->article.get(). }

//     Article_builder(/* args */);
//     ~Article_builder();
// };

class Article
{
private:
    int32_t     _id;
    string      doi;
    string      title;          // article titles
    string      type;           // journal article etc. 
    date_vec    published;      // either online (pref.) or print
    double      score;
    date_vec    issued;         // date of issue
    string      volume;
    string      issue;          
    str_vec     ct_numbers;      // it doesn't make sense to make ref-s to NCT 
                                // IDs, since almost never two articles ref. 
                                // the same one
    int32_t     ref_num;
    int32_t     ref_by_num; 
    std::vector<Author> authors;
    mutable std::vector<std::reference_wrapper<const Journal>> journals; 
    std::vector<std::reference_wrapper<const Subject>> subjects;  // optional
    str_vec     references;   // optional
public:
    string                get_title() const { return title; }
    std::vector<Journal>  get_journals() 
    {  
        std::vector<Journal> journals_tmp;

        for (auto &jcref : journals)
        {
            auto j = jcref.get();
            journals_tmp.emplace_back(j);
        }
    
        return journals_tmp;
    }

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
        std::vector<std::reference_wrapper<const Journal>> journals_b;
        std::vector<Author> authors_b;
        std::vector<std::reference_wrapper<const Subject>> subjects_b;
        std::vector<string> references_b;

        Article build()
        {
            return Article(*this);
        }

        Builder(
            string title, 
            string doi, 
            std::vector<std::reference_wrapper<const Journal>> journals,
            std::vector<Author> authors): 
            doi_b(std::move(doi)), 
            title_b(std::move(title)), 
            journals_b(std::move(journals)),
            authors_b(std::move(authors))
        { }
    };
    
    // Article &operator =(const Article &) = delete;
    // Article(const Article &other) = delete;
    Article(Builder b): 
        doi(b.doi_b), 
        title(b.title_b), 
        type(b.type_b), 
        published(b.published_b),
        score(b.score_b), 
        issued(b.issued_b), 
        volume(b.volume_b), 
        issue(b.issue_b), 
        ct_numbers(b.ct_numbers_b), 
        ref_num(b.ref_num_b),
        ref_by_num(b.ref_by_num_b), 
        journals(b.journals_b),
        authors(b.authors_b),
        subjects(b.subjects_b),
        references(b.references_b)
    { }

    ~Article() { };
};
