/* 
 * Copyright (c) 2022 - present, GitHub: @cubter
 * 
 * See COPYING.txt in the project root for license information.
 */

// #include "async_api_connector.h"
#include "conditional.h"
#include "article.h"
#include "logger.h"
// #include "orc/OrcFile.hh"

#define JSON_DIAGNOSTICS 1

#include "nlohmann/json.hpp"
#include "cpr/cpr.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <regex>

using Author                = metasci::Author;
using Article               = metasci::Article;
using Date                  = metasci::Date;
using date_vec              = metasci::date_vec;
using Journal               = metasci::Journal;
using Journal_hasher        = metasci::Journal_hasher;
using Journal_comparator    = metasci::Journal_comparator;
using Subject               = metasci::Subject;
using subj_vec              = std::vector<Subject>;
using Publisher             = metasci::Publisher;
using json_log_vec          = std::vector<metasci::Json_logger>;
using json                  = nlohmann::json;
using article_vec           = std::vector<Article>;
using journal_uset          = 
    std::unordered_set<Journal, Journal_hasher, Journal_comparator>;

using std::endl;
using std::cout;
using std::cerr;

// Setting static IDs of classes
int32_t Journal::max_id_ = 0;
int32_t Article::max_id_ = 0;
int32_t Publisher::max_id_ = 0;
int32_t Author::max_id_ = 0;
metasci::subj_id Subject::max_id_ = 0;

void 
parse_crossref_json(json &crossref_json,
    json_log_vec  &json_logs, 
    journal_uset  &journals,
    article_vec   &articles, 
    subj_vec      &subjects);

int main(int argc, char const *argv[])
{
    // string url_prefix = "https://api.crossref.org/works?rows=1000&select=DOI,title,author,publisher,published-online,container-title,score,issued,subject,published,volume,clinical-trial-number,references-count&cursor=*";
    // string url_suffix = "";
    
    std::ifstream inf(argv[1]);
    if (!inf)
    {
        cerr << "Couldn't open Crossref's json file. Aborting" << endl;
        return 1;
    }

    std::ofstream json_log_file("json_parser.log");
    if (!json_log_file)
    {
        cerr << "Couldn't open/create json_parser.log. Aborting" << endl;
        return 1;
    }

    // TODO: exception handling
    json j = json::parse(inf);

    journal_uset journals;
    std::vector<Article> articles;
    std::vector<Author> authors;
    json_log_vec json_logs;
    // You may ask, why didn't I define the subjects vector in the corresp. 
    // article file, like the publication types? Because I've no idea what 
    // subjects there are. Crossref doesn't provide a full list of 'em.  
    std::vector<Subject> subjects;

    auto t1 = std::chrono::high_resolution_clock::now();
    parse_crossref_json(j, json_logs, journals, articles, subjects);
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> m_diff = t2 - t1;
    std::cout << "time, ms: " << m_diff.count() << std::endl;


    // for (auto &a : articles)
    // {
    //     cout << "Article: " << a.get_title() << '\n';

    //     auto js = a.get_journals();
        
    //     cout << "Are journals empty? " << (js.size() == 0) << '\n';

    //     for (auto &j : js)
    //     {
    //         cout << j.get_title() << endl;
    //     }

    //     auto authors = a.get_authors();
        
    //     cout << "Authors: " << '\n';
        
    //     for (auto &a : authors)
    //     {
    //         cout << a.get_family_name() << '\n';
    //     }
        
    //     cout << '\n';

    //     auto subjects = a.get_subjects();
        
    //     cout << "Subjects: " << '\n';
        
    //     for (auto &s : subjects)
    //     {
    //         cout << s.get_title() << '\n';
    //     }

    //     cout << '\n';
    //     // std::copy(j_names.begiqn(), j_names.end(),
    //     //     std::ostream_iterator<string>(std::cout, "\n"),
    //     //     []q(string &jour)
    //     //     {
    //     //         return jour;
    //     //     }
    //     // )
    // }

    return 0;
}

