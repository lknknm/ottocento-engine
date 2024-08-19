#include "application.hpp"

int main()
{
    HelloTriangleApplication app;
    static HelloTriangleApplication* currentAppInstance = nullptr;

    try { app.run(); }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}