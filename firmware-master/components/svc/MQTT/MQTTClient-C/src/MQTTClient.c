/*******************************************************************************
 * Copyright (c) 2014, 2015 IBM Corp.
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
 *    Allan Stockdill-Mander/Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/
#include "MQTTClient.h"

#define DEBUG_MQTT_CLIENT
#define LOG_HEADER 			"[MQTT CLIENT] "

#define MAX_NO_OF_REMAINING_LENGTH_BYTES  	4

#define MAX_PACKET_ID 65535 														/*!< according to the MQTT specification - do not change! */
#define MQTT_CLIENT_SEMAPHORE_TIMEOUT		5000 / portTICK_PERIOD_MS				/*!< Waiting timeout to access MQTT client shared resources */
#define PING_PERIOD_MS						(60000)									/*!< Minimum interval between PING requests */
#define WAIT_SLEEP_MS						(100)									/*!< Sleep time for iterations waiting socket incoming data */
#define ACKNOWLEDGMENT_TIMEOUT_MS			(20000) 								/* Acknowledgment timeout for command sent to broker */
#define SEND_COMMAND_TIMEOUT_MS				(5000) 									/* Timeout for send commands to broker */

/************************************************************************************************/
 /* Local Functions Definition */
 /************************************************************************************************/

/**
 *
 * @param c
 * @param timer
 * @return
 */
static int __MQTTClient_readPacket(MQTTClient* c, MQTTmbedtls_Timer* timer);

/**
 *
 * @param c
 * @param length
 * @param timer
 * @return
 */
static int __MQTTClient_sendPacket(MQTTClient* c, int length, MQTTmbedtls_Timer* timer);

/**
 *
 * @param c
 * @param value
 * @param timeout
 * @return
 */
static int __MQTTClient_decodePacket(MQTTClient* c, int* value, int timeout);

/**
 * Assume topic filter and name is in correct format. # can only be at end, + and # can only be next to separator.
 *
 * @param topicFilter
 * @param topicName
 * @return
 */
static char __MQTTClient_isTopicMatched(char* topicFilter, MQTTString* topicName);

/**
 *
 * @param c
 * @param topicName
 * @param message
 * @return
 */
static int __MQTTClient_deliverMessage(MQTTClient* c, MQTTString* topicName, MQTTMessage* message);

/**
 *
 * @param c
 * @return
 */
static int __MQTTClient_keepalive(MQTTClient* c);

/**
 *
 * @param c
 * @param timer
 * @return
 */
static int __MQTTClient_cycle(MQTTClient* c, MQTTmbedtls_Timer* timer);

/**
 *
 * @param c
 * @param packet_type
 * @param timer
 * @return
 */
static int __MQTTClient_waitfor(MQTTClient* c, int packet_type, MQTTmbedtls_Timer* timer);

/**
 *
 * @param md
 * @param aTopicName
 * @param aMessage
 */
static void __MQTTClient_NewMessageData(MessageData* md, MQTTString* aTopicName, MQTTMessage* aMessage);

/**
 *
 * @param c
 * @return
 */
static int __MQTTClient_getNextPacketId(MQTTClient *c);

/**
 *
 * @param timer
 * @return
 */
static bool __MQTTClient_TimerIsExpired(MQTTmbedtls_Timer* timer);

/**
 *
 * @param timer
 * @param timeout_ms
 */
static void __MQTTClient_TimerStart(MQTTmbedtls_Timer* timer, unsigned int timeout_ms);

/**
 *
 * @param timer
 * @return
 */
static uint64_t __MQTTClient_TimerLeftMS(MQTTmbedtls_Timer* timer);


/************************************************************************************************/
/* Public Functions Implementation */
/************************************************************************************************/
void MQTTClient_Init(MQTTClient* c, MQTTmbedtls_Network* network, unsigned int command_timeout_ms,
		unsigned char* sendbuf, size_t sendbuf_size, unsigned char* readbuf, size_t readbuf_size)
{
    int i;
    c->ipstack = network;
    
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
        c->messageHandlers[i].topicFilter = 0;
    c->command_timeout_ms = command_timeout_ms;
    c->buf = sendbuf;
    c->buf_size = sendbuf_size;
    c->readbuf = readbuf;
    c->readbuf_size = readbuf_size;
    c->isconnected = 0;
    c->defaultMessageHandler = NULL;
	c->next_packetid = 1;
	c->ping_outstanding = 0;
    TimerInit(&c->ping_timer);
    c->semaphore = xSemaphoreCreateMutex();
}

