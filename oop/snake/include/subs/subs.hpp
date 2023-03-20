#pragma once

//=========================================================

class Subscriber_on_key
{
    virtual void on_key(int key) = 0;
};

class Subscriber_on_timer
{
    virtual void on_timer() = 0;
};