<p align="center">
   ﷽
</p>

# Demo Server
Residue server is running on http://residue-demo.muflihun.com to demonstrate how it all works. This server is open and non-restrictive. This means, it can:

 * Accept plain connections
 * Accept plain log requests
 * Allow unknown clients and loggers
 * Logs are streamed on browser using [`resitail`](https://www.npmjs.com/package/resitail)
 * Streamed logs are not using https
 
Resitail is currently under development but basic `tail -f` functionality can be seen and filtered for clients by going to http://residue-demo.muflihun.com/?clientId=muflihun00102030

## Node.js Sample Client
The most simple demo client can help you understand on how it all works.

 * Download [Node.js sample app](https://github.com/muflihun/residue-node/tree/master/samples/node) code
 * Install the package using `npm install`
 * Change `client.conf.json` `url` from `localhost:8777` to `residue-demo.muflihun.com:8777`
 * Start the app using `node app.js`
 * Open [Residue logs streaming](http://residue-demo.muflihun.com/?clientId=muflihun00102030)
 * Now open http://localhost:3009 and check the residue logs streaming tab on your browser
 
## Summary
This is only a quick demo. We will extend this demo as we work our way through `resitail`.


