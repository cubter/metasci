/* 
 * Copyright (c) 2022 - present, GitHub: @cubter
 * 
 * See COPYING.txt in the project root for license information.
 */

#include <string>
#include <iostream>
#include <fstream>
#include "nlohmann/json.hpp"

using string = std::string;

class Logger
{
public:
    virtual void write_msg(std::ofstream &of) = 0;
    virtual void write_msg(std::ostream &os) = 0;
    virtual int16_t get_msg_code() const = 0;
    
    Logger() { };
    virtual ~Logger() { };
};

class Json_logger: public Logger
{
private:
    int16_t msg_code;
    string  msg;
    string  context;
public:
    void write_msg(std::ofstream &of) { of << msg << '|' << context << '\n'; };
    void write_msg(std::ostream &os)  { os << msg << '|' << context << '\n'; };
    int16_t get_msg_code() const { return msg_code; }

    Json_logger(int16_t code, string message):
        msg_code(code),
        msg(std::move(message))
    { };
    Json_logger(int16_t code, string message, string context):
        msg_code(code),
        msg(std::move(message)),
        context(std::move(context))
    { };
    virtual ~Json_logger() { };
};