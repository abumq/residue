//
//  Ripe tool
//
//  Copyright © 2017 Muflihun.com. All rights reserved.
//

#include <iomanip>
#include <cstring>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "include/Ripe.h"

void displayVersion()
{
    std::cout << "Ripe - Lightweight cryptography library wrapper" << std::endl << "Version: " << RIPE_VERSION << std::endl << "https://muflihun.com" << std::endl;
}

void displayUsage()
{
    // we want to keep the order so don't use std::map or std::unordered_map
    std::vector<std::pair<std::string, std::string> > options;
    options.push_back(std::make_pair("--version", "Display version information"));
    options.push_back(std::make_pair("-g", "Generate key"));
    options.push_back(std::make_pair("-e", "Encrypt / encode / inflate the data"));
    options.push_back(std::make_pair("-d", "Decrypt / decrypt / deflate the data"));
    options.push_back(std::make_pair("-s", "Sign the data"));
    options.push_back(std::make_pair("-v", "Verify the signed data"));
    options.push_back(std::make_pair("--aes", "Generate AES key (requires -g)"));
    options.push_back(std::make_pair("--key", "Symmetric key for encryption / decryption"));
    options.push_back(std::make_pair("--in-key", "Symmetric key for encryption / decryption file path"));
    options.push_back(std::make_pair("--iv", "Initializaion vector for decription"));
    options.push_back(std::make_pair("--rsa", "Use RSA encryption/decryption"));
    options.push_back(std::make_pair("--zlib", "ZLib compression/decompression"));
    options.push_back(std::make_pair("--raw", "Raw output for rsa encrypted data"));
    options.push_back(std::make_pair("--base64", "Tells ripe the data needs to be decoded before decryption (this can be used for decoding base64)"));
    options.push_back(std::make_pair("--hex", "Tells ripe the data is hex string"));
    options.push_back(std::make_pair("--clean", "(Only applicable when --base64 data provided) Tells ripe to clean the data before processing"));
    options.push_back(std::make_pair("--in", "Input file. You can also pipe in the data. In that case you do not have to provide this parameter"));
    options.push_back(std::make_pair("--out", "Tells ripe to store encrypted data in specified file. (Outputs IV in console)"));
    options.push_back(std::make_pair("--length", "Specify key length"));
    options.push_back(std::make_pair("--secret", "Secret key for encrypted private key (RSA only)"));

    displayVersion();
    std::cout << "Usage: " << std::endl;
    std::cout << "ripe [-d | -e | -g | -s | -v] [--in <input_file_path>] [--key <key>] [--in-key <file_path>] [--out-public <output_file_path>] [--out-private <output_file_path>] [--iv <init vector>] [--base64] [--rsa] [--length <key_length>] [--out <output_file_path>] [--clean] [--aes [<key_length>]] [--secret] [--hex] [--signature]" << std::endl;
    std::cout << std::endl;
    const std::size_t LONGEST = 20;
    for (std::vector<std::pair<std::string, std::string> >::iterator it = options.begin();
         it < options.end(); ++it) {
        std::pair<std::string, std::string> option = *it;
        std::cout << "     " << option.first;
        for (std::size_t i = 0; i < LONGEST - option.first.length(); ++i) {
            std::cout << " ";
        }
        std::cout << option.second << std::endl;
    }
    std::cout << std::endl;
}

#define TRY try {
#define CATCH }  catch (const std::exception& e) { std::cout << "ERROR: " << e.what() << std::endl; }

void encryptAES(std::string& data, const std::string& key,
                const std::string& iv, const std::string& clientId,
                const std::string& outputFile)
{
    TRY
        std::cout << Ripe::encryptAES(data, key, clientId, outputFile, iv);
    CATCH
}

void decryptAES(std::string& data, const std::string& key,
                std::string& iv, bool isBase64, bool isHex)
{
    TRY
        std::cout << Ripe::decryptAES(data, key, iv, isBase64, isHex);
    CATCH
}

void generateAESKey(int length)
{
    if (length == 0 || length == 2048) {
        std::cout << "ERROR: Please provide valid key length" << std::endl;
        return;
    }
    TRY
        std::cout << Ripe::generateNewKey(length / 8);
    CATCH
}

void encodeBase64(std::string& data)
{
    TRY
        std::cout << Ripe::base64Encode(data);
    CATCH
}

void decodeBase64(std::string& data)
{
    TRY
        std::cout << Ripe::base64Decode(data);
    CATCH
}

void encodeHex(std::string& data)
{
    TRY
        std::cout << Ripe::stringToHex(data);
    CATCH
}

void decodeHex(std::string& data)
{
    TRY
        std::cout << Ripe::hexToString(data);
    CATCH
}

