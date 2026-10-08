#pragma once
#include <vector>

template <class T>
class CStack
{
public:
	CStack();
	bool Push(T value);
	bool Pop(T& out);
private:
	std::vector<T> m_vecData;
	int m_iTop;
	int m_iCapacity;
};

template<class T>
inline CStack<T>::CStack()
{
	m_iCapacity = 20;
	m_vecData.resize(m_iCapacity);
	m_iTop = -1;
}

template<class T>
inline bool CStack<T>::Push(T value)
{
	if (m_iTop + 1 >= m_iCapacity)
		return false;

	m_vecData[++m_iTop] = value;
	return true;
}

template<class T>
inline bool CStack<T>::Pop(T& out)
{
	if (m_iTop < 0)
		return false;

	out = m_vecData[m_iTop--];
	return true;
}
