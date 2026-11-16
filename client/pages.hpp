#pragma once
#include "elements/elements.hpp"
#include <memory>

class pages {
    public:
    static std::unique_ptr<elements> constructMainPage ();
    static std::unique_ptr<elements> constructGameDetailsPage ();
    static std::unique_ptr<elements> constructAddGamePage ();
};