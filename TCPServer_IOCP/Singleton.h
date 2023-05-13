#pragma once
#ifndef SINGLETON_H
#define SINGLETON_H

template <class T>
class Singleton {
private:
	static T* instance_;   //��̬���Աʵ���ӳٴ���
	Singleton(void);		   //˽�й��캯��

public:
	static T* GetInstance(void);
	static void Close(void);
};

//ģ����static���� �������������ⶨ��
template <class T>
T* Singleton<T>::instance_ = nullptr;

//static T* getInstance()����ʵ��
template <class T>
T* Singleton<T>::GetInstance(void)
{
	static std::once_flag oc;  //����call_once�ľֲ���̬����
	std::call_once(oc,[&](void)
	{
		instance_ = new T();
	});
	return instance_;
}


//static void close()����ʵ��
template<class T>
void Singleton<T>::Close(void)
{
	if (instance_)
	{
		delete instance_;
	}
}

#endif // !SINGLETON
