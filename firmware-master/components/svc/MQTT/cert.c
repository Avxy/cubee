/* This is the CA certificate for the CA trust chain of
   www.howsmyssl.com in PEM format, as dumped via:

   openssl s_client -showcerts -connect mqtt.thingspeak.com:443 </dev/null

   The CA cert is the last cert in the chain output by the server.
*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/*
 1 s:/C=US/O=Let's Encrypt/CN=Let's Encrypt Authority X3
   i:/O=Digital Signature Trust Co./CN=DST Root CA X3
 */
const char *server_root_cert = "-----BEGIN CERTIFICATE-----\r\n"
"MIIC9jCCAd6gAwIBAgIBATANBgkqhkiG9w0BAQsFADATMREwDwYDVQQDDAhNeVRl\r\n"
"c3RDQTAeFw0xNjEwMzEwNzE1NDVaFw0xNzEwMzEwNzE1NDVaMDkxJjAkBgNVBAMT\r\n"
"HWRlbmdoYWlndWlkZU1hY0Jvb2stQWlyLmxvY2FsMQ8wDQYDVQQKEwZzZXJ2ZXIw\r\n"
"ggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC4Ena4vgWrzwUB0hGW1v0v\r\n"
"K986FhU5ZdYz5H5MGonfWwv89nR2DlftSDXEvKFyc2MT81GGm16VJv3mVpQJLuKA\r\n"
"xLBLY7a1zSrJdugXWy+mgJJTPW6KjTY4jPtfCl6x/yVr8YclVa8XO0JFzOme2LMV\r\n"
"Ylc/ixVEa66UpxRNrg5yWHS26KcB1lE3GLERoRBKF7nsyGqGY4X9TypBwglCVoqK\r\n"
"3dKVGwCvFur+oPnt/C5pwR6UmUV/Ppf1EaRD7Po+xcyJSeCvszG3FH4iHsDHnjLe\r\n"
"DR6lxouvMCb+aKJi9d0xowOjhbKoFMF179t4SVnptQeq+U6ui3cPKUjia7Zh1tZT\r\n"
"AgMBAAGjLzAtMAkGA1UdEwQCMAAwCwYDVR0PBAQDAgUgMBMGA1UdJQQMMAoGCCsG\r\n"
"AQUFBwMBMA0GCSqGSIb3DQEBCwUAA4IBAQB2jlDPiZfP/whsvvFn43g37QMwX5ST\r\n"
"Z5OpmEFnFjAH3ec0PPqPrKYEu00q5wEC+8L6uVH8FHOFf11JLH4wl11/C/mvE92D\r\n"
"qZtGG8KCnG2+rk5OJPGX+28Z+OnCZlXOjQ8qd2x5KtIW50JuXJ3cbDRHtF/TVanm\r\n"
"Exu+TCBeToNwbcU2sfQnbljkUTj4idUFz0pq3uvw3dA4R1J2foungPAYXSWcVhtb\r\n"
"RYtG8epIvkAyyUE5nY3kC05AUml6gSZkrJiYM5I1IJTX1lQ7Pv2yxRBZUtTx33rP\r\n"
"ccnsW6tbHTDBG8UDHx4LKHErdWFgCJWI81EUEcTip9g2zCOGTWKnpz+z\r\n"
"-----END CERTIFICATE-----\r\n";
