#ifndef __SINGLETON_H__
#define __SINGLETON_H__

template <typename T>
class Singleton {
public:
  static T* GetInstance() {
    if (instance_ == NULL) {
      instance_ = new T();
    }
    return instance_;
  }

protected:
  Singleton() {}
  virtual ~Singleton() {}

  static T* instance_;

private:
  Singleton(const Singleton&); 
  Singleton& operator =(const Singleton&);
};

template<typename T> T* Singleton<T>::instance_ = NULL;

#endif // __SINGLETON_H__