void compress(std::string& data, bool isBase64, bool isHex,
              const std::string& outputFile)
{
    TRY
        std::string o = Ripe::compressString(data);

        if (isHex) {
            o = Ripe::stringToHex(o);
        }
        if (isBase64) {
            o = Ripe::base64Encode(o);
        }
        if (outputFile.empty()) {
            std::cout << o;
        } else {
            std::ofstream out(outputFile.c_str());
            out << o;
            out.close();
        }
    CATCH
}

void decompress(std::string& data, bool isBase64, bool isHex,
                const std::string& outputFile)
{
    TRY
        if (isBase64) {
            data = Ripe::base64Decode(data);
        }
        if (isHex) {
            data = Ripe::hexToString(data);
        }
        std::string o = Ripe::decompressString(data);
        if (outputFile.empty()) {
            std::cout << o;
        } else {
            std::ofstream out(outputFile.c_str());
            out << o;
            out.close();
        }
    CATCH
}

void encryptRSA(std::string& data, const std::string& key,
                const std::string& outputFile, bool isRaw)
{
    TRY
        std::cout << Ripe::encryptRSA(data, key, outputFile, isRaw);
    CATCH
}

void decryptRSA(std::string& data, const std::string& key,
                bool isBase64, bool isHex, const std::string& secret)
{
    TRY
        std::cout << Ripe::decryptRSA(data, key, isBase64, isHex, secret);
    CATCH
}

void signRSA(std::string& data, const std::string& key,
             const std::string& keySecret)
{
    TRY
        std::cout << Ripe::signRSA(data, key, keySecret);
    CATCH
}

void verifyRSA(std::string& data, const std::string& signature,
               const std::string& key)
{
    TRY
        std::cout << (Ripe::verifyRSA(data, signature, key) ? "OK" : "FAIL");
    CATCH
}

void writeRSAKeyPair(const std::string& publicFile,
                     const std::string& privateFile, std::size_t length,
                     const std::string& secret)
{
    TRY
       std::cout << "Generating key pair that can encrypt " << Ripe::maxRSABlockSize(length) << " Ripebytes" << std::endl;
        if (Ripe::writeRSAKeyPair(publicFile, privateFile, length, secret)) {
            std::cout << "Successfully saved!" << std::endl;
        }
    CATCH
}

void generateRSAKeyPair(std::size_t length, const std::string& secret)
{
    TRY
        std::cout << Ripe::generateRSAKeyPairBase64(length, secret);
    CATCH
}

