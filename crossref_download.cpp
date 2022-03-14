/* 
 * Copyright (c) 2022 - present, GitHub: @cubter
 * 
 * See COPYING.txt in the project root for license information.
 */

// #include "async_api_connector.h"
#include "article.h"
#include "logger.h"
#include "orc/OrcFile.hh"

#define JSON_DIAGNOSTICS 1

#include "nlohmann/json.hpp"
#include "cpr/cpr.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <random>
#include <regex>

using std::endl;
using std::cout;
using std::cerr;
using json          = nlohmann::json;
using journal_uset  = 
    std::unordered_set<Journal, Journal_hasher, Journal_comparator>;
using article_vec   = std::vector<Article>;
using subject_uset  = 
    std::unordered_set<Subject, Subject_hasher, Subject_comparator>;

struct Id_tracker
{
    int32_t subj_max_id;
    int32_t author_max_id;
    int32_t journal_max_id;
    int32_t publisher_max_id;
    int32_t article_max_id;
};

std::vector<uint32_t>
gen_rand_seq(uint32_t min, uint32_t max, uint32_t seq_len);
void 
parse_crossref_json_files(
    std::ifstream &inf, 
    std::ofstream &json_log, 
    journal_uset  &journals,
    article_vec   &articles, 
    subject_uset  &subjects,
    Id_tracker    &id_tracker);

