/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *    pcbreflux - mbedtls implementation via tls and websocket
 *******************************************************************************/

#include "MQTTmbedtls.h"

#define xMBEDTLS_MQTT_DEBUG
#define MBEDTLS_DEBUG_LEVEL 3
#define LOG_HEADER "[MQTTmbedtls] "

/************************************************************************************************/
 /* Local Variables */
 /************************************************************************************************/

/* Root cert for mqtt.thingspeak.com, found in cert.c */
extern const char *server_root_cert;

/************************************************************************************************/
 /* Local Functions Definition */
 /************************************************************************************************/

/**
 *
 * @param n
 * @param addr
 * @param port
 * @return
 */
static int __MQTTmbedtls_connectSSL(MQTTmbedtls_Network* n, char* addr, int port);

/**
 *
 * @param n
 * @param addr
 * @param port
 * @return
 */
static int __MQTTmbedtls_connectWebSocket(MQTTmbedtls_Network* n, char* addr, int port);

/**
 *
 * @param n
 * @param addr
 * @param port
 * @return
 */
static int __MQTTmbedtls_connectSocket(MQTTmbedtls_Network* n, char* addr, int port);

/**
 *
 * @param n
 */
static void __MQTTmbedtls_deinitSSL(MQTTmbedtls_Network* n);

/**
 *
 * @param n
 */
static void __MQTTmbedtls_deinitSocket(MQTTmbedtls_Network* n);

/**
 *
 * @param opcode
 * @param framebuffer
 * @param data
 * @param len
 * @param mask_flag
 * @return
 */
static int __MQTTmbedtls_websocket_create_frame(uint8_t opcode,unsigned char* framebuffer, unsigned char* data,int len,uint8_t mask_flag);

/**
 *
 * @param n
 * @param framebuffer
 * @param data
 * @param framlen
 * @return
 */
static int __MQTTmbedtls_websocket_remove_frame(MQTTmbedtls_Network* n,unsigned char* framebuffer, unsigned char* data, int framlen);

/**
 *
 * @param n
 * @param timeout_ms
 * @return
 */
static int __MQTTmbedtls_buffered_read(MQTTmbedtls_Network* n, int timeout_ms);

/**
 *
 * @param n
 * @param buffer
 * @param len
 * @param timeout_ms
 * @return
 */
static int __MQTTmbedtls_buffered_write(MQTTmbedtls_Network* n, unsigned char* buffer, int len, int timeout_ms);

/**
 * Ask a socket for the last error it encountered
 * @param socket - socket handler
 * @return last error code
 */
static int __MQTTmbedtls_espx_last_socket_errno(int socket);

/************************************************************************************************/
/* Public Functions Implementation */
/************************************************************************************************/

void MQTTmbedtls_initNetwork(MQTTmbedtls_Network* n, bool isSSL, bool isWebSocket)
{
	n->ws_recv_offset=0;
	n->ws_recv_len=0;
	n->mqtt_recv_offset=0;
	n->mqtt_recv_len=0;
	n->socket= -1;
	n->mqttread=MQTTmbedtls_read;
	n->mqttwrite=MQTTmbedtls_write;

	if(isSSL)
	{
		n->isSSL = 1;
		if(isWebSocket)
		{
			n->isWebsocket = 1;
		}
	}
	else
	{
		n->isSSL = 0;
		n->isWebsocket = 0;
	}
}


int MQTTmbedtls_ConnectNetwork(MQTTmbedtls_Network* n, char* addr, int port) {

	if((n->isSSL) && (__MQTTmbedtls_connectSSL(n,addr,port) != 0))
	{
		ESP_LOGE(LOG_HEADER, "Error initializing SSL connection");
		return -1;
	}
	else if((n->isWebsocket) && (__MQTTmbedtls_connectWebSocket(n,addr,port) != 0))
	{
		ESP_LOGE(LOG_HEADER, "Error initializing WEBSocket connection");
		return -1;
	}
	else if((!n->isSSL) && (__MQTTmbedtls_connectSocket(n,addr,port) != 0))
	{
		ESP_LOGE(LOG_HEADER, "Error initializing socket connection");
		return -1;
	}

	/* The connection was successfully initialized */
	return 0;
}



