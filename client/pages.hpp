#pragma once
#include "elements/elements.hpp"
#include <memory>
#include "database.hpp"

class pages {
    public:
    static std::unique_ptr<elements> constructGameDetailsPage (game game);
    static std::unique_ptr<elements> constructGameListPage ();
    static std::unique_ptr<elements> constructExportGamesPage (); // TODO
};