int MQTTClient_Yield(MQTTClient* c, int timeout_ms)
{
    int result = SUCCESS;
    MQTTmbedtls_Timer yieldTimer;

	while( xSemaphoreTake( c->semaphore, MQTT_CLIENT_SEMAPHORE_TIMEOUT ) != pdTRUE )
	{
		vTaskDelay(WAIT_SLEEP_MS/portTICK_PERIOD_MS);
	}

	/* Initialize yield timer */
	TimerInit(&yieldTimer);
	__MQTTClient_TimerStart(&yieldTimer, timeout_ms);

    do
    {
		if(__MQTTClient_cycle(c, &yieldTimer) == FAILURE)
		{
			ESP_LOGE(LOG_HEADER, "MQTT yield detect a network fail!");
			result = FAILURE;
		}
		else if(__MQTTClient_keepalive(c) == FAILURE)
		{
			ESP_LOGE(LOG_HEADER, "MQTT keep alive failed!");
			result = FAILURE;
		}
		else
		{
			vTaskDelay(WAIT_SLEEP_MS/portTICK_PERIOD_MS);
		}
    }
	while ((result != FAILURE) && (!__MQTTClient_TimerIsExpired(&yieldTimer)));

    /* Release the semaphore */
    xSemaphoreGive(c->semaphore);

	return result;
}

int MQTTClient_Connect(MQTTClient* c, MQTTPacket_connectData* options)
{
    MQTTmbedtls_Timer connect_timer;
    int result = SUCCESS;
    MQTTPacket_connectData default_options = MQTTPacket_connectData_initializer;
    int byteCounter = 0;
    unsigned char connack_rc = 255;
    unsigned char sessionPresent = 0;

	while( xSemaphoreTake( c->semaphore, MQTT_CLIENT_SEMAPHORE_TIMEOUT ) != pdTRUE )
	{
		vTaskDelay(WAIT_SLEEP_MS/portTICK_PERIOD_MS);
	}

	if (c->isconnected)
	{
		/* don't send connect packet again if we are already connected */
	    /* Release the semaphore */
	    xSemaphoreGive(c->semaphore);
		return SUCCESS;
	}

	/* set default options if none were supplied */
	if (options == NULL)
	{
		options = &default_options;
	}
    
	/* Initilize connect timer */
    TimerInit(&connect_timer);
    __MQTTClient_TimerStart(&connect_timer, SEND_COMMAND_TIMEOUT_MS);


    /* Build, send and analyze response for MQTT connection packet */
    if ((byteCounter = MQTTSerialize_connect(c->buf, c->buf_size, options)) <= 0)
    {
#ifdef DEBUG_MQTT_CLIENT
    	ESP_LOGE(LOG_HEADER, "Failure building MQTT connection packet");
#endif
    	result = FAILURE;
    }
    else if(__MQTTClient_sendPacket(c, byteCounter, &connect_timer) != SUCCESS)
    {
#ifdef DEBUG_MQTT_CLIENT
    	ESP_LOGE(LOG_HEADER, "Failure sending MQTT connection packet");
#endif
    	result = FAILURE;
    }
    else
    {
    	/* Restart timer with acknowledgment timeout */
    	__MQTTClient_TimerStart(&connect_timer, ACKNOWLEDGMENT_TIMEOUT_MS);
    }
    if(__MQTTClient_waitfor(c, CONNACK, &connect_timer) != CONNACK)
    {
#ifdef DEBUG_MQTT_CLIENT
    	ESP_LOGE(LOG_HEADER, "Failure receiving MQTT connection acknowledgment");
#endif
    	result = COMMAND_NACK;
    }
    else if(MQTTDeserialize_connack(&sessionPresent, &connack_rc, c->readbuf, c->readbuf_size) != 1)
    {
#ifdef DEBUG_MQTT_CLIENT
    	ESP_LOGE(LOG_HEADER, "Failure decoding MQTT connection acknowledgment");
#endif
    	result = FAILURE;
    }

    if(result == SUCCESS)
    {
    	c->isconnected = 1;
    	ESP_LOGI(LOG_HEADER, "MQTT client succesfully connected");
    	/* Initialize ping timer */
    	c->keepAliveInterval = (options->keepAliveInterval * 1000);
    	__MQTTClient_TimerStart(&c->ping_timer, PING_PERIOD_MS);
    }
    else
    {
    	ESP_LOGE(LOG_HEADER, "MQTT client connection Failed");
    }

    /* Release the semaphore */
    xSemaphoreGive(c->semaphore);

    return result;
}


