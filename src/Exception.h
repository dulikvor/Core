#pragma once

#include <exception>
#include <string>
#include "Logger.h"

namespace core
{
	/**/
	class Exception : public std::exception
	{
	public:
		/*Exception constructor will megerge the received exception message and print out the message and the entire
		stack frames to the log.*/
		template<typename ... Args>
		Exception(const Source& source, const char* format, Args ... args)
		{
			m_message = Logger::Instance().BuildMessage(source, format, args...);
			TRACE_ERROR("%s", m_message.c_str());
			Logger::Instance().PrintStack();
			Logger::Instance().Flush();
		}

		Exception() = delete;
		virtual ~Exception() noexcept {}
		//Accessor
		std::string GetMessage() const {return m_message;}
	private:
		std::string m_message;
	};
}
