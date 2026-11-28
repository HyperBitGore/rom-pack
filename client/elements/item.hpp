#pragma once

// how do I implement text inputs tho?
class item {
    public:
    virtual ~item() = default;
    item () = default;
    virtual void render () = 0;
};