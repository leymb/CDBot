#include "../include/DiscordHandler.h"

#include <string>

DiscordHandler::DiscordHandler(const char* a_Webhook) : m_Webhook(a_Webhook), m_CurlInstance(curl_easy_init())
{
	if (m_CurlInstance)
	{
		curl_easy_setopt(m_CurlInstance, CURLOPT_URL, a_Webhook);

		m_HTTPHeader = curl_slist_append(m_HTTPHeader, "Content-Type: application/json");
		curl_easy_setopt(m_CurlInstance, CURLOPT_HTTPHEADER, m_HTTPHeader);
	}
}

DiscordHandler::~DiscordHandler()
{
	curl_easy_cleanup(m_CurlInstance);
}

CURLcode DiscordHandler::Send(const char* a_Payload) const
{
	std::string json_payload = std::string(a_Payload);

	curl_easy_setopt(m_CurlInstance, CURLOPT_POSTFIELDS, json_payload.c_str());

	return curl_easy_perform(m_CurlInstance);
}
