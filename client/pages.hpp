#pragma once
#include "elements/elements.hpp"
#include <memory>
#include "database.hpp"

class pages {
    public:
    static std::unique_ptr<elements> constructMainPage ();
    static std::unique_ptr<elements> constructGameDetailsPage (game game);
    static std::unique_ptr<elements> constructAddGamePage (); // TODO switch from popup to this
    static std::unique_ptr<elements> constructGameListPage ();
};