bool rtrimPred(char c) {
    return !std::isspace(c);
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        displayUsage();
        return 1;
    }

    if (strcmp(argv[1], "--version") == 0) {
        displayVersion();
        return 0;
    }

    // This is quick check for args, use getopt in future
    int type = -1; // Decryption encryption, generate, verify or sign

    std::string key;
    std::string publicKeyFile;
    std::string privateKeyFile;
    std::string iv;
    int keyLength = 2048;
    std::string data;
    std::string clientId;
    std::string secret;
    std::string signatureHex;
    bool isAES = false;
    bool isZlib = false;
    bool isBase64 = false;
    bool isHex = false;
    bool clean = false;
    bool isRSA = false;
    bool isRaw = false;
    std::string outputFile;
    bool fileArgSpecified = false;

    for (int i = 0; i < argc; i++) {
        std::string arg(argv[i]);
        bool hasNext = i + 1 < argc;
        if (arg == "-d" && type == -1) {
            type = 1;
        } else if (arg == "-e" && type == -1) {
            type = 2;
        } else if (arg == "-g" && type == -1) {
            type = 3;
        } else if (arg == "-s" && type == -1) {
            type = 4;
        } else if (arg == "-v" && type == -1) {
            type = 5;
        } else if (arg == "--base64") {
            isBase64 = true;
        } else if (arg == "--hex") {
            isHex = true;
        } else if (arg == "--rsa") {
            isRSA = true;
        } else if (arg == "--zlib") {
            isZlib = true;
        } else if (arg == "--signature" && hasNext) {
            signatureHex = argv[++i];
        } else if (arg == "--raw") {
            isRaw = true;
        } else if (arg == "--key" && hasNext) {
            key = argv[++i];
        } else if (arg == "--aes") {
            isAES = true;
            if (i + 1 < argc) {
                int k = atoi(argv[++i]);
                if (k > 0) {
                    keyLength = k;
                } else {
                    --i;
                }
            }
        } else if (arg == "--length" && hasNext) {
            keyLength = atoi(argv[++i]);
        } else if (arg == "--secret" && hasNext) {
            secret = argv[++i];
        } else if (arg == "--clean") {
            clean = true;
        } else if (arg == "--out-public" && hasNext) {
            publicKeyFile = argv[++i];
        } else if (arg == "--out-private" && hasNext) {
            privateKeyFile = argv[++i];
        } else if (arg == "--in-key" && hasNext) {
            std::fstream fs;
            // Do not increment i here as we are only changing 'data'
            fs.open (argv[i + 1], std::fstream::binary | std::fstream::in);
            key = std::string((std::istreambuf_iterator<char>(fs)),
                            (std::istreambuf_iterator<char>()));
            fs.close();
        } else if (arg == "--out" && hasNext) {
            outputFile = argv[++i];
        } else if (arg == "--iv" && hasNext) {
            iv = argv[++i];
        } else if (arg == "--client-id" && hasNext) {
            clientId = argv[++i];
        } else if (arg == "--in" && hasNext) {
            fileArgSpecified = true;
            std::fstream fs;
            // Do not increment i here as we are only changing 'data'
            fs.open (argv[i + 1], std::fstream::binary | std::fstream::in);
            data = std::string((std::istreambuf_iterator<char>(fs) ),
                            (std::istreambuf_iterator<char>()));
            fs.close();
        }
    }

    if ((type == 1 || type == 2 || type == 4 || type == 5) && !fileArgSpecified) {
        std::stringstream ss;
        for (std::string line; std::getline(std::cin, line);) {
            ss << line << std::endl;
        }
        data = ss.str();
        // Remove last 'new line'
        data.erase(data.size() - 1);
    }

    if ((isBase64 || isHex) && clean) {
        // Right trim
        data.erase(std::find_if(data.rbegin(), data.rend(), rtrimPred).base(), data.end());
    }
    if (type == 1) { // Decrypt / Decode
        if (isBase64 && key.empty() && iv.empty() && !isZlib) {
            // base64 decode
            decodeBase64(data);
        } else if (isHex && key.empty() && iv.empty() && !isZlib) {
            // hex to ascii
            decodeHex(data);
        } else if (isZlib) {
            decompress(data, isBase64, isHex, outputFile);
        } else if (isRSA) {
            // RSA decrypt (base64-flexible)
            decryptRSA(data, key, isBase64, isHex, secret);
        } else {
            // AES decrypt (base64-flexible)
            decryptAES(data, key, iv, isBase64, isHex);
        }
    } else if (type == 2) { // Encrypt / Encode
        if (isBase64 && key.empty() && iv.empty() && !isZlib) {
            encodeBase64(data);
        } else if (isHex && key.empty() && iv.empty() && !isZlib) {
            encodeHex(data);
        } else if (isZlib) {
            compress(data, isBase64, isHex, outputFile);
        } else if (isRSA) {
            encryptRSA(data, key, outputFile, isRaw);
        } else {
            encryptAES(data, key, iv, clientId, outputFile);
        }
    } else if (type == 3) { // Generate
        if (isRSA) {
            if (publicKeyFile.empty() && privateKeyFile.empty()) {
                if (!secret.empty() && secret.size() < 4) {
                    std::cerr << "ERROR: Please choose secret of at least 4 characters for it to be compatible with other decryption tools" << std::endl;
                } else {
                    generateRSAKeyPair(keyLength, secret);
                }
            } else if (publicKeyFile.empty() || privateKeyFile.empty()) {
                std::cerr << "ERROR: Please provide both private and public key files [out-public] and [out-private]" << std::endl;
            } else {
                if (!secret.empty() && secret.size() < 4) {
                    std::cerr << "ERROR: Please choose secret of at least 4 characters for it to be compatible with other decryption tools" << std::endl;
                } else {
                    writeRSAKeyPair(publicKeyFile, privateKeyFile, keyLength, secret);
                }
            }
        } else if (isAES) {
            generateAESKey(keyLength);
        } else {
            std::cerr << "ERROR: Please provide method (you probably forgot '--rsa' or '--aes')" << std::endl;
        }
    } else if (type == 4) { // Sign RSA
        if (key.empty()) {
            std::cerr << "ERROR: Please provide private key to sign the data with" << std::endl;
        } else {
            signRSA(data, key, secret);
        }
    } else if (type == 5) { // Verify RSA
        if (key.empty()) {
            std::cerr << "ERROR: Please provide public key to verify the data with" << std::endl;
        } if (signatureHex.empty()) {
            std::cerr << "ERROR: Please provide signature (in hex format)" << std::endl;
        } else {
            verifyRSA(data, signatureHex, key);
        }
    } else {
        displayUsage();
        return 1;
    }

    return 0;
}