int MQTTClient_Subscribe(MQTTClient* c, const char* topicFilter, enum QoS qos, messageHandler messageHandler)
{ 
    int result = SUCCESS;
    MQTTmbedtls_Timer subscriptionTimer;
    int byteCounter = 0;
    MQTTString topic = MQTTString_initializer;
    unsigned short packetid;
    int grantedQoSsMax = 1;
    int grantedQoSsCount = 0;
    int grantedQoSs;
    int messageHandlerIndex = 0;

    topic.cstring = (char *)topicFilter;

	while( xSemaphoreTake( c->semaphore, MQTT_CLIENT_SEMAPHORE_TIMEOUT ) != pdTRUE )
	{
		vTaskDelay(WAIT_SLEEP_MS/portTICK_PERIOD_MS);
	}

	if (!c->isconnected)
	{
		ESP_LOGE(LOG_HEADER, "MQTT subscription failed, client isn't connected");
	    /* Release the semaphore */
	    xSemaphoreGive(c->semaphore);
		return FAILURE;
	}

	/* Initialize subscription timer */
    TimerInit(&subscriptionTimer);
    __MQTTClient_TimerStart(&subscriptionTimer, SEND_COMMAND_TIMEOUT_MS);
    
    /* Build, send and analyze response for MQTT subscribe packet */
    if((byteCounter = MQTTSerialize_subscribe(c->buf, c->buf_size, 0, __MQTTClient_getNextPacketId(c), 1, &topic, (int*)&qos)) <= 0)
    {
#ifdef DEBUG_MQTT_CLIENT
    	ESP_LOGE(LOG_HEADER, "Failure building MQTT subscribe packet");
#endif
		result = FAILURE;
    }
    else if(__MQTTClient_sendPacket(c, byteCounter, &subscriptionTimer) != SUCCESS)
    {
#ifdef DEBUG_MQTT_CLIENT
    	ESP_LOGE(LOG_HEADER, "Failure sending MQTT subscribe packet");
#endif
        result = FAILURE;
    }
    else
	{
		/* Restart timer with acknowledgment timeout */
		__MQTTClient_TimerStart(&subscriptionTimer, ACKNOWLEDGMENT_TIMEOUT_MS);
	}
	if(__MQTTClient_waitfor(c, SUBACK, &subscriptionTimer) != SUBACK)
	{
#ifdef DEBUG_MQTT_CLIENT
    	ESP_LOGE(LOG_HEADER, "Failure receiving MQTT subscribe acknowledgment");
#endif
        result = COMMAND_NACK;
	}
    else if(MQTTDeserialize_suback(&packetid, grantedQoSsMax, &grantedQoSsCount, &grantedQoSs, c->readbuf, c->readbuf_size) != 1)
    {
#ifdef DEBUG_MQTT_CLIENT
    	ESP_LOGE(LOG_HEADER, "Failure decoding MQTT subscribe acknowledgment");
#endif
        result = FAILURE;
    }
    else if(grantedQoSs != qos)
    {
#ifdef DEBUG_MQTT_CLIENT
    	ESP_LOGE(LOG_HEADER, " Subscription failure, QOS different of the required ");
#endif
    	result = FAILURE;
    }
    else
    {
    	/* Look for an unused message handler */
    	while((messageHandlerIndex < MAX_MESSAGE_HANDLERS) && (c->messageHandlers[messageHandlerIndex].topicFilter != 0))
    	{
    		messageHandlerIndex++;
    	}

    	if(messageHandlerIndex < MAX_MESSAGE_HANDLERS)
    	{
  			c->messageHandlers[messageHandlerIndex].topicFilter = topicFilter;
    		c->messageHandlers[messageHandlerIndex].fp = messageHandler;
    	}
    	else
    	{
    	   	ESP_LOGE(LOG_HEADER, "Subscription failure, there is no message handler available");
    	    result = FAILURE;
    	}
    }

    /* Release the semaphore */
    xSemaphoreGive(c->semaphore);
    return result;
}


