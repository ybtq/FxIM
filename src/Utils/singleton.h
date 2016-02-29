#ifndef _SINGLETON_H_
#define _SINGLETON_H_

#include <memory>

template<typename T>
class Singleton
{
public:
    static T* getInstance()
	{
        if (m_pInstance.get() == NULL)
        {
			if (m_hMutex == NULL)
			{
				m_hMutex = CreateMutex (NULL, FALSE, NULL);
			}
			WaitForSingleObject(m_hMutex, INFINITE);
            if (m_pInstance.get() == NULL)
                m_pInstance.reset(new T);
            ReleaseMutex(m_hMutex);
		}
        return m_pInstance.get();
	}	
private:
    Singleton(){}
    ~Singleton(){}
    Singleton(const Singleton&);
    Singleton& operator= (const Singleton &);

	static HANDLE			m_hMutex;
    static std::auto_ptr<T> m_pInstance;
};

template <class T>
HANDLE Singleton<T>::m_hMutex = NULL;

template <class T>
std::auto_ptr<T> Singleton<T>::m_pInstance;

#define DECLARE_SINGLETON_CLASS(type) \
	friend class std::auto_ptr<type>; \
    friend class Singleton<type>;

#endif