void MQTTmbedtls_deinitNetwork(MQTTmbedtls_Network* n) {
	if(n->isSSL)
	{
		__MQTTmbedtls_deinitSSL(n);
	}
	else
	{
		__MQTTmbedtls_deinitSocket(n);
	}
}

void TimerInit(MQTTmbedtls_Timer* timer)
{
	ESP_LOGD(LOG_HEADER, "TimerInit");
	timer->end_time = (struct timeval){0, 0};
}


int MQTTmbedtls_read(MQTTmbedtls_Network* n, unsigned char* buffer, int len, int timeout_ms)
{

	int bytes = 0;


	/* Clean output buffer */
	memset(buffer, 0, len);

	/* Verifies if the size required is available in the current MQTT buffer */
	if(len <= (n->mqtt_recv_len - n->mqtt_recv_offset))
	{
		/* Get the data from the current MQTT buffer offset and updates the offset */
		memmove(buffer, n->mqtt_recvbuf + n->mqtt_recv_offset, len);
		n->mqtt_recv_offset += len;
		bytes = len;
	}
	/* The size required is not available in the current MQTT buffer */
	else
	{
		if((n->mqtt_recv_len - n->mqtt_recv_offset) != 0)
		{
			/* Get data available in current MQTT buffer */
			memmove(buffer, n->mqtt_recvbuf + n->mqtt_recv_offset, n->mqtt_recv_len - n->mqtt_recv_offset);
			n->mqtt_recv_offset +=  (n->mqtt_recv_len - n->mqtt_recv_offset);
			bytes =  (n->mqtt_recv_len - n->mqtt_recv_offset);
		}
		/* Recharges MQTT buffer */
		if(__MQTTmbedtls_buffered_read(n, timeout_ms) <= 0)
		{
			/* Failed recharging MQTT buffer */
			return bytes;
		}
		/* Verifies if the remaining size required is available in MQTT buffer  */
		else if((len - bytes) <= n->mqtt_recv_len)
		{
			/* Get the data from the current MQTT buffer offset and updates the offset */
			memmove(buffer + bytes, n->mqtt_recvbuf, len - bytes);
			n->mqtt_recv_offset = len - bytes;
			bytes = len;
		}
		else
		{
			/* The remaining size isn't available in MQTT buffer */
			/* Get only the data available */
			memmove(buffer + bytes, n->mqtt_recvbuf, n->mqtt_recv_len);
			n->mqtt_recv_offset +=  n->mqtt_recv_len;
			bytes +=  n->mqtt_recv_len;
		}
	}

	return bytes;
}


int MQTTmbedtls_write(MQTTmbedtls_Network* n, unsigned char* buffer, int len, int timeout_ms) {
    int ret = __MQTTmbedtls_buffered_write(n, buffer, len, timeout_ms);
    if (ret>len) {  // websocket frame around mqtt
    	ret = len;
    }
    return ret;
}

/************************************************************************************************/
/* Local Functions Implementation */
/************************************************************************************************/

