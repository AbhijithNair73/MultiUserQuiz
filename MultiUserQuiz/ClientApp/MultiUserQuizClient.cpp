#include "ClientConnMgr.hpp"

int main ()
{
    try {

        ClientConnMgr::ConnectToServer ();

    } catch (const std::exception & e) {

        std::cerr << "Exception: " << e.what () << std::endl;
        return -1;
    }
    return 0;
}
