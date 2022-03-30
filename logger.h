/* 
 * Copyright (c) 2022 - present, GitHub: @cubter
 * 
 * See COPYING.txt in the project root for license information.
 */
#include <string>
#include <iostream>
#include <fstream>

using string = std::string;

namespace metasci
{
class Logger
{
public:
    virtual void write(std::ofstream &of) = 0;
    virtual void write(std::ostream &os) = 0;
    virtual int16_t get_message_code() const = 0;
    
    Logger() {};
    virtual ~Logger() {};
};

class Json_logger: public Logger
{
private:
    int16_t message_code;
    string  message;
    string  context;
public:
    void write(std::ofstream &of)   { of << message << '|' << context << '\n'; }
    void write(std::ostream &os)    { os << message << '|' << context << '\n'; }
    int16_t get_message_code() const { return message_code; }

    Json_logger(int16_t code, string message);
    Json_logger(int16_t code, string message, string context);
    virtual ~Json_logger() {};
};

Json_logger::Json_logger(int16_t message_code, string message):
    message_code(message_code),
    message(std::move(message))
{};

Json_logger::Json_logger(int16_t message_code, string message, string context):
    message_code(message_code),
    message(std::move(message)),        
    context(std::move(context))
{};
}