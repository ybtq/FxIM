#ifndef _SINGLETON_H_
#define _SINGLETON_H_

#include <memory>

template<typename T>
class Singleton
{
public:
    static T* getInstance()
	{
        if (_instance.get() == NULL)
        {
			if (_mutex == NULL)
			{
				_mutex = CreateMutex (NULL, FALSE, NULL);
			}
			WaitForSingleObject(_mutex, INFINITE);
            if (_instance.get() == NULL)
                _instance.reset(new T);
            ReleaseMutex(_mutex);
		}
        return _instance.get();
	}	
private:
    Singleton(){}
    ~Singleton(){}
    Singleton(const Singleton&);
    Singleton& operator= (const Singleton &);

    static std::auto_ptr<T> _instance;
    static HANDLE _mutex;
};

template <class T>
std::auto_ptr<T> Singleton<T>::_instance;

template <class T>
HANDLE Singleton<T>::_mutex = NULL;

#define DECLARE_SINGLETON_CLASS(type) \
	friend class std::auto_ptr<type>; \
    friend class Singleton<type>;

#endif

