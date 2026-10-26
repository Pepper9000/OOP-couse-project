#include "Server.h"
#include "ClothingFactory.h"
#include "JsonSerialization.h"
#include "RecommendationEngine.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

namespace smartwardrobe {

namespace {

bool fileExists(const std::string& path) {
    std::ifstream f(path.c_str());
    return f.good();
}

std::string readFileContent(const std::string& path) {
    std::ifstream in(path.c_str(), std::ios::binary);
    if (!in.is_open()) return "";
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

std::string buildHttpResponse(int statusCode, const std::string& statusText,
                             const std::string& contentType, const std::string& body) {
    std::ostringstream res;
    res << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    res << "Content-Type: " << contentType << "\r\n";
    res << "Content-Length: " << body.size() << "\r\n";
    res << "Access-Control-Allow-Origin: *\r\n";
    res << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
    res << "Access-Control-Allow-Headers: Content-Type, Authorization, X-Requested-With\r\n";
    res << "Connection: close\r\n\r\n";
    res << body;
    return res.str();
}

std::string buildJsonResponse(int statusCode, const std::string& statusText, const nlohmann::json& j) {
    return buildHttpResponse(statusCode, statusText, "application/json; charset=utf-8", j.dump());
}

std::string buildErrorResponse(int statusCode, const std::string& statusText, const std::string& message) {
    nlohmann::json j;
    j["error"] = message;
    return buildJsonResponse(statusCode, statusText, j);
}

} // namespace

Server::Server(WardrobeManager& manager, std::string dataFilePath, int port)
    : manager_(manager),
      dataFilePath_(std::move(dataFilePath)),
      port_(port) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
}

Server::~Server() {
    stop();
    WSACleanup();
}

bool Server::initPersistence(std::string* error) {
    if (fileExists(dataFilePath_)) {
        if (!CSVRepository::load(dataFilePath_, manager_, error)) {
            if (error) {
                *error = "Failed loading configured data file: " + *error;
            }
            return false;
        }
        std::cout << "[Server] Loaded " << manager_.size() << " items from " << dataFilePath_ << std::endl;
        return true;
    }

    // If configured data file does not exist, check demo data file
    const std::string demoPath = "backend/data/demo_wardrobe.csv";
    if (fileExists(demoPath)) {
        if (!CSVRepository::load(demoPath, manager_, error)) {
            if (error) {
                *error = "Failed loading demo wardrobe data: " + *error;
            }
            return false;
        }
        std::cout << "[Server] Seeded " << manager_.size() << " items from " << demoPath << std::endl;
        if (!persistWardrobe(error)) {
            if (error) {
                *error = "Failed persisting initial seeded data: " + *error;
            }
            return false;
        }
        return true;
    }

    std::cout << "[Server] Starting with empty wardrobe (no existing or demo data file found)." << std::endl;
    return true;
}

bool Server::persistWardrobe(std::string* error) {
    std::string tempPath = dataFilePath_ + ".tmp";
    if (!CSVRepository::save(manager_, tempPath, error)) {
        return false;
    }
    std::remove(dataFilePath_.c_str());
    if (std::rename(tempPath.c_str(), dataFilePath_.c_str()) != 0) {
        if (error) {
            *error = "Failed replacing destination data file with updated state";
        }
        std::remove(tempPath.c_str());
        return false;
    }
    return true;
}

int Server::assignNextId() const {
    int maxId = 0;
    for (const auto& item : manager_.getItems()) {
        if (item && item->getId() > maxId) {
            maxId = item->getId();
        }
    }
    return maxId + 1;
}

bool Server::run() {
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        std::cerr << "[Server] Failed creating socket: " << WSAGetLastError() << std::endl;
        return false;
    }

    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1
    addr.sin_port = htons(static_cast<u_short>(port_));

