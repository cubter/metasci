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
// Publisher. publisher is a parent of journal. Each journal always has 
// exactly one publisher.
class publisher
{
public:
    string get_title() const { return title; };
    
    publisher &operator=(publisher &&other)      = default;
    publisher &operator=(const publisher &other) = default;

    publisher() {};
    publisher(string title);
    publisher(publisher &&other)        = default;
    publisher(const publisher &other)   = default;
    virtual ~publisher() {};

private:
    int32_t         id;  // my own id
    static int32_t  max_id_;
    string          title;
};

publisher::publisher(string title) : 
    title(std::move(title)) 
{ 
    // Each time a new instance of the class is created, the class' max 
    // id is incremeted, and the instance receives an ID.
    id = ++max_id_; 
};

// A journal is a child of a publisher. No journal can have more than one 
// publisher.
class journal : public publisher
{
public:

    inline string get_title() const           { return title; }
    inline string get_publisher_title() const { return publisher::get_title(); }
    
    journal &operator=(journal &&other)         = default;
    journal &operator=(const journal &other)    = default;

    journal() {};
    journal(string title, string publisher_title);
    journal(journal &&other)        = default;
    journal(const journal &other)   = default;
    virtual ~journal() {};
private:
    int32_t         id; // my own id
    static int32_t  max_id_;
    string          title;
};

journal::journal(string title, string publisher_title) :
    title(std::move(title)), 
    publisher(std::move(publisher_title)) 
{ 
    // Each time a new instance of the class is created, the class' max 
    // id is incremeted, and the instance receives an ID.
    id = ++max_id_; 
};
// Hasher & comparator to enable creation of unordered sets.
struct journal_hasher
{
    size_t operator()(const journal &j) const noexcept
    {
        return std::hash<string>()(j.get_title());
    }
};
struct journal_comparator
{
    bool operator()(const journal &j1, const journal &j2) const noexcept
    {
        return (j1.get_title() == j2.get_title());
    }
};

}