int MQTTClient_Unsubscribe(MQTTClient* c, const char* topicFilter)
{   
    int rc = FAILURE;
    MQTTmbedtls_Timer timer;    
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicFilter;
    int len = 0;
    unsigned short mypacketid;
    int topicIndex = -1;

	while( xSemaphoreTake( c->semaphore, MQTT_CLIENT_SEMAPHORE_TIMEOUT ) != pdTRUE )
	{
		vTaskDelay(WAIT_SLEEP_MS/portTICK_PERIOD_MS);
	}

    /* Look for topic in subscription topic list */
    for(int subscriprionIndex = 0; subscriprionIndex < MAX_MESSAGE_HANDLERS; ++subscriprionIndex)
    {
        if (strcmp(c->messageHandlers[subscriprionIndex].topicFilter, topicFilter) == 0)
        {
            topicIndex = subscriprionIndex;
        }
    }
    if((topicIndex == -1) || (!c->isconnected) )
    {
        /* Release the semaphore */
        xSemaphoreGive(c->semaphore);
    	return FAILURE;
    }

    TimerInit(&timer);
    __MQTTClient_TimerStart(&timer, c->command_timeout_ms);
    
    if (((len = MQTTSerialize_unsubscribe(c->buf, c->buf_size, 0, __MQTTClient_getNextPacketId(c), 1, &topic)) != SUCCESS) ||
    		((rc = __MQTTClient_sendPacket(c, len, &timer)) != SUCCESS) ||
			(__MQTTClient_waitfor(c, UNSUBACK, &timer) != UNSUBACK) ||
			(MQTTDeserialize_unsuback(&mypacketid, c->readbuf, c->readbuf_size) != 1))

    {
    	ESP_LOGE("MQTTClient", "Unsubscription failed, topic %s", topicFilter);
    	rc = FAILURE;
    }
    else
    {
    	ESP_LOGI("MQTTClient", "Unsubscription successful , topic %s", topicFilter);
        c->messageHandlers[topicIndex].topicFilter = NULL;
        c->messageHandlers[topicIndex].fp = NULL;
    	rc = SUCCESS;
    }

    /* Release the semaphore */
    xSemaphoreGive(c->semaphore);

    return rc;
}


