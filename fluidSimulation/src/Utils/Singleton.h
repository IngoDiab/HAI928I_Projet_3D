#pragma once

template<typename UserClass>
class Singleton
{
    static UserClass* mInstance;

public:
    static UserClass* Instance(){return mInstance;};

    Singleton()
    {
        if(!mInstance) mInstance = (UserClass*)this;
        else delete this;
    }

public:
        Singleton(UserClass const&) = delete;
        void operator=(Singleton const&) = delete;
};

template<typename UserClass>
UserClass* Singleton<UserClass>::mInstance = nullptr;