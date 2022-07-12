/* 
 * Copyright (c) 2022 - present, GitHub: @cubter
 * 
 * See COPYING.txt in the project root for license information.
 */
#include <iostream>
#include <vector>

namespace metasci
{
using str_vec   = std::vector<std::string>;
using string    = std::string;

// author. 
class author
{
public:
    string      get_first_name() const { return first_name; }
    string      get_family_name() const { return family_name; }
    str_vec     get_affiliations() const { return affiliations; }
    inline void add_affiliation(string aff);
    inline void set_affiliations(str_vec &&aff);
    inline void set_affiliations(const str_vec &aff);

    author   &operator=(author &&other)         = default;
    author   &operator=(const author &other)    = default; 

    author() {};
    author(string first_name, string family_name);
    author(string first_name, 
        string family_name, 
        string orcid, 
        bool is_authenticated_orcid); 
    author(author &&other)      = default;
    author(const author &other) = default;  
    ~author() {};

private:
    // id is my own internal ID. It has nothing to do with Crossref and is 
    // assigned automatically every time a new instance of author is created. 
    // Same applies to the other classes.  
    int32_t         id; 
    // From here on, the max_id_ param. is to track the currently assigned IDs. 
    // Every time an instance of author is created, max_id_ is incremented.
    static int32_t  max_id_; 
    string          orcid;  // unique author's ID; many authors lack it.
    bool            is_auth_orcid;  // is the ORCID authenticated
    string          first_name;     
    string          family_name;
    str_vec         affiliations;   // list of affiliations
};

// Adds an afiiliation to the list.
inline void author::add_affiliation(string aff) 
{ 
    affiliations.emplace_back(std::move(aff)); 
};
// Sets author's affilitations.
inline void author::set_affiliations(str_vec &&aff)
{
    affiliations = std::move(aff);
};
inline void author::set_affiliations(const str_vec &aff)
{
    affiliations = aff;
};

// Constructor for an author having no ORCID.
author::author(string first_name, 
    string family_name) : 
    first_name(std::move(first_name)), 
    family_name(std::move(family_name)) 
{};
// Constructor for an author having ORCID.
author::author(string first_name, 
    string family_name, 
    string orcid, 
    bool   is_authenticated_orcid) : 
    first_name(std::move(first_name)), 
    family_name(std::move(family_name)),
    orcid(std::move(orcid)), 
    is_auth_orcid(is_authenticated_orcid) 
{};
}