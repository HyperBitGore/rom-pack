#pragma once

// how do I implement text inputs tho?
class item {
    public:
    virtual ~item() = default;
    item () = default;
    virtual bool onclick (float x, float y) = 0;
    virtual void render () = 0;
    virtual bool onkeydown (int key) = 0;
};