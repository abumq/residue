<p align=center>
   ﷽
</p>

[🏠 Configurations](/docs/CONFIGURATION.md)

# compression
[Boolean] Specifies whether compression is enabled or not.

You should note few points:

 * If this is enabled client libraries should take advantage of it and send compressed data. However, if it doesn't, request is still processed.
 * Only log requests should be compressed, other requests (connection, touch etc) are sent normally.
 * Outgoing data (from server) is never compressed.

Compression has great affect and can save big data. We recommend you to enable compression in your server. Just to give you little bit of idea, when we run [simple example project](https://github.com/muflihun/residue-cpp/tree/master/samples/detailed-cmake) to log 1246 requests, without compression data transferred was `488669 bytes` and with compression it was `44509 bytes`. Same data transferred has same performance with high reliability.

It always has very good performance when you have [`compression`](#compression) and [`allow_bulk_log_request`](#allow_bulk_log_request) both active. In one of our big test we had following result:

 * With compression and bulk (size: 20): `24100423 bytes`
 * Without compression and bulk (size: 20): `327100683 bytes` (more than 92.6% of less-data transferred)
 * Without compression and non-bulk request: `398001463 bytes`

This is because lossless-compression is done on similar bytes. If you wish to know more about compression algorithm see [gzlib algorithm](www.gzip.org/algorithm.txt).

Default: `true`

[🏠 Configurations](/docs/CONFIGURATION.md)

