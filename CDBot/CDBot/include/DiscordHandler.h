#pragma once
#include <curl/curl.h>

class DiscordHandler
{
public:
	DiscordHandler(const char* a_Webhook);
	~DiscordHandler();

	CURLcode Send(const char* a_Payload) const;

private:
	const char* m_Webhook;
	curl_slist *m_HTTPHeader = nullptr;
	CURL* m_CurlInstance;
};

