#define CURL_STATICLIB
#include <string>
#include <chrono>

#include "curl/curl.h"
#include "include/nlohmann/json.hpp"
#include "include/DiscordHandler.h"
#include "include/helper.h"

using json = nlohmann::json;

int main()
{
	std::string t_ConfluenceBaseURL;
	std::string t_ConfluenceAuthorisation;
	std::string t_DiscordWebhook;
	int t_UpdateInterval = 1;

	HandleUserInput(t_ConfluenceBaseURL, t_ConfluenceAuthorisation, t_DiscordWebhook, t_UpdateInterval);

	std::string t_SearchQuery = t_ConfluenceBaseURL;
	t_SearchQuery.append("/rest/api/content/search?cql=label=decisions");

	std::chrono::system_clock::time_point t_LastUpdate = std::chrono::system_clock::now();

	//TODO improve escape functionality
	while (true)
	{
		// if time elapsed is bigger than UpdateInterval in ms
		auto t_TimeSinceUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - t_LastUpdate);

		if (t_TimeSinceUpdate.count() > 60000 * t_UpdateInterval)
		{
			t_LastUpdate = std::chrono::system_clock::now();

			// get data from confluence using search query
			json data = GetContentByURL(t_SearchQuery.c_str(), t_ConfluenceAuthorisation);

			for (auto it = data.begin(); it != data.end(); ++it)
			{
				for (auto& el : it->items())
				{
					if (el.value().contains("title"))
					{
						DiscordHandler t_Discord = {t_DiscordWebhook.c_str()};

						std::string t_ID = el.value().at("id");

						// get expanded data to receive decision status & when the page was last updated
						std::string t_URL = t_ConfluenceBaseURL;
						t_URL.append("/rest/api/content/" + t_ID + "?expand=body.storage,history.lastUpdated");
						json expanded_data = GetContentByURL(t_URL.c_str(), t_ConfluenceAuthorisation);

						// calculate time difference between now and the time the page was updated
						std::string t_LastUpdate = expanded_data.at("history").at("lastUpdated").at("when");
						std::chrono::system_clock::time_point t_UpdateTimePoint = string_to_time_point(t_LastUpdate);

						auto t_TimeElapsed = std::chrono::duration_cast<std::chrono::minutes>(
							std::chrono::system_clock::now() - t_UpdateTimePoint);

						std::string t_BodyData = expanded_data.at("body").at("storage").at("value");

						std::string t_Description = t_ConfluenceBaseURL;
						t_Description.append(el.value().at("_links").at("webui"));

						//TODO rework, using >< from the surrounding html tags is scuffed but works for now
						bool t_Decided = ContainsWord(t_BodyData, ">Decided<");
						bool t_InProgress = ContainsWord(t_BodyData, ">In progress<");

						if ((t_Decided || t_InProgress) &&
							t_TimeElapsed.count() < t_UpdateInterval)
						{

							std::string t_Title = "__A decision has been made:__ ";
							std::string t_Color = "65280";

							t_Title.append(el.value().at("title"));

							if (t_InProgress)
							{
								t_Title = "__A decision is in Progress:__ ";
								t_Title.append(el.value().at("title"));
								t_Color = "15844367";
							}
							
							std::string json_payload = std::string("{\"username\": \"Confluence Bot\", "
								"\"embeds\": [{\"title\": \"") + t_Title + ("\", "
								"\"description\": \"" + t_Description + "\", "
								"\"color\": \"" + t_Color + "\"}]}");

							t_Discord.Send(json_payload.c_str());
						}
					}
				}
			}
		}
	}

	curl_global_cleanup();
}
