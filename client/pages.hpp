#pragma once
#include "elements/elements.hpp"
#include <memory>

class pages {
    public:
    static std::unique_ptr<elements> constructLoginPage (std::string address);
    static std::unique_ptr<elements> constructMainPage ();
    static std::unique_ptr<elements> constructGameDetailsPage ();
    static std::unique_ptr<elements> constructAddGamePage ();
    static std::unique_ptr<elements> constructLocalSelectionPage ();
    static std::unique_ptr<elements> constructServerConnectPage ();
};