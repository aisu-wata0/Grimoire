
#include "Logger.hpp"

// std::ostream &operator<<(std::ostream &out, const gm::LogLvl &value)
// {
//    std::string s;
// #define CASE_VAL(p) \
//    case (p):        \
//       s = #p;       \
//       break;
//    switch (value)
//    {
//       CASE_VAL(gm::LogLvl::Fatal);
//       CASE_VAL(gm::LogLvl::Critical);
//       CASE_VAL(gm::LogLvl::Error);
//       CASE_VAL(gm::LogLvl::Warn);
//       CASE_VAL(gm::LogLvl::Note);
//       CASE_VAL(gm::LogLvl::Info);
//       CASE_VAL(gm::LogLvl::Debug);
//    }
// #undef CASE_VAL

//    return out << s;
// }