// TODO: add checking status, if it's ok, if no, write log
void 
parse_crossref_json(json &crossref_json,
    json_log_vec  &json_logs, 
    journal_uset  &journals,
    article_vec   &articles,
    subj_vec      &subjects)
{
    json items;

    try
    {
        items = crossref_json.at("items");
    }
    catch(const json::exception &e)
    {
        json_logs.emplace_back(e.id, e.what(), "items missing");
        return;
    }

    for (auto &item : items)
    {   
        string title;
        string doi;
        string publisher;
        metasci::cref_vec<Journal>  journal_refs_tmp;
        std::vector<Author>         authors;

        try
        {
            // Nlohmann's JSON lib. supports implicit conversions, so 
            // item.at("title").at(0), although of json type, will be
            // converted to string.
            title = std::move(item.at("title").at(0));
        }
        catch(const json::exception &e)
        {
            std::cerr << e.what() << '\n';
        }

        try
        {
            doi = std::move(item.at("DOI"));
        }
        catch(const json::exception &e)
        {
            std::cerr << e.what() << '\n';
        }

        try
        {
            publisher = std::move(item.at("publisher"));
        }
        catch(const json::exception &e)
        {
            std::cerr << e.what() << '\n';
        }

        // Journals' titles. 
        try
        {
            json container_titles = item.at("container-title");

            for (auto &ct : container_titles)
            {
                Journal j(std::move(ct), std::move(publisher));

                metasci::cond::Emplacer<journal_uset, journal_uset::iterator, Journal> emp;

                auto emplace_res = emp.emplace_to(journals, std::forward<Journal>(j));
                
                journal_refs_tmp.emplace_back(*emplace_res.first);
            }
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, e.what(), "title: " + title);
        }
        catch(const json::exception &e)
        { }

        try
        {
            json    local_authors = item.at("author");    
            string  orcid; 
            bool    is_auth_orcid;

            for (const auto &loc_a : local_authors)
            {
                try
                {        
                    // ORCID is an author's unique ID. Quite many authors lack 
                    // it.      
                    auto cut_orcid = [&](string s) 
                    {  
                        return string{s.begin() + s.find_first_not_of("http://orcid.org/"), s.end()};
                    };

                    orcid = cut_orcid(loc_a.at("ORCID"));
                    is_auth_orcid = loc_a.at("authenticated-orcid");
                }
                catch(const json::type_error &e)
                {
                    json_logs.emplace_back(e.id, e.what(), "title: " + title);
                }    
                catch(const json::exception &e) 
                { }   

                authors.emplace_back(std::move(loc_a.at("given")), 
                    std::move(loc_a.at("family")), 
                    std::move(orcid), 
                    is_auth_orcid);
                
                // Author's affiliations; often left empty.
                try
                {                   
                    authors.back().set_affiliations(
                        std::move(loc_a.at("affiliation")));  
                }
                catch(const json::type_error &e)
                {
                    json_logs.emplace_back(e.id, e.what(), "title: " + title);
                }   
                catch(const json::exception &e) 
                { } 
            }
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, e.what(), "title: " + title);
        }   
        catch(const json::exception &e) 
        { }

        auto article_b = Article::Builder(std::move(title), 
            std::move(doi), 
            std::move(journal_refs_tmp), 
            std::move(authors));

        try
        {
            // Issues are short (usually, numbers encoded as strings), so no 
            // need to `move` them.
            article_b.issue_b = item.at("issue");
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, e.what(), "title: " + title);
        }
        catch(const json::exception &e) 
        { }
		
        try
        {
            // Volumes are short strings as well.
            article_b.volume_b = item.at("volume");
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, e.what(), "title: " + title);
        }
        catch(const json::exception &e) 
        { }
        
        try
        {
            // Types are also short.
            string type = item.at("type");
            
            auto it = 
                std::find(metasci::publication_types.begin(), metasci::publication_types.end(), type);
            
            if (it != metasci::publication_types.end())
            {
                article_b.type_b = it->id;
            }
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, e.what(), "title: " + title);
        }
        catch(const json::exception &e) 
        { }
		
        try
        {
            // Same. See above.
            article_b.ref_by_num_b = item.at("is-referenced-by-count");
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, e.what(), "title: " + title);
        }
        catch(const json::exception &e) 
        { }
		
        try
        {
            // Same. See above.
            article_b.ref_num_b = item.at("references-count");
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, e.what(), "title: " + title);
        }
        catch(const json::exception &e) 
        { }

        try
        {
            auto issued = item.at("issued").at("date-parts");

            for (auto &el : issued)
            {
                article_b.issued_b.emplace_back(Date{el.at(0), el.at(1), el.at(2)});
            }
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, e.what(), "title: " + title);
        }
        catch(const json::exception &e) 
        { }
    
        try
        {           
            json score = item.at("score");
            if (!score.is_null()) 
            {
                article_b.score_b = score;
            } 
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, e.what(), "title: " + title);
        }
        catch(const json::exception &e) 
        { }

        try
        {
            // The full list of subjects is not provided by Crossref; hence, 
            // it's updated during the parsing.
            json local_subjects = item.at("subject");

            for (auto &local_subj : local_subjects)
            {
                string local_subj_str = local_subj; 

                auto it = std::find(subjects.begin(), subjects.end(), 
                    local_subj_str);

                // If there already exists such subject in the global pool of 
                // subjects, add its ID to the article builder's subject list,
                if (it != subjects.end())
                {
                    article_b.subjects_ids_b.push_back(it->get_id());
                }
                // otherwise, firstly, add a new subject to the pool.
                else
                {
                    subjects.emplace_back(local_subj_str);
                    article_b.subjects_ids_b.push_back(subjects.back().get_id()); 
                }
            }      
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, e.what(), "title: " + title);
        }
        catch(const json::exception &e) 
        { }
        
        try
        {
            // Clinical trial number is nothing else than NCT ID. 
            json ct_nums = item.at("clinical-trial-number");

            std::move(ct_nums.begin(), ct_nums.end(), std::back_inserter(article_b.ct_numbers_b));         
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, e.what(), "title: " + title);
        }
        catch(const json::exception &e) 
        { }
		
		// I prefer the date of online publication if it's present, since, 
        // well, it's an online era.
        try
        {
            bool is_published = item.contains("published-online"); 
            json published_dates;

            if (!is_published)
            {
                published_dates = item.at("published-print").at("date-parts");
            }
            else
            {
                published_dates = item.at("published-online").at("date-parts");
            }

            for (auto &pd : published_dates)
            {
                article_b.published_b.emplace_back(Date{pd.at(0), pd.at(1), pd.at(2)});
            }
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, e.what(), "title: " + title);
        }
        catch(const json::exception &e) 
        { }
		
		// All the references in the publication.
        try
        {
            json references = item.at("reference");

            for (auto &el : references)
            {
                article_b.references_b.emplace_back(el.at("DOI"));   
            }            
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, e.what(), "title: " + title);
        }
        catch(const json::exception &e) 
        { }
        
        articles.emplace_back(*article_b.build().get());
    }  
}