static int __MQTTmbedtls_connectSSL(MQTTmbedtls_Network* n, char* addr, int port)
{
    char portbuf[10];
	int result = 0;

	/* Free SSL network resources */
	__MQTTmbedtls_deinitSSL(n);

	/* MbedTLS Initializations */
	mbedtls_net_init(&n->server_fd);
    mbedtls_ssl_init(&n->ssl);
    mbedtls_ssl_config_init(&n->conf);
    mbedtls_ctr_drbg_init(&n->ctr_drbg);
    mbedtls_entropy_init(&n->entropy);
//    mbedtls_x509_crt_init(&n->cacert);

    /* Convert port to char * */
    sprintf(portbuf,"%d",port);

    /* Initialize RAMDOM number generator */
    if(mbedtls_ctr_drbg_seed(&n->ctr_drbg, mbedtls_entropy_func, &n->entropy, NULL, 0) != 0)
    {
        ESP_LOGE(LOG_HEADER, "Error initializing ramdom number generator");
        result = -1;
    }
    else if(mbedtls_net_connect(&n->server_fd, addr, portbuf, MBEDTLS_NET_PROTO_TCP) != 0) /* Connect the socket with HOST:PORT provided */
    {
        ESP_LOGE(LOG_HEADER, "Error connecting socket");
        result = -1;
    }
    else if(mbedtls_ssl_config_defaults(&n->conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT) != 0) /* Set SSL defaults configuration */
    {
        ESP_LOGE(LOG_HEADER, "Error setting SSL defaults configuration");
        result = -1;
    }
    else
	{
    	/* Configure SSL authentication mode */
    	mbedtls_ssl_conf_authmode(&n->conf, MBEDTLS_SSL_VERIFY_NONE);	/* TODO: Activate verification */
    	/* tell the environment which random number generator we wish to use */
    	mbedtls_ssl_conf_rng(&n->conf, mbedtls_ctr_drbg_random, &n->ctr_drbg);
	}

    if((result == 0) && (mbedtls_ssl_set_hostname(&n->ssl, addr) != 0)) /* Set the hostname to check against the received server  certificate*/
    {
    	ESP_LOGE(LOG_HEADER, "Error setting SSL hostname");
    	result = -1;
    }

    if(result == 0)
    {
    	/* instruct the SSL environment which functions to use to send and receive data */
    	mbedtls_ssl_set_bio(&n->ssl, &n->server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);
    }

//    /* Setup certificate */
//    if(mbedtls_x509_crt_parse(&n->cacert, (uint8_t*)server_root_cert, strlen(server_root_cert)+1) < 0)
//    {
//    	ESP_LOGE(LOG_HEADER, "Error parsing SSL certificate");
//    	result = -1;
//    }
//    mbedtls_ssl_conf_ca_chain(&n->conf, &n->cacert, NULL);
    mbedtls_net_set_block(&n->server_fd);

	/* Set up the SSL context for use. Can't modify the configuration structure after this*/
	if((result == 0) &&  (mbedtls_ssl_setup(&n->ssl, &n->conf) != 0))
	{
		ESP_LOGE(LOG_HEADER, "Set up of SSL context for use");
		result = -1;
	}

	/* SSL Handshake */
	if((result == 0) && (mbedtls_ssl_handshake(&n->ssl) != 0))
	{
    	ESP_LOGE(LOG_HEADER, "Error at SSL handshake");
    	result = -1;
	}

	if(result == 0)
	{
		ESP_LOGI(LOG_HEADER, "SSL connection successfully opened!");
	}

	return result;
}

