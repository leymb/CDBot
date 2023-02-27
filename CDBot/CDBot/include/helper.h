#include <chrono>
#include <iostream>
#include <regex>
#include <sstream>
#include "include/nlohmann/json.hpp"

using json = nlohmann::json;

inline std::chrono::system_clock::time_point string_to_time_point(const std::string& str)
{
	// Parse the year, month, day, hour, minute, and second from the input string
	int year, month, day, hour, minute, second;
	std::stringstream ss(str);
	ss >> year;
	ss.ignore(1); // Ignore the dash separator
	ss >> month;
	ss.ignore(1);
	ss >> day;
	ss.ignore(1);
	ss >> hour;
	ss.ignore(1);
	ss >> minute;
	ss.ignore(1);
	ss >> second;

	// Convert the parsed values into a std::tm struct
	std::tm time_struct = {0};
	time_struct.tm_year = year - 1900; // Years since 1900
	time_struct.tm_mon = month - 1; // Months since January (0-11)
	time_struct.tm_mday = day; // Day of the month (1-31)
	time_struct.tm_hour = hour; // Hours since midnight (0-23)
	time_struct.tm_min = minute; // Minutes after the hour (0-59)
	time_struct.tm_sec = second; // Seconds after the minute (0-60)

	// Convert the std::tm struct into a std::time_t value
	std::time_t time = std::mktime(&time_struct);

	// Convert the std::time_t value into a std::chrono::system_clock time_point
	return std::chrono::system_clock::from_time_t(time);
}

// function taken from https://stackoverflow.com/questions/44994203/how-to-get-the-http-response-string-using-curl-in-c
inline size_t WriteCallback(char* contents, size_t size, size_t nmemb, void* userp)
{
	static_cast<std::string*>(userp)->append(contents, size * nmemb);
	return size * nmemb;
}

inline json GetContentByURL(const char* a_URL, const std::string& a_ConfluenceAuthorisation)
{
	std::string t_Result;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_ALL);
	CURL* t_curl_instance = curl_easy_init();

	struct curl_slist* t_Headers = nullptr;

	if (t_curl_instance)
	{
		curl_easy_setopt(t_curl_instance, CURLOPT_URL, a_URL);

		// add header data
		std::string t_Authorization =  "Authorization: Bearer ";
		t_Authorization.append(a_ConfluenceAuthorisation);
		t_Headers = curl_slist_append(t_Headers,  t_Authorization.c_str());
		t_Headers = curl_slist_append(t_Headers, "Content-Type: application/json");

		curl_easy_setopt(t_curl_instance, CURLOPT_HTTPHEADER, t_Headers);
		curl_easy_setopt(t_curl_instance, CURLOPT_HTTPGET, 1);
		curl_easy_setopt(t_curl_instance, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(t_curl_instance, CURLOPT_WRITEDATA, &t_Result);
		curl_easy_setopt(t_curl_instance, CURLOPT_VERBOSE, 1L);
		res = curl_easy_perform(t_curl_instance);
		curl_easy_cleanup(t_curl_instance);
		if (CURLE_OK != res)
		{
			std::cerr << "CURL error: " << res << '\n';
		}
	}

	json data = json::parse(t_Result);
	curl_global_cleanup();

	delete t_Headers;

	return data;
}

inline bool ContainsWord(std::string& a_String, const char* a_Word)
{
	std::regex rgx(a_Word);
	std::smatch t_Match;

	return std::regex_search(a_String, t_Match, rgx);
}

inline void HandleUserInput(std::string& a_ConfluenceBaseURL, std::string& a_ConfluenceAuthorisation, std::string& a_DiscordWebhookURL, int& a_DurationInMin)
{
	std::cout << "Welcome to CDBot!" << std::endl;
	std::cout << "This Bot will connect to your Confluence Server instance and output your decisions to Discord." << std:: endl;

	std::cout << "Please input your Confluence Servers base URL: " << std::endl;
	std::cin >> a_ConfluenceBaseURL;

	std::cout << "Please input your personal Confluence Authorisation Token: " << std::endl;
	std::cin >> a_ConfluenceAuthorisation;

	std::cout << "Please input your Discord Webhook URL: " << std::endl;
	std::cin >> a_DiscordWebhookURL;

	std::cout << "Please input the desired update interval in minutes" << std::endl;
	std::cin >> a_DurationInMin;

	std::cout << "Configuration finished. The bot will now run every " << a_DurationInMin << "min." << std::endl;

	std::string t_Temp;

	while (t_Temp != "y")
	{
		std::cout << "\nContinue? y:n" << std::endl;
		std::cin >> t_Temp;
	}
}
