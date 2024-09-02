#include "application.hpp"
#include "camera.hpp"

int main()
{
    otrApplication app;

    try { app.run(); }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}