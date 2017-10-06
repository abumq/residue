<p align="center">
   ﷽
</p>

# Demo Server
Residue server is hosted on http://residue-demo.muflihun.com to demonstrate how it all works.

This machine has two servers running, one is open and non-restrictive and other is closed server.

## Non-Restrictive Server
This server can be connected via port 8777. This means, it can:
 
 * Accept plain connections
 * Accept plain log requests
 * Allow unknown clients and loggers
 
You can stream these logs on your browser via http://residue-demo.muflihun.com:8775/?clientId=muflihun00102030

## Closed Server
This server can be connected via port 8772.

 * You will need [server public key](/tools/closed-server/server.pub) to access this server
 * Unknown clients are not allowed and there is only one client registered (client_id: `client-1`, ([private key](/tools/closed-server/client-1.pem)))
 * Unknown loggers are allowed

You can stream these logs on your browser via http://residue-demo.muflihun.com:8770/?clientId=client-1

## Node.js Sample Client
The most simple demo client can help you understand on how it all works.

 * Download [Node.js sample app](https://github.com/muflihun/residue-node/tree/master/samples/node) code
 * Install the package using `npm install`
 * Start the app using `node app.js`
 * [Stream the logs](http://residue-demo.muflihun.com:8775/?clientId=muflihun00102030)
 * Now open http://localhost:3009 and check the residue logs streaming tab on your browser
 
## Summary
The streaming UI uses [`resitail`](https://www.npmjs.com/package/resitail) which is under development but has enough functionality to show you how residue works and what it does. Currently, it does not support HTTPS, filtering or searching but it's irrelevant to the residue server.


