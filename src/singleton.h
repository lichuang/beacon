#ifndef __SINGLETON_H__
#define __SINGLETON_H__

template <typename T>
class Singleton {
public:
  static T* GetInstance() {
    return instance_;
  }

  static void Init() {
    instance_ = new T();
  }

protected:
  Singleton() : instance_(NULL) {}
  virtual ~Single() {}

  static T* instance_;

private:
  Singleton(const Singleton&); 
  Singleton& operator =(const Singleton&);
};

#endif // __SINGLETON_H__
