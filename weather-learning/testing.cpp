#include <iostream>
#include <algorithm>
#include <cctype>

#include "json.hpp"
#include "cpr/cpr.h"


/*****************************************************************************
 * Testing dynamically built API calls and parsing JSON data in responses to *
 * output various weather data. CLI asks user to enter city or zip code for  *
 * which to fetch data and outputs a description, temperature, humidity, the *
 * high and low for the day, and what it feels like outside.                 *
 *****************************************************************************/


int main(){
    // const variable for api key
    const char* apiKey = std::getenv("OPENWEATHERMAP_API_KEY");
    if (!apiKey){
        std::cerr << "Error: OPENWEATHERMAP_API_KEY environment variable not set.\n";
        return 1;
    }
    std::string API_KEY(apiKey);      
  
  
    // cincy latitude and longitude
    std::string lat{"39.1014537"};
    std::string lon{"-84.5124602"};
    // api call url
    std::string url = "https://api.openweathermap.org/data/3.0/onecall?lat=" + lat + "&lon=" + lon + "&units=imperial&appid=" + API_KEY;

    // get request and store response in _response_

    cpr::Response response = cpr::Get(cpr::Url{url});

/*  std::cout << "Status: " << response.status_code << std::endl;
    if (response.status_code == 200){
        std::cout << response.text << std::endl;
    } else{ 
        std::cout << "Error with request" << std::endl;
    }
*/  

    /*
    * geocoding request format (city): https://api.openweathermap.org/geo/1.0/direct?q={city name}&limit=5&appid={API key}
    * geocoding request format (zip): https://api.openweathermap.org/geo/1.0/zip?zip={zip code},{country code}&appid={API key}
    */

    // string variables for weather call url and city/zip
    std::string weatherURL{"https://api.openweathermap.org/data/3.0/onecall?lat="}, cityOrZip;    
    
    std::cout << "Enter your city or ZIP code (US): ";       // prompt for city or zip
    std::cin >> cityOrZip;                                   // take in city or zip

    std::string geocodeURL;
    //different paths if given city name or zip code
    if (std::all_of(cityOrZip.begin(), cityOrZip.end(), ::isdigit)){
        // build api call with zip code
        geocodeURL = "https://api.openweathermap.org/geo/1.0/zip?zip=" + cityOrZip + ",US&appid=" + API_KEY;
        
    } else{
        // build api call with city name
        geocodeURL = "https://api.openweathermap.org/geo/1.0/direct?q=" + cityOrZip + "&limit=5&appid=" + API_KEY;
    }
    // get geocode JSON object
    cpr::Response geocode = cpr::Get(cpr::Url{geocodeURL});
    // initialize latitude and longitude variables
    std::string geolat, geolon, cityName;

    //std::cout << geocode.text << std::endl;      // display geocode JSON data

    // parse geocode data

    nlohmann::json geocodeObject = nlohmann::json::parse(geocode.text);

    //std::cout << geocodeObject.dump(2);

    if (std::all_of(cityOrZip.begin(), cityOrZip.end(), ::isdigit)){
        geolat = std::to_string(geocodeObject["lat"].get<double>());
        geolon = std::to_string(geocodeObject["lon"].get<double>());
        cityName = geocodeObject["name"].get<std::string>();
    } else {
        geolat = std::to_string(geocodeObject[0]["lat"].get<double>());
        geolon = std::to_string(geocodeObject[0]["lon"].get<double>());
        cityName = cityOrZip;
    }
    //std::cout << geocodeObject.dump(2) << std::endl;



    

    // build final api call url
    weatherURL += (geolat + "&lon=" + geolon + "&units=imperial&appid=" + API_KEY);
   
    /* debugging: confirm the url was built accurately
    *
    * std::cout << "[Test] Manual URL: " << url << std::endl;
    * std::cout << "[Test] Geocoded URL: " << weatherURL << std::endl;
    * 
    */

    // final request
    cpr::Response weatherData = cpr::Get(cpr::Url(weatherURL));


    // parse weatherData JSON object

    nlohmann::json parsedWeatherData = nlohmann::json::parse(weatherData.text);



    //std::cout << parsedWeatherData.dump(2) << std::endl;

    int currentTemp, currentFeelsLike, currentHumidity, todayHigh, todayLow;
    std::string currentDescription;

    currentTemp = static_cast<int>(std::round(parsedWeatherData["current"]["temp"].get<double>()));
    currentFeelsLike = static_cast<int>(std::round(parsedWeatherData["current"]["feels_like"].get<double>()));
    currentHumidity = static_cast<int>(std::round(parsedWeatherData["current"]["humidity"].get<double>()));
    todayHigh = static_cast<int>(std::round(parsedWeatherData["daily"][0]["temp"]["max"].get<double>()));
    todayLow = static_cast<int>(std::round(parsedWeatherData["daily"][0]["temp"]["min"].get<double>()));
    currentDescription = parsedWeatherData["current"]["weather"][0]["description"].get<std::string>();
    

    std::cout << "It is currently " << currentDescription << " and " << currentTemp << " degrees outside in " << cityName << ".\n";
    std::cout << "The humidity level is " << currentHumidity << "% and it feels like " << currentFeelsLike << " degrees.\n";
    std::cout << "Today's High: " << todayHigh << "\nToday's Low: " << todayLow << std::endl;

    return 0;
}