int MQTTClient_Publish(MQTTClient* c, const char* topicName, MQTTMessage* message)
{
    int result = SUCCESS;
    MQTTmbedtls_Timer publishTimer;   
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicName;
    int byteCounter = 0;
    unsigned short mypacketid;
    unsigned char dup, type;

	if( xSemaphoreTake( c->semaphore, MQTT_CLIENT_SEMAPHORE_TIMEOUT ) != pdTRUE )
	{
		/* We could not obtain the semaphore and can therefore not access the shared resource safely. */
#ifdef DEBUG_MQTT_CLIENT
		ESP_LOGE(LOG_HEADER,"Error taking MQTT Client semaphore, MQTTClient_Publish");
#endif /* DEBUG_MQTT_CLIENT */
		return SEMAPHORE_TIMEOUT;
	}

	if (!c->isconnected)
	{
	    /* Release the semaphore */
		ESP_LOGE(LOG_HEADER,"Error sending data over MQTT Network, client isn't connected");
	    xSemaphoreGive(c->semaphore);
		return FAILURE;
	}

	/* Initialize publish timer */
    TimerInit(&publishTimer);
    __MQTTClient_TimerStart(&publishTimer, SEND_COMMAND_TIMEOUT_MS);

    if (message->qos == QOS1 || message->qos == QOS2)
    {
    	message->id = __MQTTClient_getNextPacketId(c);
    }

    /* Build, send and analyze response for MQTT publish packet */
    if((byteCounter = MQTTSerialize_publish(c->buf, c->buf_size, message->dup, message->qos, message->retained, message->id,
            topic, (unsigned char*)message->payload, message->payloadlen)) <= 0)
    {
#ifdef DEBUG_MQTT_CLIENT
    	ESP_LOGE(LOG_HEADER, "Failure building MQTT publish packet");
#endif
		result = FAILURE;
    }
    else if( __MQTTClient_sendPacket(c, byteCounter, &publishTimer) != SUCCESS)
    {
#ifdef DEBUG_MQTT_CLIENT
    	ESP_LOGE(LOG_HEADER, "Failure sending MQTT publish packet");
#endif
        result = FAILURE;
    }
    else
    {
    	/* Restart timer for wait PUBACK, it can take so long */
    	__MQTTClient_TimerStart(&publishTimer, ACKNOWLEDGMENT_TIMEOUT_MS);
    }
    if((message->qos == QOS1) && (__MQTTClient_waitfor(c, PUBACK, &publishTimer) != PUBACK))
	{
    	result = COMMAND_NACK;
#ifdef DEBUG_MQTT_CLIENT
    	ESP_LOGE(LOG_HEADER, "Failure receiving MQTT publish acknowledgment");
#endif

	}
    if((message->qos == QOS2) && (__MQTTClient_waitfor(c, PUBCOMP, &publishTimer) != PUBCOMP))
    {
#ifdef DEBUG_MQTT_CLIENT
    	ESP_LOGE(LOG_HEADER, "Failure receiving MQTT publish acknowledgment");
#endif
    	result = COMMAND_NACK;
    }
    else if(MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
    {
#ifdef DEBUG_MQTT_CLIENT
    	ESP_LOGE(LOG_HEADER, "Failure decoding MQTT publish acknowledgment");
#endif
        result = FAILURE;
    }
    
#ifdef DEBUG_MQTT_CLIENT
    if(result == SUCCESS)
    {
    	ESP_LOGI(LOG_HEADER, "MESSAGE PUBLISHED! Status: %d, MEssage ID: %d, Acknowledgment Packet ID: %d", result, message->id, mypacketid);
    }
#endif

    /* Release the semaphore */
    xSemaphoreGive(c->semaphore);

    return result;
}


int MQTTClient_Disconnect(MQTTClient* c)
{  
    MQTTmbedtls_Timer disconnectionTimer;     // we might wait for incomplete incoming publishes to complete
    int byteCounter = 0;
    int result = SUCCESS;

	while( xSemaphoreTake( c->semaphore, MQTT_CLIENT_SEMAPHORE_TIMEOUT ) != pdTRUE )
	{
		vTaskDelay(WAIT_SLEEP_MS/portTICK_PERIOD_MS);
	}

	if(!c->isconnected)
	{
		/* Client already disconnected */
	    /* Release the semaphore */
	    xSemaphoreGive(c->semaphore);
		return SUCCESS;
	}

	/* Initialize disconnection packet */
	TimerInit(&disconnectionTimer);
    __MQTTClient_TimerStart(&disconnectionTimer, SEND_COMMAND_TIMEOUT_MS);

	while((c->isconnected) && (!__MQTTClient_TimerIsExpired(&disconnectionTimer)))
	{
	    /* Build, send and analyze response for MQTT disconnection packet */
	    if ((byteCounter = MQTTSerialize_disconnect(c->buf, c->buf_size)) <= 0)
	    {
#ifdef DEBUG_MQTT_CLIENT
	    	ESP_LOGE(LOG_HEADER, "Failure building MQTT disconnection packet");
#endif
	    	result = FAILURE;
	    }
	    else if(__MQTTClient_sendPacket(c, byteCounter, &disconnectionTimer) != SUCCESS)
		{
	        result = FAILURE;
		}
	}

	if(result != SUCCESS)
	{
    	ESP_LOGE(LOG_HEADER, "Failure disconnecting MQTT client");
	}

	c->isconnected = 0;

    /* Release the semaphore */
    xSemaphoreGive(c->semaphore);
    return result;
}

/************************************************************************************************/
/* Local Functions Implementation */
/************************************************************************************************/

static int __MQTTClient_sendPacket(MQTTClient* c, int length, MQTTmbedtls_Timer* timer)
{
    int writeCounter = 0;
    int byteSent = 0;
    int result = FAILURE;


    while ((!__MQTTClient_TimerIsExpired(timer)) && (result != SUCCESS))
    {
    	writeCounter = c->ipstack->mqttwrite(c->ipstack, &c->buf[byteSent], length, __MQTTClient_TimerLeftMS(timer));
    	if(writeCounter == length)
    	{
    		result = SUCCESS;
    	}
    }

    return result;
}

static int __MQTTClient_decodePacket(MQTTClient* c, int* value, int timeout)
{
    int multiplier = 1;
    int len = 0;
    uint8_t digit;

    /* Algorithm for decoding the Remaining Length field. Reference: MQTT 3.1 - SPEC */
    multiplier = 1;
    *value = 0;
    do
    {
        if((++len > MAX_NO_OF_REMAINING_LENGTH_BYTES) || (c->ipstack->mqttread(c->ipstack, &digit, 1, timeout) != 1))
        {
        	return MQTTPACKET_READ_ERROR;
        }
        else
        {
        	*value += (digit & 127) * multiplier;
        	multiplier *= 128;
        }

    }
    while ((digit & 128) != 0);

    return SUCCESS;
}

static int __MQTTClient_readPacket(MQTTClient* c, MQTTmbedtls_Timer* timer)
{
    MQTTHeader header = {0};
    int len = 0;
    int rem_len = 0;

    /* 1. read the header byte.  This has the packet type in it */
    if(c->ipstack->mqttread(c->ipstack, c->readbuf, 1, __MQTTClient_TimerLeftMS(timer)) != 1)
    {
    	/* Error reading MQTT packet header. Maybe we doesn't have any packet, so this may not be a connection error. */
    	return 0;
    }
    /* 2. read the remaining length.  This is variable in itself */
    else if(__MQTTClient_decodePacket(c, &rem_len, __MQTTClient_TimerLeftMS(timer)) != SUCCESS)
    {
    	return FAILURE;
    }
    else
    {
    	/* put the original remaining length back into the buffer */
    	len = 1 + MQTTPacket_encode(c->readbuf + 1, rem_len);

    	/* 3. read the rest of the buffer based on remaining length */
    	if((rem_len > 0) && (c->ipstack->mqttread(c->ipstack, c->readbuf + len, rem_len, __MQTTClient_TimerLeftMS(timer)) != rem_len))
    	{
    		return FAILURE;
    	}
    	else
    	{
    		header.byte = c->readbuf[0];
    	}
    }

    /* The message was successfully read, return the message type */
    return header.bits.type;
}

static char __MQTTClient_isTopicMatched(char* topicFilter, MQTTString* topicName)
{
    char* curf = topicFilter;
    char* curn = topicName->lenstring.data;
    char* curn_end = curn + topicName->lenstring.len;

    while (*curf && curn < curn_end)
    {
        if (*curn == '/' && *curf != '/')
            break;
        if (*curf != '+' && *curf != '#' && *curf != *curn)
            break;
        if (*curf == '+')
        {   // skip until we meet the next separator, or end of string
            char* nextpos = curn + 1;
            while (nextpos < curn_end && *nextpos != '/')
                nextpos = ++curn + 1;
        }
        else if (*curf == '#')
            curn = curn_end - 1;    // skip until end of string
        curf++;
        curn++;
    };

    return (curn == curn_end) && (*curf == '\0');
}

static int __MQTTClient_deliverMessage(MQTTClient* c, MQTTString* topicName, MQTTMessage* message)
{
    int messageHandlerIndex;
    int result = FAILURE;
    MessageData md;

    // we have to find the right message handler - indexed by topic
    for (messageHandlerIndex = 0; messageHandlerIndex < MAX_MESSAGE_HANDLERS; ++messageHandlerIndex)
    {
        if ((c->messageHandlers[messageHandlerIndex].topicFilter != NULL) && (MQTTPacket_equals(topicName, (char*)c->messageHandlers[messageHandlerIndex].topicFilter) ||
                __MQTTClient_isTopicMatched((char*)c->messageHandlers[messageHandlerIndex].topicFilter, topicName)))
        {
            if (c->messageHandlers[messageHandlerIndex].fp != NULL)
            {
                __MQTTClient_NewMessageData(&md, topicName, message);
                c->messageHandlers[messageHandlerIndex].fp(&md);
                result = SUCCESS;
            }
        }
    }

    if ((result == FAILURE))
    {
    	if((c->defaultMessageHandler != NULL))
    	{
            __MQTTClient_NewMessageData(&md, topicName, message);
            c->defaultMessageHandler(&md);
            result = SUCCESS;
    	}
    	else
    	{
    		ESP_LOGE(LOG_HEADER, "Failed delivering MQTT message received");
    	}
    }

    return result;
}

static int __MQTTClient_keepalive(MQTTClient* c)
{
    int result = SUCCESS;
    MQTTmbedtls_Timer pingRequestTimer;
    int byteCounter;

    if (c->keepAliveInterval == 0)
    {
    	/* The client doen't have keep alive interval, and so it doesn't need ping */
        result = SUCCESS;
    }

    /* Initilize ping request timer, the ping request must be sent in 1 second */
    TimerInit(&pingRequestTimer);
    __MQTTClient_TimerStart(&pingRequestTimer, SEND_COMMAND_TIMEOUT_MS);

    if (__MQTTClient_TimerIsExpired(&c->ping_timer))
    {
    	if(c->ping_outstanding == 1)
        {
#ifdef DEBUG_MQTT_CLIENT
            ESP_LOGE(LOG_HEADER, "Error receiving PING response");
#endif /* DEBUG_MQTT_CLIENT */
            c->ping_outstanding = 0;
            result = FAILURE;
        }
    	else if((byteCounter = MQTTSerialize_pingreq(c->buf, c->buf_size)) <= 0)
        {
#ifdef DEBUG_MQTT_CLIENT
            ESP_LOGE(LOG_HEADER, "Error serializing ping request");
#endif /* DEBUG_MQTT_CLIENT */
            result = FAILURE;
        }
        else if (__MQTTClient_sendPacket(c, byteCounter, &pingRequestTimer) != SUCCESS)
        {
#ifdef DEBUG_MQTT_CLIENT
            ESP_LOGE(LOG_HEADER, "Error sending PING");
#endif /* DEBUG_MQTT_CLIENT */
            result = FAILURE;
        }
        else
        {
            /* Restart ping timer */
            __MQTTClient_TimerStart(&c->ping_timer, PING_PERIOD_MS);
            c->ping_outstanding = 1;
#ifdef DEBUG_MQTT_CLIENT
            ESP_LOGI(LOG_HEADER, "PING SENT");
#endif /* DEBUG_MQTT_CLIENT */
        }
    }

    return result;
}

static int __MQTTClient_cycle(MQTTClient* c, MQTTmbedtls_Timer* timer)
{

	unsigned short packet_type;
	int byteCounter = 0;
	int result = SUCCESS;
	int intQoS;
	MQTTString topicName;
	MQTTMessage msg;
	unsigned short mypacketid;
	unsigned char dup;
	unsigned char type;

	/* read the socket, see what work is due */
	packet_type = __MQTTClient_readPacket(c, timer);

	switch (packet_type)
	{
	case CONNACK:
#ifdef DEBUG_MQTT_CLIENT
		ESP_LOGI(LOG_HEADER, "CONNACK packet received");
#endif /* DEBUG_MQTT_CLIENT */
		break;

	case PUBACK:
#ifdef DEBUG_MQTT_CLIENT
		ESP_LOGI(LOG_HEADER, "PUBACK packet received, packet id: %d", mypacketid);
#endif /* DEBUG_MQTT_CLIENT */
		break;

	case SUBACK:
#ifdef DEBUG_MQTT_CLIENT
		ESP_LOGI(LOG_HEADER, "SUBACK packet received");
#endif /* DEBUG_MQTT_CLIENT */
		break;

	case PUBLISH:
	{
#ifdef DEBUG_MQTT_CLIENT
		ESP_LOGI(LOG_HEADER, "PUBLISH packet received");
#endif /* DEBUG_MQTT_CLIENT */

		if (MQTTDeserialize_publish(&msg.dup, &intQoS, &msg.retained, &msg.id, &topicName, (unsigned char**)&msg.payload, (int*)&msg.payloadlen, c->readbuf, c->readbuf_size) != 1)
		{
#ifdef DEBUG_MQTT_CLIENT
			ESP_LOGE(LOG_HEADER, "Error deserializing received publish");
#endif /* DEBUG_MQTT_CLIENT */
			result = FAILURE;
		}
		else
		{
			msg.qos = (enum QoS)intQoS;
			__MQTTClient_deliverMessage(c, &topicName, &msg);
		}
		if((msg.qos == QOS1) && ((byteCounter = MQTTSerialize_ack(c->buf, c->buf_size, PUBACK, 0, msg.id)) <= 0))
		{
#ifdef DEBUG_MQTT_CLIENT
			ESP_LOGE(LOG_HEADER, "Error serializing publish acknowledgment");
#endif /* DEBUG_MQTT_CLIENT */
			result = FAILURE;
		}
		else if((msg.qos == QOS2) && ((byteCounter = MQTTSerialize_ack(c->buf, c->buf_size, PUBREC, 0, msg.id)) <= 0))
		{
#ifdef DEBUG_MQTT_CLIENT
			ESP_LOGE(LOG_HEADER, "Error serializing publish acknowledgment");
#endif /* DEBUG_MQTT_CLIENT */
			result = FAILURE;
		}
		else if(__MQTTClient_sendPacket(c, byteCounter, timer) == FAILURE)
		{
#ifdef DEBUG_MQTT_CLIENT
			ESP_LOGE(LOG_HEADER, "Error sending PUBREC acknowledgment");
#endif /* DEBUG_MQTT_CLIENT */
			result = FAILURE;
		}
		break;
	}
	case PUBREC:
	{
#ifdef DEBUG_MQTT_CLIENT
		ESP_LOGI(LOG_HEADER, "PUBREC packet received");
#endif /* DEBUG_MQTT_CLIENT */

		if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
		{
#ifdef DEBUG_MQTT_CLIENT
			ESP_LOGE(LOG_HEADER, "Error deserializing publish acknowledgment");
#endif /* DEBUG_MQTT_CLIENT */
			result = FAILURE;
		}
		else if((byteCounter = MQTTSerialize_ack(c->buf, c->buf_size, PUBREL, 0, mypacketid)) <= 0)
		{
#ifdef DEBUG_MQTT_CLIENT
			ESP_LOGE(LOG_HEADER, "Error serializing publish acknowledgment");
#endif /* DEBUG_MQTT_CLIENT */
			result = FAILURE;
		}
		else if (__MQTTClient_sendPacket(c, byteCounter, timer) != SUCCESS) // send the PUBREL packet
		{
#ifdef DEBUG_MQTT_CLIENT
			ESP_LOGE(LOG_HEADER, "Error sending PUBREL acknowledgment");
#endif /* DEBUG_MQTT_CLIENT */
			result = FAILURE;
		}
		break;
	}
	case PUBCOMP:
#ifdef DEBUG_MQTT_CLIENT
		ESP_LOGI(LOG_HEADER, "PUBCOMP packet received");
#endif /* DEBUG_MQTT_CLIENT */
		break;

	case PINGRESP:
#ifdef DEBUG_MQTT_CLIENT
		ESP_LOGI(LOG_HEADER, "PINGRESP packet received");
#endif /* DEBUG_MQTT_CLIENT */
		c->ping_outstanding = 0;
		break;

	case DISCONNECT:
#ifdef DEBUG_MQTT_CLIENT
		ESP_LOGI(LOG_HEADER, "DISCONNECT packet received");
#endif /* DEBUG_MQTT_CLIENT */
		result = FAILURE;
		break;
	}

	/* Call the function to send ping if necessary */
	if(result != SUCCESS)
	{
		return FAILURE;
	}
	else
	{
		return packet_type;
	}
}

int __MQTTClient_waitfor(MQTTClient* c, int packet_type, MQTTmbedtls_Timer* timer)
{
	int result = SUCCESS;

	while((__MQTTClient_cycle(c, timer) != packet_type) && (result != FAILURE))
	{
		if(__MQTTClient_TimerIsExpired(timer))
		{
			result = FAILURE;
		}
		else
		{
			vTaskDelay(WAIT_SLEEP_MS/portTICK_PERIOD_MS);
		}
	}
	if(result == SUCCESS)
	{
		return packet_type;
	}
	else
	{
		return FAILURE;
	}
}

static void __MQTTClient_NewMessageData(MessageData* md, MQTTString* aTopicName, MQTTMessage* aMessage)
{
    md->topicName = aTopicName;
    md->message = aMessage;
}

static int __MQTTClient_getNextPacketId(MQTTClient *c)
{
    return c->next_packetid = (c->next_packetid == MAX_PACKET_ID) ? 1 : c->next_packetid + 1;
}

static void __MQTTClient_TimerStart(MQTTmbedtls_Timer* timer, unsigned int timeout_ms)
{
	uint64_t timeNow_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
	uint64_t endTime_ms = timeNow_ms + timeout_ms;

	timer->end_time.tv_sec = endTime_ms / 1000;
	timer->end_time.tv_usec = (endTime_ms % 1000) * 1000;
}

static bool __MQTTClient_TimerIsExpired(MQTTmbedtls_Timer* timer)
{
	uint64_t endTime_ms = (timer->end_time.tv_sec * 1000) + (timer->end_time.tv_usec / 1000);
	uint64_t timeNow_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

	return (timeNow_ms >= endTime_ms);
}

static uint64_t __MQTTClient_TimerLeftMS(MQTTmbedtls_Timer* timer)
{
	uint64_t endTime_ms = (timer->end_time.tv_sec * 1000) + (timer->end_time.tv_usec / 1000);
	uint64_t timeNow_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
	uint64_t timerLeft_ms;

	if(endTime_ms > timeNow_ms)
	{
		timerLeft_ms = endTime_ms - timeNow_ms;
	}
	else
	{
		timerLeft_ms = 0;
	}

	return timerLeft_ms;
}

