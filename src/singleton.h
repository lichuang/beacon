#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include <pthread.h>

template <typename T>
class Singleton {
public:
  static T* GetInstance() {
    pthread_once(&ponce_, &Singleton::init);
    return instance_;
  }

private:
  static void init() {
    if (instance_ == NULL) {
      instance_ = new T();
    }
  }
protected:
  Singleton() {}
  virtual ~Singleton() {}

  static T* instance_;

private:
  static pthread_once_t ponce_;
  Singleton(const Singleton&); 
  Singleton& operator =(const Singleton&);
};

template<typename T> T* Singleton<T>::instance_ = NULL;
template<typename T> pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

#endif // __SINGLETON_H__
