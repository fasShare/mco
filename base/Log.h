#ifndef MOXIE_LOG_H
#define MOXIE_LOG_H
#include <iostream>

#include <define.h>

#define LOGGER_TRACE(MSG)
#define LOGGER_DEBUG(MSG)
#define LOGGER_INFO(MSG)
#define LOGGER_WARN(MSG)
#define LOGGER_ERROR(MSG)
#define LOGGER_FATAL(MSG)
#define LOGGER_SYSERR(MSG)
//#define LOGGER_TRACE(MSG) (std::cout << "[" << gettid() << "]" << __FILE__<< " " << __LINE__ << " " << MSG << "\n")
//#define LOGGER_DEBUG(MSG) (std::cout << "[" << gettid() << "]" << __FILE__<< " " << __LINE__ << " " << MSG << "\n")
//#define LOGGER_INFO(MSG) (std::cout << "[" << gettid() << "]" << __FILE__<< " " << __LINE__ << " " << MSG << "\n")
//#define LOGGER_WARN(MSG) (std::cout << "[" << gettid() << "]" << __FILE__<< " " << __LINE__ << " " << MSG << "\n")
//#define LOGGER_ERROR(MSG) (std::cout << "[" << gettid() << "]" << __FILE__<< " " << __LINE__ << " " << MSG << "\n")
//#define LOGGER_FATAL(MSG) (std::cout << "[" << gettid() << "]" << __FILE__<< " " << __LINE__ << " " << MSG << "\n")
//#define LOGGER_SYSERR(MSG) (std::cout << "[" << gettid() << "]" << __FILE__<< " " << __LINE__ << " " << MSG << "\n")
#endif // MOXIE_LOG_H

