#include <Residue.h>

int main(int argc, char** argv) {
    Residue::setApplicationArgs(argc, argv);

    // You can use config file as well
    Residue::AccessCodeMap accessCodes = {
        {
            "sample-app", "a2dcb"
        }
    };

    Residue::connect(&accessCodes);

    std::cout << "Run \"tail -f /tmp/logs/sample-app.log\" to keep eye on output.\nPress Ctrl+C to exit\n" << std::endl;

    while (true) {
        std::wstring input;
        std::cout << "Type > ";
        std::getline(std::wcin, input);
        if (!input.empty()) {
            std::wcout << L"Logging <" << input << L">\n";
            CLOG(INFO, "sample-app") << input;
            // Or you can use LOG(INFO) but define ELPP_DEFAULT_LOGGER
            // see https://github.com/muflihun/residue/blob/master/samples/clients/c%2B%2B/simple/src/log.h
        }
    }

    Residue::disconnect();

    return 0;
}
