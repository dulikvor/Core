#pragma once

#include <list>

namespace core
{
	//OrderedList provide a mean to represent an ordered list, the class will not force the ordering, but its up to the user
	//to invoke the ordering request and to provide a weak order mean to validate that orderering (functor for that purpose).
	//The class provides the mean to retrieve the smallest element of the list if sorted (the user needs to know that).
	template<typename T>
	class OrderedList
	{
	public:
		explicit OrderedList(const std::list<T>& elements) : m_elements(elements){}
		OrderedList(){}
		~OrderedList(){}

		void PushBack(const T& element)
		{
			m_elements.push_back(element);
		}
		//First will return the first element residing with in the list, if sorted, that element is the smallest one by '<' ratio.
		const T& First()
		{
			m_elements.begin();
		}
		//Sort will sort the internal list, it will be done by using a received functor which will imply the '<' rulling.
		void Sort(const std::function<bool(const T& first, const T& second)>& comperator)
		{
			m_elements.sort(comperator);
		}
		//Sort will sort the internsl list, it will be done by using the < operator of the residing list elements.
		void Sort()
		{
			m_elements.sort();
		}
		
	private:
		std::list<T> m_elements;
	};
}
