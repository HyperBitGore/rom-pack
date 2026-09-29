#pragma once

// how do I implement text inputs tho?
class item {
    public:
    virtual ~item() = default;
    item () = delete;
    virtual bool onclick (float x, float y) = 0;
    virtual void render () = 0;
    virtual bool onkeydown (int key) = 0;
};