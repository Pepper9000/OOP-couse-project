#include "Server.h"
#include "WardrobeManager.h"

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>

static smartwardrobe::Server* g_serverInstance = nullptr;

void signalHandler(int signum) {
    std::cout << "\n[Main] Received signal " << signum << ", shutting down server..." << std::endl;
    if (g_serverInstance) {
        g_serverInstance->stop();
    }
}

int main(int argc, char* argv[]) {
    int port = 18080;
    std::string dataPath = "backend/data/wardrobe.csv";

    // Check environment variable for data file override
    const char* envDataPath = std::getenv("WARDROBE_DATA_FILE");
    if (envDataPath && *envDataPath) {
        dataPath = envDataPath;
    }

    // CLI arguments: [port] [data_file_path]
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (...) {
            std::cerr << "Invalid port argument '" << argv[1] << "', using default 18080." << std::endl;
        }
    }
    if (argc > 2) {
        dataPath = argv[2];
    }

    std::cout << "==================================================" << std::endl;
    std::cout << "          Smart Wardrobe - Web Application        " << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "Port:            " << port << std::endl;
    std::cout << "Data File:       " << dataPath << std::endl;
    std::cout << "Web Application: http://localhost:" << port << "/" << std::endl;
    std::cout << "REST API Base:   http://localhost:" << port << "/api" << std::endl;
    std::cout << "==================================================" << std::endl;

    smartwardrobe::WardrobeManager manager;
    smartwardrobe::Server server(manager, dataPath, port);
    g_serverInstance = &server;

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::string initError;
    if (!server.initPersistence(&initError)) {
        std::cerr << "[Main] Failed to initialize wardrobe persistence: " << initError << std::endl;
        return 1;
    }

    if (!server.run()) {
        std::cerr << "[Main] Server run failed." << std::endl;
        return 1;
    }

    std::cout << "[Main] Server exited cleanly." << std::endl;
    return 0;
}
