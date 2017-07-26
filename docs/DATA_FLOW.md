                                       ‫بسم الله الرَّحْمَنِ الرَّحِيمِ

## Data Flow

Following diagram shows you data flow in from residue server to client library. Note, under "obtain token" section, developer is not shown, this means developer has no role in obtaining new token.

[![dataflow_diagram]](https://raw.githubusercontent.com/muflihun/residue/master/docs/data-flow.jpg)

* Token requires access code for logger as per your configurations
* Log request is ignored for following reasons:
  * Logger needs token and token not valid or not found (in request)
  * Logger is blacklisted (see [CONFIGURATION.md](/docs/CONFIGURATION.md) documentation)
  * `residue` logger that is explicitely for the server use
* Log server returns response code `0` (SUCCESS) if it was successfully received. Response code does not suggest if log was written successfully as this process is asyncronous.

  [dataflow_diagram]: https://raw.githubusercontent.com/muflihun/residue/master/docs/data-flow.jpg?
