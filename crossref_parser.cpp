/* 
 * Copyright (c) 2022 - present, GitHub: @cubter
 * 
 * See COPYING.txt in the project root for license information.
 */

// #include "async_api_connector.h"
#include "article.h"
#include "conditional.h"
#include "logger.h"

#define JSON_DIAGNOSTICS 1

#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <regex>

using author                = metasci::author;
using article               = metasci::article;
using date                  = metasci::date;
using date_vec              = metasci::date_vec;
using journal               = metasci::journal;
using journal_hasher        = metasci::journal_hasher;
using journal_comparator    = metasci::journal_comparator;
using subject               = metasci::subject;
using publication_type      = metasci::publication_type;
using subject_vec           = std::vector<subject>;
using pub_type_vec          = std::vector<publication_type>;
using publisher             = metasci::publisher;
using json_log_vec          = std::vector<metasci::json_log>;
using json                  = nlohmann::json;
using article_vec           = std::vector<article>;
using journal_uset          = 
    std::unordered_set<journal, journal_hasher, journal_comparator>;

using std::endl;
using std::cout;
using std::cerr;

// Setting currently assigned max IDs
int32_t journal::max_id_                        = 0;
int32_t article::max_id_                        = 0;
int32_t publisher::max_id_                      = 0;
int32_t author::max_id_                         = 0;
metasci::subject_id subject::max_id_            = 0;
metasci::pub_type_id publication_type::max_id_ = 0;

void usage(int argc);
void parse_crossref_json(json &crossref_json,
    json_log_vec  &json_logs, 
    journal_uset  &journals,
    article_vec   &articles, 
    subject_vec   &subjects,
    pub_type_vec  &publication_types);

int main(int argc, char const *argv[])
{
    // List of current pulication types
    std::vector<publication_type> publication_types
    {
        { "book_section"        },
        { "monograph"           },
        { "report"              },
        { "peer_review"         },
        { "book_track"          },
        { "journal_article"     },
        { "book_part"           },
        { "other"               },
        { "book"                },
        { "journal_volume"      },
        { "book_set"            },
        { "reference_entry"     },
        { "proceedings_article" },
        { "journal"             },
        { "component"           },
        { "book_chapter"        },
        { "proceedings_series"  },
        { "report_series"       },
        { "proceedings"         },
        { "standard"            },
        { "reference_book"      },
        { "posted_content"      },
        { "journal_issue"       },
        { "dissertation"        },
        { "grant"               },
        { "dataset"             },
        { "book_series"         },
        { "edited_book"         },
        { "standard_series"     }
    };

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

    json crossref_json;
    try
    {
        crossref_json = json::parse(inf);
    }
    catch(const std::exception& e)
    {
        cerr << e.what() << '\n';
        return 1;
    }

    std::vector<article>    articles;
    std::vector<author>     authors;
    std::vector<subject>    subjects;
    journal_uset            journals;
    json_log_vec            json_logs;


    parse_crossref_json(crossref_json, json_logs, journals, articles, subjects, 
        publication_types);

    return 0;
}

void usage(int argc)
{
    if (argc != 2)
    {
        cerr << "Wrong input. Usage: crossref_download <file_name>" << endl;
    }
}

