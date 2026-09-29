#pragma once

class page {
    public:
    virtual void render () = 0;
    virtual void registerclick (float x, float y) = 0;

    private:


};