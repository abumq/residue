//
//  clients.cc
//  Residue
//
//  Copyright 2017-present Muflihun Labs
//
//  Author: @abumusamq
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#include "src/cli/clients.h"
#include "src/core/registry.h"
#include "src/core/configuration.h"
#include "src/utils/utils.h"
#include "src/tasks/client-integrity-task.h"

using namespace residue;

Clients::Clients(Registry* registry) :
    Command("clients",
            "List, add or remove clients from the server configuration",
            "clients [list] [add --client-id <id> --rsa-public-key-file <rsa_key>] [remove --client-id <client-id>] [clean]",
            registry)
{
}

void Clients::execute(std::vector<std::string>&& params, std::ostringstream& result, bool ignoreConfirmation) const
{
    if (params.empty()) {
        result << "Clients: " << registry()->clients().size() << std::endl;
    }
    if (hasParam(params, "list")) {
        list(result);
    } else if (hasParam(params, "clean")) {
        if (registry()->clientIntegrityTask()->isExecuting()) {
            result << "\nAlready running, please try again later" << std::endl;;
            return;
        }

        if (ignoreConfirmation || getConfirmation("This will run client integrity task and clean the expired clients")) {
            registry()->clientIntegrityTask()->kickOff();
            result << "\nFinished client integrity task" << std::endl;
        }
    }else if (hasParam(params, "remove")) {
        const std::string clientId = getParamValue(params, "--client-id");
        if (clientId.empty()) {
            result << "\nNo client ID provided" << std::endl;
            return;
        }
        Client* client = registry()->findClient(clientId);
        if (client == nullptr) {
            result << "\nClient not found [" << clientId << "]" << std::endl;
            return;
        }
        if (ignoreConfirmation || getConfirmation("It can be extremely dangerous to remove the client. "
                                                  "Your server may crash!!1 Just be sure what you're doing.")) {
            registry()->removeClient(client);
        }
    } else if (hasParam(params, "add")) {
        const std::string clientId = getParamValue(params, "--client-id");
        const std::string rsaPublicKey = getParamValue(params, "--rsa-public-key-file");
        if (clientId.empty() || rsaPublicKey.empty()) {
            result << "\nNo client ID provided" << std::endl;
            return;
        }
        if (rsaPublicKey.empty()) {
            result << "\nNo public key provided" << std::endl;
            return;
        }
        Client* client = registry()->findClient(clientId);
        if (client != nullptr) {
            result << "\nClient already exists [" << clientId << "]" << std::endl;
            return;
        }
        if (Utils::fileExists(rsaPublicKey.c_str())) {
            if (Utils::isAlphaNumeric(clientId, "-_@#")) {
                bool cont = registry()->configuration()->addKnownClient(clientId, rsaPublicKey);
                cont = cont && registry()->configuration()->save(registry()->configuration()->configurationFile());
                result << (cont ? "Done" : "Failed") << std::endl;
            } else {
                result << "Invalid character in client ID, should be alpha-numeric "
                           "(can also include these characters excluding square brackets: [_@-#])" << std::endl;
            }
        } else {
            result << "Public key file does not exist on the server" << std::endl;
        }
    }
}

void Clients::list(std::ostringstream& result) const
{
    int i = 1;
    for (auto& c : registry()->clients()) {
        result << (i++) << " > " << c.second.id()
                  << ", Age: " << (Utils::now() - c.second.dateCreated()) << "s, Status: "
                  << (!c.second.isAlive() ? "DEAD" : "ALIVE " + std::to_string(c.second.age() - (Utils::now() - c.second.dateCreated())) + "s")
                  << ", Key: " << c.second.key();
    }
}
