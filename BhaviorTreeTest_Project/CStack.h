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
	std::vector<T> m_vec;
	int m_Top;
	int m_Size;
	int m_Count;
};

template<class T>
inline CStack<T>::CStack()
{
	m_Size = 20;
	m_vec.resize(m_Size);
	m_Top = -1;
	m_Count = 0;
}

template<class T>
inline bool CStack<T>::Push(T value)
{
	if (m_Count >= m_Size)
		return false;

	m_vec[++m_Top];
	m_Count++;
	return true;
}

template<class T>
inline bool CStack<T>::Pop(T& out)
{
	if (m_Count <= 0)
		return false;

	out = m_vec[m_Top--];
	m_Count--;
	return true;
}