int main(int argc, char const *argv[])
{
    string url_prefix = "https://api.crossref.org/works?rows=1000&select=DOI,title,author,publisher,published-online,container-title,score,issued,subject,published,volume,clinical-trial-number,references-count&cursor=*";
    string url_suffix = "";
    
    // str_vec urls;
    
    // const string tls_v = "1.2";

    // cpr::SslOptions ssl_opts = cpr::Ssl(cpr::ssl::TLSv1_2{ });
    // cpr::Response   resp     = cpr::Get(cpr::Url{ingred_url}, ssl_opts); 
    // if (resp.status_code != 200) 
    // {
    //     cout << "Crossref's response error. Status code: " << resp.status_code << endl;
    //     return 1;
    // }

    // json crossref_first_resp = json::parse(resp.text);

    std::ifstream inf(argv[1]);
    if (!inf)
    {
        cerr << "Couldn't open test.json. Aborting" << endl;
        return 1;
    }

    std::ofstream json_log_file("json_parser.log");
    if (!json_log_file)
    {
        cerr << "Couldn't open json_parser.log. Aborting" << endl;
        return 1;
    }

    journal_uset journals;
    std::vector<Article> articles;
    std::vector<Author> authors;
    subject_uset subjects;
    Id_tracker id_tracker;

    parse_crossref_json_files(inf, json_log_file, journals, articles, subjects, id_tracker);

    // for (auto &a : articles)
    // {
    //     auto js = a.get_journals();
        
    //     for (auto &j : js)
    //     {
    //         json_log_file << j.get_title() << endl;
    //     }
        
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

std::vector<uint32_t>
gen_rand_seq(uint32_t min, uint32_t max, uint32_t seq_len)
{
    std::random_device rdev;
    std::mt19937::result_type seed = rdev();
    std::mt19937 rng(seed);  
    std::uniform_int_distribution<uint32_t> dist(min, max);
    
    std::vector<uint32_t> v;

    for (int32_t i = 0; i < seq_len; i++)
    {
        v.push_back(dist(rng));
    }

    return v;
};

// TODO: add checking status, if it's ok, if no write log
void 
parse_crossref_json_files(
    std::ifstream &inf, 
    std::ofstream &json_log, 
    journal_uset  &journals,
    article_vec   &articles,
    subject_uset  &subjects,
    Id_tracker    &id_tracker)
{
    std::vector<Json_logger> json_logs;

    json crossref_file = json::parse(inf);

    json items;

    try
    {
        items = crossref_file.at("items");
    }
    catch(const json::exception &e)
    {
        std::cerr << e.what() << '\n';
        return;
    }

    for (auto &item : items)
    {   
        string title;
        string doi;
        string publisher;
        std::vector<std::reference_wrapper<const Journal>> journal_refs_tmp;
        std::vector<Author> authors;

        try
        {
            title = item.at("title").at(0);
        }
        catch(const json::exception &e)
        {
            std::cerr << e.what() << '\n';
        }

        try
        {
            doi = item.at("DOI");
        }
        catch(const json::exception &e)
        {
            std::cerr << e.what() << '\n';
        }

        try
        {
            publisher = item.at("publisher");
        }
        catch(const json::exception &e)
        {
            std::cerr << e.what() << '\n';
        }

        // Journals' titles. 
        try
        {
            auto container_title = item.at("container-title");

            for (auto &el : container_title)
            {
                journals.emplace(el, publisher);

                Journal j(el, publisher);

                auto j_it = journals.find(j);
                if (j_it != journals.end())
                {
                    journal_refs_tmp.emplace_back(*j_it);
                }
            }
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, e.what(), "title: " + title);
        }
        catch(...) 
        { }

        try
        {
            json author = item.at("author");
      
            string  orcid; 
            bool    is_auth_orcid;

            for (const auto &el : author)
            {
                try
                {              
                    auto cut_orcid = [&](string s)
                    {  
                        return string{s.begin() + s.find_first_not_of("http://orcid.org/"), s.end()};
                    };

                    orcid = cut_orcid(el.at("ORCID"));

                    is_auth_orcid = el.at("authenticated-orcid");
                }
                catch(const json::type_error &e)
                {
                    json_logs.emplace_back(e.id, e.what(), "title: " + title);
                }    
                catch(const json::exception &e) 
                { }   

                authors.emplace_back(el.at("given"), el.at("family"), orcid, is_auth_orcid);
                
                try
                {
                    json affiliations = el.at("affiliation");

                    for (auto &aff : affiliations)
                    {
                        authors.back().add_affiliation(aff);
                    }   
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

        auto article_b = Article::Builder(title, doi, journal_refs_tmp, authors);

        // Issues are short, so small string optim-on will possibly be applied.
        try
        {
            article_b.issue_b = item.at("issue");
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, e.what(), "title: " + title);
        }
        catch(const json::exception &e) 
        { }

        // Volumes are short, so small string optim-on will possibly be applied.
        try
        {
            article_b.volume_b = item.at("volume");
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, e.what(), "title: " + title);
        }
        catch(const json::exception &e) 
        { }
        
        // Types are short, so small string optim-on will possibly be applied.
        try
        {
            article_b.type_b = item.at("type");
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, e.what(), "title: " + title);
        }
        catch(const json::exception &e) 
        { }

        try
        {
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
            auto score = item.at("score");
            if (!score.is_null()) article_b.score_b = score; 
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, e.what(), "title: " + title);
        }
        catch(const json::exception &e) 
        { }

        try
        {
            json subject = item.at("subject");
            
            for (const auto &el : subject)
            {
                bool is_emplaced = subjects.emplace(Subject{el}).second;

                if (is_emplaced) id_tracker.subj_max_id += 1;
                
                auto subj_it = subjects.find(Subject{el});
                if (subj_it != subjects.end())
                {
                    article_b.subjects_b.emplace_back(*subj_it);
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
            json ct_num = item.at("clinical-trial-number");

            for (auto &el : ct_num)
            {
                article_b.ct_numbers_b.emplace_back(el);
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
            bool is_published = item.contains("published-online"); 
            json published;

            if (!is_published)
            {
                published = item.at("published-print").at("date-parts");
            }
            else
            {
                published = item.at("published-online").at("date-parts");
            }

            for (auto &el : published)
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
        
        articles.emplace_back(article_b.build());
    }  

    for (auto &el : json_logs) 
    {
        el.write_msg(json_log); 
    }
}