#pragma once

#include <list>
#include <utility>
#include <string>

namespace core
{
	class CommandLine
	{
	public:
		~CommandLine(){}
		static CommandLine& Instance();
		void Parse(int argc, char const** argv);
		const std::string& GetArgument(const std::string& argumentName) const;

	private:
		CommandLine(){}
		bool ParseArgument(const char* argument, std::string& argName, std::string& argValue);

	private:
		using ArgumentList = std::list<std::pair<std::string, std::string>>;
		 ArgumentList m_arguments;

	};
}
