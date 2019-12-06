
#include "HTTPModule.h"


static void SendRequestTask(void *pvParameters)
{
	 const struct addrinfo hints = {
	        .ai_family = AF_INET,
	        .ai_socktype = SOCK_STREAM,
	    };
	    struct addrinfo *res;
	    struct in_addr *addr;
	    int s, r;
	    char recv_buf[64];
	    char* webServer;
	    char* data;
	    char *fullReq;
	    while(1) {
	    	char *curPos=fullReq;
	    	bool isAddonData=false;
	    	//Check for queue
	    	if (!xQueueReceive( reqQuee, &( fullReq ), ( TickType_t ) 10 ) )
	    	{
	    		continue;
	    	}
	    	//if string begins with htttp
	    	if (strncmp(fullReq,"http ",5)==0)
	    	{
	    		curPos+=5;
	    		if (strncmp(curPos,"GET ",4==0))
	    		{
	    			curPos+=4;
	    			webServer=curPos;
	    		}
	    		else if (strncmp(curPos,"POST ",5==0))
	    		{
	    			curPos+=5;
	    			data=strchr(curPos,' ');
	    			isAddonData=true;
	    			int addrLenngth=data-curPos;
	    			webServer=(char*)malloc(addrLenngth*sizeof(char));
	    			strncpy(curPos,webServer,addrLenngth);
	    			webServer[addrLenngth-1]='\0';
	    		}
	    		else
	    		{
	    			free(fullReq);
	    			continue;
	    		}
	    	}
	    	else
	    	{
	    		free(fullReq);
	    		continue;
	    	}
	    	char *bufferRequest[1024];
	    	int err = getaddrinfo(webServer+7, WEB_PORT, &hints, &res);
	    	if (isAddonData)
	    	{
		    	sprintf(bufferRequest,"%s%s%s%s%s%s%s%s%s%s%s","POST",WEB_PATH," HTTP/1.0\r\n",
		    			 "Host: ",webServer+7,":",WEB_PORT,"\r\n",
						 "User-Agent: esp-idf/1.0 esp32\r\n",
						 data,
						 "\r\n");
	    	}
	    	else
	    	{
	    		sprintf(bufferRequest,"%s%s%s%s%s%s%s%s%s%s","GET",WEB_PATH," HTTP/1.0\r\n",
	    			 "Host: ",webServer+7,":",WEB_PORT,"\r\n",
					 "User-Agent: esp-idf/1.0 esp32\r\n",
					 "\r\n");
	    	}
	        if(err != 0 || res == NULL) {
	            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
	            vTaskDelay(1000 / portTICK_PERIOD_MS);
	            continue;
	        }

	        /* Code to print the resolved IP.
	           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
	        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
	        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

	        s = socket(res->ai_family, res->ai_socktype, 0);
	        if(s < 0) {
	            ESP_LOGE(TAG, "... Failed to allocate socket.");
	            freeaddrinfo(res);
	            vTaskDelay(1000 / portTICK_PERIOD_MS);
	            continue;
	        }
	        ESP_LOGI(TAG, "... allocated socket");

	        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
	            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
	            close(s);
	            freeaddrinfo(res);
	            vTaskDelay(4000 / portTICK_PERIOD_MS);
	            continue;
	        }

	        ESP_LOGI(TAG, "... connected");
	        freeaddrinfo(res);

	        if (write(s, bufferRequest, strlen(bufferRequest)) < 0) {
	            ESP_LOGE(TAG, "... socket send failed");
	            close(s);
	            vTaskDelay(4000 / portTICK_PERIOD_MS);
	            continue;
	        }
	        ESP_LOGI(TAG, "... socket send success");

	        struct timeval receiving_timeout;
	        receiving_timeout.tv_sec = 5;
	        receiving_timeout.tv_usec = 0;
	        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
	                sizeof(receiving_timeout)) < 0) {
	            ESP_LOGE(TAG, "... failed to set socket receiving timeout");
	            close(s);
	            vTaskDelay(4000 / portTICK_PERIOD_MS);
	            continue;
	        }
	        ESP_LOGI(TAG, "... set socket receiving timeout success");

	        /* Read HTTP response */
	        do {
	            bzero(recv_buf, sizeof(recv_buf));
	            r = read(s, recv_buf, sizeof(recv_buf)-1);
	            for(int i = 0; i < r; i++) {
	                putchar(recv_buf[i]);
	            }
	        } while(r > 0);
            char *strToBuff=malloc(sizeof(recv_buf));
            strcpy(strToBuff,recv_buf);
            if (xQueueSendToFront(answQuee,strToBuff,(TickType_t)0)!=pdPASS)
            {
                    	ESP_LOGI(TAG, "Send queue full");
            }
	        ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d.", r, errno);
	        close(s);
	        for(int countdown = 10; countdown >= 0; countdown--) {
	            ESP_LOGI(TAG, "%d... ", countdown);
	            vTaskDelay(1000 / portTICK_PERIOD_MS);
	        }
	        ESP_LOGI(TAG, "Starting again!");
	        free(fullReq);
	        if (isAddonData)
	        {
	        	free(webServer);
	        }
	    }
}
