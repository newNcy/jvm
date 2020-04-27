#pragma 


template <typename T>
class singleton
{
	private:
		singleton (const singleton & ) = delete;
		singleton (singleton && ) = delete;
	protected:
		singleton () {};
	public:
		static T * the_instance;
		static T & instance()
		{
			if (!the_instance) the_instance = new T;
			return *the_instance;
		}
};

template <typename T>
T * singleton<T>::the_instance = nullptr;
