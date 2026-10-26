#pragma once

#include "CSVRepository.h"
#include "WardrobeManager.h"

#include <atomic>
#include <cstdint>
#include <string>

namespace smartwardrobe {

/**
 * Local HTTP REST server serving the Smart Wardrobe API and frontend.
 *
 * Listens on loopback (127.0.0.1:18080) by default.
 * Provides CORS headers, robust JSON error handling, automatic demo seeding,
 * and reliable persistence across mutations and restarts.
 */
class Server {
public:
    /**
     * @param manager Reference to in-memory WardrobeManager.
     * @param dataFilePath Path to persistent CSV data file.
     * @param port TCP port to bind (default 18080).
     */
    Server(WardrobeManager& manager, std::string dataFilePath, int port = 18080);
    ~Server();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    /**
     * Initializes storage by loading dataFilePath if it exists,
     * or seeding from demo_wardrobe.csv if dataFilePath does not yet exist.
     *
     * @param error Optional pointer to receive error description on failure.
     * @return true if initialized, false if existing file is corrupted.
     */
    bool initPersistence(std::string* error = nullptr);

    /**
     * Starts listening and serving requests. Blocks until stop() is called.
     *
     * @return true if started and exited cleanly, false if socket binding failed.
     */
    bool run();

    /**
     * Signals the server to stop listening and close active sockets.
     */
    void stop();

    int getPort() const { return port_; }
    const std::string& getDataFilePath() const { return dataFilePath_; }
    bool isRunning() const { return running_; }

private:
    WardrobeManager& manager_;
    std::string dataFilePath_;
    int port_;
    std::atomic<bool> running_{false};
    uintptr_t listenSocket_{0};

    bool persistWardrobe(std::string* error = nullptr);
    int assignNextId() const;

    void handleClient(uintptr_t clientSocket);
    std::string processRequest(const std::string& rawRequest);
};

} // namespace smartwardrobe