// Parses Crossref's JSONs.
void parse_crossref_json(json &crossref_json,
    json_log_vec  &json_logs, 
    journal_uset  &journals,
    article_vec   &articles, 
    subject_vec   &subjects,
    pub_type_vec  &publication_types)
{
    json items; // the highest level structure

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
        metasci::cref_vec<journal>  journal_refs;
        std::vector<author>         authors;

        try
        {
            // Nlohmann's JSON lib. supports implicit conversions, so 
            // item.at("title").at(0) will be converted to string.
            title = std::move(item.at("title").at(0));
        }
        catch(const json::exception &e)
        {
            json_logs.emplace_back(e.id, e.what(), "title missing.");
            return;
        }

        try
        {
            doi = std::move(item.at("DOI"));
        }
        catch(const json::exception &e)
        {
            json_logs.emplace_back(e.id, e.what(), "title: " + title);
            return;
        }

        try
        {
            publisher = std::move(item.at("publisher"));
        }
        catch(const json::exception &e)
        {
            json_logs.emplace_back(e.id, std::move(e.what()), "title: " + title);
            return;
        }

        // journals' titles. 
        try
        {
            json container_titles = item.at("container-title");

            for (auto &ct : container_titles)
            {
                journal j(std::move(ct), std::move(publisher));

                metasci::cond::Emplacer<journal_uset, journal_uset::iterator, journal> emp;

                auto emplace_res = emp.emplace_to(journals, std::forward<journal>(j));
                
                journal_refs.emplace_back(*emplace_res.first);
            }
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, std::move(e.what()), "title: " + title);
        }
        // There will be a lot out of range errors, since many elements may be 
        // absent in the concrete json file. I don't need to catch them -- I'm 
        // only interested in type errors, -- hence the body is empty. Same 
        // applies below
        catch(const json::exception &e)
        { }

        try
        {
            json    local_authors = item.at("author");    
            string  orcid; 
            bool    is_auth_orcid;

            for (const auto &local_author : local_authors)
            {
                try
                {        
                    // ORCID is an author's unique ID. Many authors lack it.
                    auto cut_orcid = [&](string s) 
                    {  
                        return string{s.begin() + s.find_first_not_of("http://orcid.org/"), s.end()};
                    };

                    orcid = cut_orcid(local_author.at("ORCID"));
                    is_auth_orcid = local_author.at("authenticated-orcid");
                }
                catch(const json::type_error &e)
                {
                    json_logs.emplace_back(e.id, std::move(e.what()), 
                        "title: " + title);
                }    
                catch(const json::exception &e) 
                { }   

                authors.emplace_back(std::move(local_author.at("given")), 
                    std::move(local_author.at("family")), 
                    std::move(orcid), 
                    is_auth_orcid);
                
                // author's affiliations; often left empty.
                try
                {                   
                    authors.back().set_affiliations(
                        std::move(local_author.at("affiliation")));  
                }
                catch(const json::type_error &e)
                {
                    json_logs.emplace_back(e.id, std::move(e.what()), 
                        "title: " + title);
                }   
                catch(const json::exception &e) 
                { } 
            }
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, std::move(e.what()), 
                "title: " + title);
        }   
        catch(const json::exception &e) 
        { }

        auto article_b = article::builder(std::move(title), 
            std::move(doi), 
            std::move(journal_refs), 
            std::move(authors));

        try
        {
            // Issues are short (usually, numbers encoded as strings), so no 
            // need to "move" them.
            article_b.issue_b = item.at("issue");
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, std::move(e.what()), 
                "title: " + title);
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
            json_logs.emplace_back(e.id, std::move(e.what()), 
                "title: " + title);
        }
        catch(const json::exception &e) 
        { }
        
        try
        {
            // Types are also short.
            string type = item.at("type");
            
            auto it = std::find(publication_types.begin(),  
                publication_types.end(), type);
            
            if (it != publication_types.end())
            {
                article_b.type_b = it->get_id();
            }
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, std::move(e.what()), 
                "title: " + title);
        }
        catch(const json::exception &e) 
        { }
		
        try
        {
            article_b.ref_by_num_b = item.at("is-referenced-by-count");
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, std::move(e.what()), 
                "title: " + title);
        }
        catch(const json::exception &e) 
        { }
		
        try
        {
            article_b.ref_num_b = item.at("references-count");
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, std::move(e.what()), 
                "title: " + title);
        }
        catch(const json::exception &e) 
        { }

        try
        {
            auto issued = item.at("issued").at("date-parts");

            for (auto &el : issued)
            {
                article_b.issued_b.emplace_back(date{el.at(0), el.at(1), el.at(2)});
            }
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, std::move(e.what()), 
                "title: " + title);
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
            json_logs.emplace_back(e.id, std::move(e.what()), 
                "title: " + title);
        }
        catch(const json::exception &e) 
        { }

        try
        {
            // The full list of subjects is not provided by Crossref; hence, 
            // it's updated during the parsing.
            json local_subjects = item.at("subject");

            for (auto &local_subject : local_subjects)
            {
                string local_subject_str = local_subject; 

                auto it = std::find(subjects.begin(), subjects.end(), 
                    local_subject_str);

                // If there already exists such subject in the global pool of 
                // subjects, add its ID to the article builder's subject list,
                if (it != subjects.end())
                {
                    article_b.subjects_ids_b.push_back(it->get_id());
                }
                // otherwise, firstly, add a new subject to the pool.
                else
                {
                    subjects.emplace_back(local_subject_str);
                    article_b.subjects_ids_b.push_back(subjects.back().get_id()); 
                }
            }      
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, std::move(e.what()), 
                "title: " + title);
        }
        catch(const json::exception &e) 
        { }
        
        try
        {
            // Clinical trial number is nothing else than NCT ID. 
            json ct_nums = item.at("clinical-trial-number");

            std::move(ct_nums.begin(), ct_nums.end(), 
                std::back_inserter(article_b.ct_numbers_b));         
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, std::move(e.what()), 
                "title: " + title);
        }
        catch(const json::exception &e) 
        { }
		
		// I prefer the date of online publication if it's present, since, 
        // well, it's an online era.
        try
        {
            bool is_published = item.contains("published-online"); 
            // published-online is an array, so there may be several dates. 
            // Note: I'm not sure what that means in practice.
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
                article_b.published_b.emplace_back(date{pd.at(0), pd.at(1), pd.at(2)});
            }
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, std::move(e.what()), 
                "title: " + title);
        }
        catch(const json::exception &e) 
        { }
		
        try
        {
            json references = item.at("reference"); // list of references

            for (auto &el : references)
            {
                article_b.references_b.emplace_back(el.at("DOI"));   
            }            
        }
        catch(const json::type_error &e)
        {
            json_logs.emplace_back(e.id, std::move(e.what()), 
                "title: " + title);
        }
        catch(const json::exception &e) 
        { }
        
        articles.push_back(std::move(article_b.build()));
    }  
}