    if (bind(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        int err = WSAGetLastError();
        std::cerr << "[Server] Failed to bind to port " << port_ << " (Error " << err << "). Port may be occupied." << std::endl;
        closesocket(s);
        return false;
    }

    if (listen(s, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "[Server] Failed to listen on socket: " << WSAGetLastError() << std::endl;
        closesocket(s);
        return false;
    }

    listenSocket_ = static_cast<uintptr_t>(s);
    running_ = true;

    std::cout << "[Server] Smart Wardrobe HTTP server listening on http://localhost:" << port_ << std::endl;

    while (running_) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(s, &readSet);

        timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 250000; // 250ms timeout for responsive stop()

        int sel = select(0, &readSet, nullptr, nullptr, &tv);
        if (sel == SOCKET_ERROR) {
            if (!running_) break;
            std::cerr << "[Server] Select error: " << WSAGetLastError() << std::endl;
            break;
        }

        if (sel > 0 && FD_ISSET(s, &readSet)) {
            sockaddr_in clientAddr;
            int clientLen = sizeof(clientAddr);
            SOCKET clientSocket = accept(s, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
            if (clientSocket != INVALID_SOCKET) {
                handleClient(static_cast<uintptr_t>(clientSocket));
            }
        }
    }

    if (listenSocket_ != 0) {
        closesocket(static_cast<SOCKET>(listenSocket_));
        listenSocket_ = 0;
    }

    std::cout << "[Server] Stopped listening on port " << port_ << std::endl;
    return true;
}

void Server::stop() {
    running_ = false;
    if (listenSocket_ != 0) {
        closesocket(static_cast<SOCKET>(listenSocket_));
        listenSocket_ = 0;
    }
}

void Server::handleClient(uintptr_t clientSocket) {
    SOCKET s = static_cast<SOCKET>(clientSocket);

    std::string rawRequest;
    char buffer[4096];
    int bytesReceived = 0;

    // Read initial headers
    while ((bytesReceived = recv(s, buffer, sizeof(buffer), 0)) > 0) {
        rawRequest.append(buffer, bytesReceived);
        if (rawRequest.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }

    // Check for Content-Length to read remainder of body if necessary
    auto headerEndPos = rawRequest.find("\r\n\r\n");
    if (headerEndPos != std::string::npos) {
        std::string headers = rawRequest.substr(0, headerEndPos);
        // Case-insensitive search for Content-Length
        std::string headersLower = headers;
        for (char& c : headersLower) c = static_cast<char>(std::tolower(c));
        std::size_t clPos = headersLower.find("content-length:");
        if (clPos != std::string::npos) {
            std::size_t valStart = headersLower.find_first_not_of(" \t", clPos + 15);
            std::size_t valEnd = headersLower.find_first_of("\r\n", valStart);
            if (valStart != std::string::npos) {
                std::size_t len = (valEnd == std::string::npos) ? std::string::npos : (valEnd - valStart);
                std::size_t expectedLen = std::stoul(headersLower.substr(valStart, len));
                std::size_t currentBodyLen = rawRequest.size() - (headerEndPos + 4);
                while (currentBodyLen < expectedLen) {
                    int toRead = static_cast<int>(std::min<std::size_t>(sizeof(buffer), expectedLen - currentBodyLen));
                    int more = recv(s, buffer, toRead, 0);
                    if (more <= 0) break;
                    rawRequest.append(buffer, more);
                    currentBodyLen += more;
                }
            }
        }
    }

    std::string response = processRequest(rawRequest);
    int totalSent = 0;
    int toSend = static_cast<int>(response.size());
    while (totalSent < toSend) {
        int sent = send(s, response.data() + totalSent, toSend - totalSent, 0);
        if (sent <= 0) break;
        totalSent += sent;
    }
    closesocket(s);
}

std::string Server::processRequest(const std::string& rawRequest) {
    if (rawRequest.empty()) {
        return buildErrorResponse(400, "Bad Request", "Empty request");
    }

    std::istringstream reqStream(rawRequest);
    std::string method, path, version;
    reqStream >> method >> path >> version;

    if (method.empty() || path.empty()) {
        return buildErrorResponse(400, "Bad Request", "Malformed request line");
    }

    // CORS preflight handling
    if (method == "OPTIONS") {
        return buildHttpResponse(200, "OK", "text/plain", "");
    }

    // Extract body
    std::string body;
    auto bodyPos = rawRequest.find("\r\n\r\n");
    if (bodyPos != std::string::npos) {
        body = rawRequest.substr(bodyPos + 4);
    }

    // Trim query parameters from path for route dispatch
    std::string routePath = path;
    auto qPos = routePath.find('?');
    if (qPos != std::string::npos) {
        routePath = routePath.substr(0, qPos);
    }

    // 1. Health check
    if (method == "GET" && routePath == "/api/health") {
        nlohmann::json res;
        res["status"] = "ok";
        res["service"] = "Smart Wardrobe Backend";
        res["itemCount"] = manager_.size();
        return buildJsonResponse(200, "OK", res);
    }

    // 2. List clothes
    if (method == "GET" && routePath == "/api/clothes") {
        nlohmann::json res = JsonSerialization::clothingListToJson(manager_.getAllItems());
        return buildJsonResponse(200, "OK", res);
    }

    // 3. Add clothing
    if (method == "POST" && routePath == "/api/clothes") {
        nlohmann::json jBody;
        try {
            jBody = nlohmann::json::parse(body);
        } catch (const std::exception& e) {
            std::cerr << "[Server POST /api/clothes] JSON parse error: " << e.what() << std::endl;
            return buildErrorResponse(400, "Bad Request", "Malformed JSON payload");
        }

        int newId = assignNextId();
        std::string err;
        auto item = JsonSerialization::jsonToClothing(jBody, newId, &err);
        if (!item) {
            return buildErrorResponse(400, "Bad Request", err);
        }

        nlohmann::json itemJson = JsonSerialization::clothingToJson(*item);

        if (!manager_.addItem(std::move(item))) {
            return buildErrorResponse(500, "Internal Server Error", "Failed to add item to wardrobe");
        }

        std::string persistErr;
        if (!persistWardrobe(&persistErr)) {
            manager_.removeItem(newId);
            return buildErrorResponse(500, "Internal Server Error", "Failed persisting wardrobe: " + persistErr);
        }

        return buildJsonResponse(201, "Created", itemJson);
    }

    // 4. Update clothing (PUT /api/clothes/{id})
    if (method == "PUT" && routePath.rfind("/api/clothes/", 0) == 0) {
        std::string idStr = routePath.substr(13);
        int id = 0;
        try {
            id = std::stoi(idStr);
        } catch (...) {
            return buildErrorResponse(400, "Bad Request", "Invalid clothing ID in path");
        }

        if (!manager_.contains(id)) {
            return buildErrorResponse(404, "Not Found", "Item with ID " + std::to_string(id) + " not found");
        }

        nlohmann::json jBody;
        try {
            jBody = nlohmann::json::parse(body);
        } catch (const std::exception& e) {
            std::cerr << "[Server PUT /api/clothes] JSON parse error: " << e.what() << std::endl;
            return buildErrorResponse(400, "Bad Request", "Malformed JSON payload");
        }

        std::string err;
        auto newItem = JsonSerialization::jsonToClothing(jBody, id, &err);
        if (!newItem) {
            return buildErrorResponse(400, "Bad Request", err);
        }

        nlohmann::json updatedJson = JsonSerialization::clothingToJson(*newItem);

        if (!manager_.updateItem(std::move(newItem))) {
            return buildErrorResponse(500, "Internal Server Error", "Failed updating item");
        }

        std::string persistErr;
        if (!persistWardrobe(&persistErr)) {
            return buildErrorResponse(500, "Internal Server Error", "Failed persisting update: " + persistErr);
        }

        return buildJsonResponse(200, "OK", updatedJson);
    }

    // 5. Delete clothing (DELETE /api/clothes/{id})
    if (method == "DELETE" && routePath.rfind("/api/clothes/", 0) == 0) {
        std::string idStr = routePath.substr(13);
        int id = 0;
        try {
            id = std::stoi(idStr);
        } catch (...) {
            return buildErrorResponse(400, "Bad Request", "Invalid clothing ID in path");
        }

        if (!manager_.contains(id)) {
            return buildErrorResponse(404, "Not Found", "Item with ID " + std::to_string(id) + " not found");
        }

        if (!manager_.removeItem(id)) {
            return buildErrorResponse(500, "Internal Server Error", "Failed removing item");
        }

        std::string persistErr;
        if (!persistWardrobe(&persistErr)) {
            return buildErrorResponse(500, "Internal Server Error", "Failed persisting deletion: " + persistErr);
        }

        nlohmann::json res;
        res["success"] = true;
        res["deleted"] = id;
        return buildJsonResponse(200, "OK", res);
    }

    // 6. Generate outfits (POST /api/outfits/generate)
    if (method == "POST" && routePath == "/api/outfits/generate") {
        nlohmann::json jBody;
        try {
            if (!body.empty()) {
                jBody = nlohmann::json::parse(body);
            } else {
                jBody = nlohmann::json::object();
            }
        } catch (...) {
            return buildErrorResponse(400, "Bad Request", "Malformed JSON payload");
        }

        RecommendationCriteria criteria = JsonSerialization::parseCriteria(jBody);
        std::vector<Outfit> outfits = RecommendationEngine::recommend(manager_, criteria);

        nlohmann::json res;
        res["outfits"] = JsonSerialization::outfitsToJson(outfits);
        return buildJsonResponse(200, "OK", res);
    }

    // 7. Static file serving (frontend direct access)
    if (method == "GET") {
        if (routePath == "/" || routePath == "/index.html") {
            std::string content = readFileContent("index.html");
            if (!content.empty()) {
                return buildHttpResponse(200, "OK", "text/html; charset=utf-8", content);
            }
        } else if (routePath == "/style.css") {
            std::string content = readFileContent("style.css");
            if (!content.empty()) {
                return buildHttpResponse(200, "OK", "text/css; charset=utf-8", content);
            }
        } else if (routePath == "/app.js") {
            std::string content = readFileContent("app.js");
            if (!content.empty()) {
                return buildHttpResponse(200, "OK", "application/javascript; charset=utf-8", content);
            }
        }
    }

    return buildErrorResponse(404, "Not Found", "Endpoint not found: " + method + " " + path);
}

} // namespace smartwardrobe