static int __MQTTmbedtls_connectWebSocket(MQTTmbedtls_Network* n, char* addr, int port)
{
	int result = 0;
	int len;
	int byteSent;
	int byteSentSum;
	size_t buflen;
	unsigned char buf[512];
	unsigned char hash[20];

	// ToDo: generate random UUID i.e. Sec-WebSocket-Key use make gen_uuid all
	/* WebSocket client's  handshake, an HTTP Upgrade request */
	sprintf((char *)n->ws_sendbuf,
			"GET /mqtt HTTP/1.1\r\n"
			"Upgrade: websocket\r\n"
			"Connection: Upgrade\r\n"
			"Host: %s:%d\r\n"
			"Origin: https://%s:%d\r\n"
			"Sec-WebSocket-Key: %s\r\n"
			"Sec-WebSocket-Version: 13\r\n"
			"Sec-WebSocket-Protocol: mqtt\r\n\r\n",
			addr,port,addr,port,MBEDTLS_WEBSOCKET_UUID);
#ifdef MBEDTLS_MQTT_DEBUG
	ESP_LOGI(LOG_HEADER, "HTTP-WEBSOCKET Upgrade request\n%s",n->ws_sendbuf);
#endif	/*MBEDTLS_MQTT_DEBUG*/
	byteSent = 0;
	byteSentSum = 0;
	len = strlen((const char *)n->ws_sendbuf);
	while(((byteSent = mbedtls_ssl_write(&n->ssl,&n->ws_sendbuf[byteSentSum],len - byteSentSum)) > 0) && (byteSentSum < len))
	{
		byteSentSum += byteSent;
	}
	if(byteSentSum < len)
	{
		ESP_LOGE(LOG_HEADER, "Error sending HTTP-WEBSOCKET Upgrade request");
		result = -1;
	}
	else
	{
		/* Receive the handshake response from the server */
		bzero(n->ws_recvbuf, sizeof(n->ws_recvbuf));
		if(mbedtls_ssl_read(&n->ssl, n->ws_recvbuf, MBEDTLS_WEBSOCKET_RECV_BUF_LEN-1) <= 0)
		{
			ESP_LOGE(LOG_HEADER, "Error receiving HTTP-WEBSOCKET Upgrade response");
			result = -1;
		}
		else
		{
#ifdef MBEDTLS_MQTT_DEBUG
			/* Print response directly to stdout as it is read */
			ESP_LOGI(LOG_HEADER, "HTTP-WEBSOCKET request response:\n%s",n->ws_recvbuf);
#endif /* MBEDTLS_MQTT_DEBUG */
			/* Verify response */
			/* build hash expected in |Sec-WebSocket-Accept| header */
			bzero(buf,sizeof(buf));
			sprintf((char *)buf,"%s%s",MBEDTLS_WEBSOCKET_UUID,"258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
			mbedtls_sha1(buf, strlen((const char*)buf), hash);
			mbedtls_base64_encode(buf, sizeof(buf), &buflen,hash, sizeof(hash));
#ifdef MBEDTLS_MQTT_DEBUG
			ESP_LOGI(LOG_HEADER, "Expected hash: %s", buf);
#endif /* MBEDTLS_MQTT_DEBUG */


			if (strstr((const char *)n->ws_recvbuf, "HTTP/1.1 101 Switching Protocols") == NULL)
			{
				ESP_LOGE(LOG_HEADER, "Invalid response for HTTP-WEBSOCKET Upgrade request");
				result = -1;
			}
			else if(strstr((char *)n->ws_recvbuf, (char *)buf)==NULL)
			{
				ESP_LOGE(LOG_HEADER, "WebSocket handshake error, Sec-WebSocket-Accept invalid");
				result = -1;
			}
		}

	}

	if(result == 0)
	{
		ESP_LOGI(LOG_HEADER, "WebSocket connection successfully opened!");
	}

	return result;
}

static int __MQTTmbedtls_connectSocket(MQTTmbedtls_Network* n, char* addr, int port)
{
	int type = SOCK_STREAM;
	struct sockaddr_in address;
	int rc = -1;
	sa_family_t family = AF_INET;
	struct addrinfo *result = NULL;
	struct addrinfo hints = {0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL};

	if ((rc = getaddrinfo(addr, NULL, &hints, &result)) == 0)	{
		struct addrinfo* res = result;

		/* prefer ip4 addresses */
		while (res) {
			if (res->ai_family == AF_INET) {
				result = res;
				break;
			}
			res = res->ai_next;
		}

		if (result->ai_family == AF_INET) {
			address.sin_port = htons(port);
			address.sin_family = family = AF_INET;
			address.sin_addr = ((struct sockaddr_in*)(result->ai_addr))->sin_addr;
		}
		else {
			rc = -1;
		}

		freeaddrinfo(result);
	}

	if (rc == 0) {
		n->socket = socket(family, type, 0);
		if (n->socket != -1)
			rc = connect(n->socket, (struct sockaddr*)&address, sizeof(address));
	}

	return rc;
}

static void __MQTTmbedtls_deinitSSL(MQTTmbedtls_Network* n)
{
//	mbedtls_ssl_close_notify(&n->ssl);
	mbedtls_net_free(&n->server_fd);
	mbedtls_ssl_free(&n->ssl);
	mbedtls_ssl_config_free(&n->conf);
	mbedtls_ctr_drbg_free(&n->ctr_drbg);
	mbedtls_entropy_free(&n->entropy);
//	mbedtls_x509_crt_free(&n->cacert);

    n->ws_recv_offset=0;
    n->ws_recv_len=0;
    n->mqtt_recv_offset=0;
    n->mqtt_recv_len=0;
    n->socket = -1;

}

static void __MQTTmbedtls_deinitSocket(MQTTmbedtls_Network* n)
{
	close(n->socket);
    n->ws_recv_offset=0;
    n->ws_recv_len=0;
    n->mqtt_recv_offset=0;
    n->mqtt_recv_len=0;
    n->socket = -1;
}

static int __MQTTmbedtls_websocket_create_frame(uint8_t opcode,unsigned char* framebuffer, unsigned char* data,int len,uint8_t mask_flag)
{

	int bytepos = 0;
#if defined(MBEDTLS_MQTT_DEBUG)
    int mask_key[4] =   { 0x01,0x02,0x03,0x04 };
#else
    int mask_key[4] =   { (int)(esp_random()&0xFF),(int)(esp_random()&0xFF),(int)(esp_random()&0xFF),(int)(esp_random()&0xFF) };
#endif

	framebuffer[bytepos] = (1 << 7 | opcode);
	bytepos++;
    if (len < 126) {
    	framebuffer[bytepos] = (mask_flag << 7 | len);
    	bytepos++;
    } else if (len < 32768) {
    	framebuffer[bytepos] = (mask_flag << 7 | 126);
    	bytepos++;
    	framebuffer[bytepos] = (len >> 8  & 0xFF);
    	bytepos++;
    	framebuffer[bytepos] = (len & 0xFF);
    	bytepos++;
    } // ToDo : Frame >= 32768 i.e. len == 127
	if (mask_flag == 1) {
    	framebuffer[bytepos] = mask_key[0];
    	bytepos++;
    	framebuffer[bytepos] = mask_key[1];
    	bytepos++;
    	framebuffer[bytepos] = mask_key[2];
    	bytepos++;
    	framebuffer[bytepos] = mask_key[3];
    	bytepos++;
	}
	for (int i=0;i<len;i++) {
		if (mask_flag == 1) {
			framebuffer[bytepos+i]=data[i]^mask_key[i%4];  // Bitwise XOR at index modulo 4
		} else {
			framebuffer[bytepos+i]=data[i];
		}
	}
    return bytepos+len;
}

static int __MQTTmbedtls_websocket_remove_frame(MQTTmbedtls_Network* n,unsigned char* framebuffer, unsigned char* data, int framlen)
{

	int bytepos = 0;
	int opcode;
	uint8_t mask_flag = 1;
    int mask_key[4] =   { 0x00,0x00,0x00,0x00 };
    int len;

    opcode = framebuffer[bytepos] & 0x0F;
	bytepos++;
	len = framebuffer[bytepos] & 0x7F;
	mask_flag = framebuffer[bytepos]>>7;
	bytepos++;
    if (len == 126) {
    	len = framebuffer[bytepos]<<8;
    	bytepos++;
    	len += framebuffer[bytepos];
    	bytepos++;
    }
    if (len == 127) {
    	len = 0; // ToDo : Frame >= 32768 i.e. len == 127
        goto exit;
    }

    if (len>framlen) {
    	len = framlen;
    }
	if (mask_flag == 1) {
		mask_key[0] = framebuffer[bytepos];
    	bytepos++;
    	mask_key[1] = framebuffer[bytepos];
    	bytepos++;
    	mask_key[2] = framebuffer[bytepos];
    	bytepos++;
    	mask_key[3] = framebuffer[bytepos];
    	bytepos++;
	}
	for (int i=0;i<len;i++) {
		if (mask_flag == 1) {
			data[i]=framebuffer[bytepos+i]^mask_key[i%4];  // Bitwise XOR at index modulo 4
		} else {
			data[i]=framebuffer[bytepos+i];
		}
	}
	if (opcode==WEBSOCKET_CONNCLOSE||opcode==WEBSOCKET_PING) {
		if (opcode==WEBSOCKET_PING) {
			opcode=WEBSOCKET_PONG;
		}
		ESP_LOGD(LOG_HEADER, "opcode %d websocket_mbedtls_write mqtt want %d",opcode,len);
		int framelen = __MQTTmbedtls_websocket_create_frame(opcode,n->ws_sendbuf,data,len,0);

		for (int i=0;i<framelen;i++) {
			ESP_LOGD(LOG_HEADER, "opcode websocket_mbedtls_write: %d %02X [%c]",i+1, n->ws_sendbuf[i], n->ws_sendbuf[i]);
		}
		int ret = mbedtls_ssl_write(&n->ssl, n->ws_sendbuf, framelen);
		ESP_LOGD(LOG_HEADER, "opcode mbedtls_ssl_write websocket %d from %d",ret,len);
	}

exit:
    return len;
}

static int __MQTTmbedtls_buffered_read(MQTTmbedtls_Network* n, int timeout_ms)
{
	int sslReadResult = 0;

	/* Restart buffers */
	memset(n->ws_recvbuf, 0, MBEDTLS_WEBSOCKET_RECV_BUF_LEN);
	memset(n->mqtt_recvbuf, 0, MBEDTLS_MQTT_RECV_BUF_LEN);
	n->mqtt_recv_len=0;
	n->mqtt_recv_offset=0;
	n->ws_recv_len=0;
	n->ws_recv_offset = 0;

	/* Set socket receive timeout */
	if(timeout_ms <= 0)
	{
		return -1;
	}
	struct timeval interval = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};

	if(n->isSSL)
	{
		setsockopt(n->server_fd.fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));
		if (n->isWebsocket)
		{
			sslReadResult = mbedtls_ssl_read(&n->ssl, n->ws_recvbuf,MBEDTLS_WEBSOCKET_RECV_BUF_LEN-1);
		}
		else
		{
			sslReadResult = mbedtls_ssl_read(&n->ssl, n->mqtt_recvbuf,MBEDTLS_MQTT_RECV_BUF_LEN-1);
		}
	}
	else
	{
		setsockopt(n->socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));
		sslReadResult = recv(n->socket, n->mqtt_recvbuf,MBEDTLS_MQTT_RECV_BUF_LEN-1, 0);
	}

	if((sslReadResult == MBEDTLS_ERR_SSL_WANT_READ) || (sslReadResult == MBEDTLS_ERR_SSL_WANT_WRITE))
	{
		return 0;
	}
	else if(sslReadResult <= 0)
	{
		return -1;
	}
	else
	{
		if (n->isWebsocket)
		{
			n->ws_recv_len = sslReadResult;
			/* Remove WEBSOCKET frame*/
			n->mqtt_recv_len=__MQTTmbedtls_websocket_remove_frame(n,n->ws_recvbuf,n->mqtt_recvbuf,n->ws_recv_len);

#ifdef MBEDTLS_MQTT_DEBUG
			ESP_LOGI(LOG_HEADER, "---------------------WEBSOCKET RECEIVED FRAME---------------------");
			for (int i=0;i<sslReadResult;i++)
			{
				if((i%32) == 0)
				{
					printf("\r\n");
				}
				printf("0x%02X,",n->ws_recvbuf[i]);
			}
			printf("\r\n");
			ESP_LOGI(LOG_HEADER, "------------------------------------------------------------------");
#endif /* MBEDTLS_MQTT_DEBUG */
		}
		else
		{
			n->mqtt_recv_len = sslReadResult;
#ifdef MBEDTLS_MQTT_DEBUG
			ESP_LOGI(LOG_HEADER, "---------------------SSL/SOCKET DATA RECEIVED---------------------");
			for (int i=0;i<sslReadResult;i++)
			{
				if((i%32) == 0)
				{
					printf("\r\n");
				}
				printf("0x%02X,",n->mqtt_recvbuf[i]);
			}
			printf("\r\n");
			ESP_LOGI(LOG_HEADER, "------------------------------------------------------------------");
#endif /* MBEDTLS_MQTT_DEBUG */
		}
	}

	return n->mqtt_recv_len;
}

