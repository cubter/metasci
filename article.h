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
#include <memory>

using str_vec   = std::vector<std::string>;
using string    = std::string;

namespace metasci
{
class Author
{
private:
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
    void    add_affiliation(string &&aff) { affiliations.emplace_back(std::move(aff)); }
    void    set_affiliations(str_vec &&aff);

    Author() { };
    Author(string &&first_name, string &&family_name);
    Author(string &&first_name, 
        string &&family_name, 
        string &&orcid, 
        bool is_authenticated_orcid); 
    ~Author() { };
};

void Author::set_affiliations(str_vec &&aff)
{
    affiliations = std::move(aff);
}

Author::Author(string &&first_name, 
    string &&family_name) : 
    first_name(std::move(first_name)), 
    family_name(std::move(family_name)) 
{ };

Author::Author(string &&first_name, 
    string &&family_name, 
    string &&orcid, 
    bool   is_authenticated_orcid) : 
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

Publisher::Publisher(string &&title) : 
    title(std::move(title)) 
{ 
    id_ = ++max_id_; 
};

class Journal : public Publisher
{
private:
    int32_t         id_;
    static  int32_t max_id_;
    string          title;

public:
    string get_title() const            { return title; }
    string get_publisher_title() const  { return Publisher::get_title(); }

    Journal() { };
    Journal(string &&title, string &&publisher_title);

    virtual ~Journal() { };
};

Journal::Journal(string &&title, string &&publisher_title) :
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
    uint16_t    year;
    uint8_t     month;
    uint8_t     day;
};

using pub_type_id = int8_t;

struct Publication_type
{
    string      crossref_id;
    pub_type_id id;

    bool operator==(const Publication_type &other);
    Publication_type(string crossref_id);
    Publication_type(string crossref_id, pub_type_id internal_id);
};

Publication_type::Publication_type(string crossref_id) :
    crossref_id(crossref_id) 
{ };

Publication_type::Publication_type(string crossref_id, pub_type_id internal_id) 
: crossref_id(crossref_id), 
id(internal_id)
{ };

bool Publication_type::operator==(const Publication_type &other)
{
    return crossref_id == other.crossref_id;
};

using subj_id = int16_t;

class Subject
{
private:
    subj_id         id;
    static subj_id  max_id_;
    string          title;

public:
    subj_id get_id() const { return id; };
    bool operator==(const Subject &other);
    Subject(string title);
};

bool Subject::operator==(const Subject &other)
{
    return title == other.title;
};

Subject::Subject(string subj_title) :
    title(subj_title)
{
    id = ++max_id_;
};

// Pulication types: crossref's ID/name, id.
// Integer IDs instead of the Crossref's ones are to reduce space consumption. 
std::vector<Publication_type> publication_types
{
    { "book_section", 1         },
    { "monograph", 2            },
    { "report", 3               },
    { "peer_review", 4          },
    { "book_track", 5           },
    { "journal_article", 6      },
    { "book_part", 7            },
    { "other", 8                },
    { "book", 9                 },
    { "journal_volume", 10      },
    { "book_set", 11            },
    { "reference_entry", 12     },
    { "proceedings_article", 13 },
    { "journal", 14             },
    { "component", 15           },
    { "book_chapter", 16        },
    { "proceedings_series", 17  },
    { "report_series", 18       },
    { "proceedings", 19         },
    { "standard", 20            },
    { "reference_book", 21      },
    { "posted_content", 22      },
    { "journal_issue", 23       },
    { "dissertation", 24        },
    { "grant", 25               },
    { "dataset", 26             },
    { "book_series", 27         },
    { "edited_book", 28         },
    { "standard_series", 29     }
};

using date_vec = std::vector<Date>;

template<typename T>
using cref_vec = std::vector<std::reference_wrapper<const T>>;

class Article
{
private:
    int32_t         id_;
    static int32_t  max_id_;
    string          doi;
    string          title;          // article's title
    pub_type_id     type;           // journal article etc. 
    date_vec        published;      // either online (pref.) or print
    int32_t         score;
    date_vec        issued;         // date of issue
    string          volume;
    string          issue;  
    // It doesn't make sense to make NCT IDs std::ref, since almost 
    // never two articles refer to the same ID.        
    str_vec         ct_numbers; 
    int32_t         ref_num;
    int32_t         ref_by_num; 
    str_vec         references;
    // I've decided to use subjects' IDs instead of references to Subjects
    // for the reason of space optimization: unlike Journal, Subject is quite 
    // a simple class, and there are not so many subjects out there. Hence, I 
    // don't see the need for the references here. 
    std::vector<subj_id>        subjects_ids;
    std::vector<Author>         authors;
    mutable cref_vec<Journal>   journals; 

public:
    string                  get_title() const { return title; };
    std::vector<Journal>    get_journals() const;
    std::vector<Author>     get_authors() const { return authors; };
    std::vector<subj_id>    get_subjects_ids() const { return subjects_ids; };
    pub_type_id             get_type() const { return type; };

    // I'm aware that it's a non-standard solution to implement
    // a builder class. However, in this case I've decided to stick to it,
    // since I don't really see any need for the Builder's inheritance here.
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
        std::vector<subj_id>        subjects_ids_b;
        std::vector<Author>         authors_b;

        std::unique_ptr<Article> build();

        Builder(string &&title, 
            string &&doi, 
            cref_vec<Journal> &&journals,
            std::vector<Author> &&authors);
    };
    
    Article(Builder &b);

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

std::unique_ptr<Article> Article::Builder::build()
{ 
    auto art_ptr = std::make_unique<Article>(Article(*this)); 

    this->~Builder(); 
    
    return art_ptr; 
};

Article::Builder::Builder(string &&title, 
    string &&doi, 
    cref_vec<Journal> &&journals,
    std::vector<Author> &&authors) :
    doi_b(std::move(doi)), 
    title_b(std::move(title)), 
    journals_b(std::move(journals)),
    authors_b(std::move(authors))
{ };

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
    id_ = ++max_id_;
}

}