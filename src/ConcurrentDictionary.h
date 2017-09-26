#pragma once

#include <map>
#include <mutex>
#include "Exception.h"

namespace core
{
	//Concurrentdictionary represents a minimal synchronise dictioary, with the following capabilities:
	//1) Add, retrieve, and remove a value by a given key.
	//2) Determine if a specific value exists by a given key.
	template <typename Key, typename Value>
	class ConcurrentDictionary
	{
	public:
		ConcurrentDictionary(){}
		~ConcurrentDictionary(){}
		//AddValue receives a kay and a value, the function will determine if the recieved key already exists
		//with in the dictionary, if no it will copy the recieved value under the given key, if yes an exception
		//will be thrown (overwrite is not legit), all will be done in sync manner.
		void AddValue(const Key& key, const Value& value)
		{
			std::unique_lock<std::mutex> localLock(m_mutex);
			m_dictionary.find(key) == m_dictionary.end() ? m_dictionary[key] = value :
				throw Exception(SOURCE, "An existing key was provided");
		}
		//RemoveValue receives a key and attempts to remove the assosiate entry from the stored dictionary.
		void RemoveValue(const Key& key)
		{
			std::unique_lock<std::mutex> localLock(m_mutex);
			m_dictionary.erase(key);
		}
		//ContaisKey receives a key, the function will determine if the given key exists with in the dictioary, returning true or false accordinaly.
		bool ContainsKey(const Key& key) const
		{
			std::unique_lock<std::mutex> localLock(m_mutex);
			return m_dictionary.find(key) != m_dictionary.end();
		}
		//operator [] will try to return a specific value designated by a received key, if the value dosn't not exits
		//it will be added and returned, else - just returned.
		Value& operator[](const Key& key) 
		{
			std::unique_lock<std::mutex> localLock(m_mutex);
			return m_dictionary[key];
		}

		std::vector<Key> GetAllKeys() const{
			std::unique_lock<std::mutex> localLock(m_mutex);
			std::vector<Key> keys;
			keys.reserve(m_dictionary.size());
			for(auto const & pair : m_dictionary)
				keys.push_back(pair.first);
			return keys;
		}


	private:
		std::map<Key, Value> m_dictionary;
		mutable std::mutex m_mutex;
	};
}