static int __MQTTmbedtls_buffered_write(MQTTmbedtls_Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	int framelen = 0;
	int byteSent = 0;
	int byteSentSum = 0;

	/* Set socket receive timeout */
	if(timeout_ms <= 0)
	{
		return -1;
	}
	struct timeval interval = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};

	/* Set socket receive timeout */

	if(n->isSSL)
	{
		setsockopt(n->server_fd.fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&interval, sizeof(struct timeval));
		if (n->isWebsocket)
		{
			/* Clean websocket send buffer */
			memset(n->ws_sendbuf, 0, MBEDTLS_WEBSOCKET_RECV_BUF_LEN);
			/* Pack the data into a WEBSOCKET frame */
			if((framelen = __MQTTmbedtls_websocket_create_frame(WEBSOCKET_BINARY,n->ws_sendbuf,buffer,len,1)) < len)
			{
				ESP_LOGE(LOG_HEADER, "Error creating websocket frame to send data");
				return -1;
			}
			byteSent = 0;
			byteSentSum = 0;
			while(((byteSent = mbedtls_ssl_write(&n->ssl,&n->ws_sendbuf[byteSentSum],framelen - byteSentSum)) > 0) && (byteSentSum < framelen))
			{
				byteSentSum += byteSent;
			}
			if(byteSentSum < framelen)
			{
#ifdef MBEDTLS_MQTT_DEBUG
				ESP_LOGE(LOG_HEADER, "Error sending WEBSOCKET frame");
#endif	/* MBEDTLS_MQTT_DEBUG */
				return -1;
			}
			else
			{
				/* Data successfully sent */
#ifdef MBEDTLS_MQTT_DEBUG
				ESP_LOGI(LOG_HEADER, "---------------------WEBSOCKET SEND FRAME---------------------");
				for (int i=0;i<framelen;i++)
				{
					if((i%32) == 0)
					{
						printf("\r\n");
					}
					printf("0x%02X,",n->ws_sendbuf[i]);
				}
				printf("\r\n");
				ESP_LOGI(LOG_HEADER, "--------------------------------------------------------------");
#endif	/* MBEDTLS_MQTT_DEBUG */
				return framelen;
			}
		}
		else
		{
			byteSent = 0;
			byteSentSum = 0;
			while(((byteSent = mbedtls_ssl_write(&n->ssl,&buffer[byteSentSum],len - byteSentSum)) > 0) && (byteSentSum < len))
			{
				byteSentSum += byteSent;
			}
			if(byteSentSum < len)
			{
#ifdef MBEDTLS_MQTT_DEBUG
				ESP_LOGE(LOG_HEADER, "Error sending data over SSL connection");
#endif	/* MBEDTLS_MQTT_DEBUG */
				return -1;
			}
			else
			{
				/* Data successfully sent */
#ifdef MBEDTLS_MQTT_DEBUG
				ESP_LOGI(LOG_HEADER, "--------------------- DATA SEND OVER SSL CONNECTION---------------------");

				for (int i=0;i < len;i++)
				{
					if((i%32) == 0)
					{
						printf("\r\n");
					}
					printf("0x%02X,",buffer[i]);
				}
				printf("\r\n");
				ESP_LOGI(LOG_HEADER, "------------------------------------------------------------------------");
#endif	/* MBEDTLS_MQTT_DEBUG */
				return len;
			}
		}
	}
	else
	{
		setsockopt(n->socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&interval,sizeof(struct timeval));
		byteSent = 0;
		byteSentSum = 0;
		while(((byteSent = write(n->socket,&buffer[byteSentSum],len - byteSentSum)) > 0) && (byteSentSum < len))
		{
			byteSentSum += byteSent;
		}
		if(byteSentSum < len)
		{
#ifdef MBEDTLS_MQTT_DEBUG
			ESP_LOGE(LOG_HEADER, "Error sending data to server");
#endif	/* MBEDTLS_MQTT_DEBUG */
			return -1;
		}
		else
		{
			/* Data successfully sent */
#ifdef MBEDTLS_MQTT_DEBUG
			ESP_LOGI(LOG_HEADER, "--------------------- DATA SEND OVER SOCKET CONNECTION---------------------");
			for (int i=0;i < len;i++)
			{
				if((i%32) == 0)
				{
					printf("\r\n");
				}
				printf("0x%02X,",buffer[i]);
			}
			printf("\r\n");
			ESP_LOGI(LOG_HEADER, "---------------------------------------------------------------------------");
#endif	/* MBEDTLS_MQTT_DEBUG */
			return len;
		}
	}

	return len;
}

static int __MQTTmbedtls_espx_last_socket_errno(int socket)
{
int ret = 0;
uint32_t optlen = sizeof(ret);
getsockopt(socket, SOL_SOCKET, SO_ERROR, &ret, &optlen);
return ret;
}
