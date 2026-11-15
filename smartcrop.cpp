
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
using namespace std;

// FUNCTION DECLARATIONS
int askChoice(int lowest, int highest);
string chooseLocation();
string chooseSeason();
string chooseCrop(string season);
bool readCrop(string crop, double tempNeed[], double phNeed[], double waterNeed[]);
int getFirstYear(string season);
int getMonth(int day, int year);
string getSeason(int month);
int getSeasonDays(string season, int year);
bool readOneSeason(string location, string season, int yearToRead,
    double temp[], double rain[], int days[], int i);
double average(double numbers[], int size);
double smallest(double numbers[], int size);
double largest(double numbers[], int size);
string tempResult(double temp, double need[]);
string phResult(double ph, double need[]);
string temperatureAdvice(double temp, double need[]);
string soilAdvice(double ph, double need[]);
string waterAdvice(double irrigation);
void showTemperature(double temp[], double need[]);
void showSoil(double soilPH, double need[]);
void showWater(double rain[], double irrigation, double need[]);
void saveReport(string farmer, string location, string season, string crop,
    int firstYear, double temp[], double rain[],
    double soilPH, double irrigation,
    double tempNeed[], double phNeed[], double waterNeed[]);

int main() {
    cout << "========== SMARTCROP ==========\n";

    string farmer;
    cout << "Farmer's name: ";
    getline(cin, farmer);

    string location = chooseLocation();
    string season = chooseSeason();
    string crop = chooseCrop(season);

    // Four numbers for temperature, four for pH, two for water.
    double tempNeed[4], phNeed[4], waterNeed[2];

    if (!readCrop(crop, tempNeed, phNeed, waterNeed)) {
        cout << "Could not read crop_requirements.csv.\n";
        return 1;
    }

    double soilPH, irrigation;
    cout << "Enter your soil pH (0 to 14): ";
    cin >> soilPH;
    while (soilPH < 0 || soilPH > 14) {
        cout << "Enter soil pH from 0 to 14: ";
        cin >> soilPH;
    }

    cout << "Enter planned irrigation for this season (mm): ";
    cin >> irrigation;
    while (irrigation < 0) {
        cout << "Irrigation cannot be negative. Try again: ";
        cin >> irrigation;
    }

    // One array entry for each of the four previous seasons.
    double temperatures[4] = { 0 };
    double rainfall[4] = { 0 };
    int days[4] = { 0 };
    int firstYear = getFirstYear(season);

    // Call the SAME small function four times, once per past season.
    for (int i = 0; i < 4; i++) {
        int yearToRead = firstYear + i;
        if (!readOneSeason(location, season, yearToRead,
            temperatures, rainfall, days, i)) {
            cout << "Could not read " << location << ".csv.\n";
            return 1;
        }
        if (days[i] != getSeasonDays(season, yearToRead)) {
            cout << "Some weather days are missing in " << season
                << " " << yearToRead << ".\n";
            return 1;
        }
        temperatures[i] = temperatures[i] / days[i];
    }

    cout << fixed << setprecision(2);
    cout << "\n========== RESULTS ==========\n";
    cout << "Farmer: " << farmer << "\n";
    cout << "Location: " << location << ", Free State\n";
    cout << "Crop: " << crop << " | Season: " << season << "\n";

    cout << "\nPREVIOUS FOUR SEASONS\n";
    for (int i = 0; i < 4; i++) {
        cout << firstYear + i;
        if (season == "Summer") cout << "/" << firstYear + i + 1;
        cout << "  Temperature: " << temperatures[i] << " C"
            << "  Rainfall: " << rainfall[i] << " mm\n";
    }

    showTemperature(temperatures, tempNeed);
    showSoil(soilPH, phNeed);
    showWater(rainfall, irrigation, waterNeed);

    saveReport(farmer, location, season, crop, firstYear, temperatures,
        rainfall, soilPH, irrigation, tempNeed, phNeed, waterNeed);
    return 0;
}

// Ask the user for a menu number.
int askChoice(int lowest, int highest) {
    int choice;
    cin >> choice;
    while (choice < lowest || choice > highest) {
        cout << "Choose a number from " << lowest << " to " << highest << ": ";
        cin >> choice;
    }
    return choice;
}

string chooseLocation() {
    string locations[8] = { "Bethlehem", "Welkom", "Bloemfontein", "Kroonstad",
                           "Bothaville", "Ficksburg", "Harrismith", "Sasolburg" };
    cout << "\nChoose location:\n";
    for (int i = 0; i < 8; i++) cout << i + 1 << ". " << locations[i] << "\n";
    cout << "Your choice: ";
    return locations[askChoice(1, 8) - 1];
}

string chooseSeason() {
    string seasons[4] = { "Summer", "Autumn", "Winter", "Spring" };
    cout << "\nChoose season:\n";
    for (int i = 0; i < 4; i++) cout << i + 1 << ". " << seasons[i] << "\n";
    cout << "Your choice: ";
    return seasons[askChoice(1, 4) - 1];
}

string chooseCrop(string season) {
    string crops[5];
    int count = 0;
    if (season == "Summer") {
        crops[0] = "sunflowers"; crops[1] = "groundnuts";
        crops[2] = "sorghum"; crops[3] = "dry beans";
        crops[4] = "maize"; count = 5;
    }
    else if (season == "Spring") {
        crops[0] = "maize"; crops[1] = "soybeans";
        crops[2] = "sunflowers"; crops[3] = "sorghum";
        crops[4] = "dry beans"; count = 5;
    }
    else if (season == "Autumn") {
        crops[0] = "wheat"; crops[1] = "oats"; count = 2;
    }
    else {
        crops[0] = "wheat"; crops[1] = "oats";
        crops[2] = "barley"; count = 3;
    }
    cout << "\nChoose crop:\n";
    for (int i = 0; i < count; i++) cout << i + 1 << ". " << crops[i] << "\n";
    cout << "Your choice: ";
    return crops[askChoice(1, count) - 1];
}

// Crop CSV columns: crop, seasons, 4 temperature numbers,
// 4 pH numbers, 2 water numbers, and source websites.
bool readCrop(string crop, double tempNeed[], double phNeed[], double waterNeed[]) {
    ifstream file("crop_requirements.csv");
    if (!file.is_open()) return false;

    string line, name, seasons;
    getline(file, line); // Skip column headings.
    char comma;
    double values[10];

    while (getline(file, name, ',')) {
        getline(file, seasons, ',');

        if (name == crop) {
            for (int i = 0; i < 10; i++) {
                if (!(file >> values[i] >> comma)) return false;
            }
            for (int i = 0; i < 4; i++) tempNeed[i] = values[i];
            for (int i = 0; i < 4; i++) phNeed[i] = values[i + 4];
            waterNeed[0] = values[8];
            waterNeed[1] = values[9];
            return true;
        }
        getline(file, line); // Skip other crops.
    }
    return false;
}

// Which four historical years are available?
int getFirstYear(string season) {
    if (season == "Summer" || season == "Spring") return 2022;
    return 2023;
}

// Convert a day of the year (1 to 365/366) into a month (1 to 12).
int getMonth(int day, int year) {
    int monthDays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (year % 4 == 0) monthDays[1] = 29; // Leap year in our data: 2024.
    int month = 1;
    while (month <= 12 && day > monthDays[month - 1]) {
        day = day - monthDays[month - 1];
        month++;
    }
    return month;
}

string getSeason(int month) {
    if (month == 12 || month <= 2) return "Summer";
    if (month <= 5) return "Autumn";
    if (month <= 8) return "Winter";
    return "Spring";
}

int getSeasonDays(string season, int year) {
    if (season == "Autumn" || season == "Winter") return 92;
    if (season == "Spring") return 91;
    // Summer includes December, followed by January and February.
    if ((year + 1) % 4 == 0) return 91;
    return 90;
}

// Read ONE past season from the six-column NASA CSV file.
// We call this function four times, once for each historical year.
bool readOneSeason(string location, string season, int yearToRead,
    double temp[], double rain[], int days[], int i) {
    ifstream file(location + ".csv");
    if (!file.is_open()) return false;

    string text, dateText;
    getline(file, text); // Skip: YEAR, DOY, T_Average, T_Max, T_Min, Rainfall.

    int year, previousYear = 0, day = 0;
    double temperature, hottest, coldest, rainfall;
    char comma;

    while (file >> year >> comma) {
        getline(file, dateText, ','); // Skip the DOY column.
        if (!(file >> temperature >> comma >> hottest >> comma
            >> coldest >> comma >> rainfall)) return false;
        file.ignore(1000, '\n'); // Go to the next row.

        if (year != previousYear) {
            day = 0; // Start counting days again in January.
            previousYear = year;
        }
        day++;

        int month = getMonth(day, year);
        int startYear = year;
        if (month == 1 || month == 2) startYear = year - 1;

        if (getSeason(month) == season && startYear == yearToRead) {
            if (temperature > -900 && hottest > -900 &&
                coldest > -900 && rainfall >= 0) {
                temp[i] = temp[i] + temperature;
                rain[i] = rain[i] + rainfall;
                days[i]++;
            }
        }
    }
    file.close();
    return true;
}

double average(double numbers[], int size) {
    double total = 0;
    for (int i = 0; i < size; i++) total = total + numbers[i];
    return total / size;
}

double smallest(double numbers[], int size) {
    double answer = numbers[0];
    for (int i = 1; i < size; i++) {
        if (numbers[i] < answer) answer = numbers[i];
    }
    return answer;
}

double largest(double numbers[], int size) {
    double answer = numbers[0];
    for (int i = 1; i < size; i++) {
        if (numbers[i] > answer) answer = numbers[i];
    }
    return answer;
}

string tempResult(double temp, double need[]) {
    if (temp < need[0]) return "Below the recorded temperature range";
    if (temp < need[1]) return "Colder than ideal";
    if (temp <= need[2]) return "Within the ideal temperature range";
    if (temp <= need[3]) return "Hotter than ideal";
    return "Above the recorded temperature range";
}

string phResult(double ph, double need[]) {
    if (ph < need[0]) return "Below the recorded soil pH range";
    if (ph < need[1]) return "More acidic than ideal";
    if (ph <= need[2]) return "Within the ideal soil pH range";
    if (ph <= need[3]) return "More alkaline than ideal";
    return "Above the recorded soil pH range";
}

// These short advice functions contain research-based explanations.
// They return words, not scores. This keeps the report easy to understand.
string temperatureAdvice(double value, double need[]) {
    if (value < need[0]) {
        return "The historical average is below this crop's recorded temperature range. "
            "Very cool conditions can slow growth and development, and cold periods "
            "may damage sensitive plants. Check the local temperature forecast before "
            "planting and ask a local agricultural adviser about the best planting time.";
    }
    else if (value < need[1]) {
        return "The historical average is colder than the crop's ideal range. "
            "Cool conditions can slow plant growth and delay development. "
            "Check the planting time and local forecast so that the crop can grow "
            "during warmer conditions, where possible.";
    }
    else if (value <= need[2]) {
        return "The average temperature of the past four seasons is within the crop's "
            "ideal range. This is encouraging for temperature, but unusually hot or "
            "cold days may still occur. Keep checking the weather during the season.";
    }
    else if (value <= need[3]) {
        return "The historical average is warmer than the crop's ideal range. "
            "High temperatures can stress plants, raise their water needs and "
            "sometimes affect flowering or grain production. Watch soil moisture "
            "and ask about locally suitable planting dates or crop varieties.";
    }
    else {
        return "The historical average is above the crop's recorded temperature range. "
            "Very hot conditions can seriously limit growth and increase water use. "
            "Ask a local agricultural adviser whether this planting season or "
            "variety is appropriate, and check how irrigation will be managed.";
    }
}

string soilAdvice(double value, double need[]) {
    if (value < need[0]) {
        return "Your soil is more acidic than the crop's recorded tolerance range. "
            "Strongly acidic soils can make some nutrients harder for roots to take up "
            "and may increase harmful aluminium levels. This can restrict root growth "
            "and reduce crop performance. Get a soil test and local advice before "
            "adding lime or any other soil treatment.";
    }
    else if (value < need[1]) {
        return "Your soil is more acidic than the crop's ideal range. "
            "Acidity can change how easily roots obtain nutrients such as phosphorus. "
            "The crop may still grow, but nutrient problems could affect its growth. "
            "Have the soil tested before deciding whether it needs treatment.";
    }
    else if (value <= need[2]) {
        return "Your soil pH is within the crop's ideal range. This is good for the "
            "pH part of growing conditions and generally supports nutrient availability. "
            "However, pH alone cannot tell us whether the soil contains enough "
            "nutrients, so regular soil testing is still useful.";
    }
    else if (value <= need[3]) {
        return "Your soil is more alkaline than the crop's ideal range. "
            "High pH can make nutrients such as iron, zinc and phosphorus less "
            "available, which may affect healthy growth. A soil test can show "
            "whether nutrient problems are likely. Ask an adviser before changing pH.";
    }
    else {
        return "Your soil is more alkaline than the crop's recorded tolerance range. "
            "At very high pH, important nutrients may become difficult for plants "
            "to absorb, and growth may suffer. Arrange a soil test and get local "
            "advice before applying any soil treatment.";
    }
}

string waterAdvice(double irrigation) {
    if (irrigation == 0) {
        return "You have not planned any irrigation for this season, so the crop would "
            "depend on rain and moisture already stored in the soil. In a dry period, "
            "a lack of water can slow growth and reduce production, especially at "
            "sensitive growth stages. Check soil moisture and consider a backup "
            "water source if dry conditions are expected.";
    }
    return "You have planned irrigation, which may help the crop during dry periods. "
        "However, crops need water at the right time, not only a large total amount. "
        "Check rainfall and soil moisture before irrigating. Too little water can "
        "stress the crop, while too much can waste water or cause waterlogging "
        "where drainage is poor.";
}

void showTemperature(double temp[], double need[]) {
    double value = average(temp, 4);
    cout << "\nTEMPERATURE FEEDBACK\n";
    cout << "Crop ideal: " << need[1] << " to " << need[2] << " C. ";
    cout << "Historical average: " << value << " C.\n";
    cout << "Coolest past seasonal average: " << smallest(temp, 4) << " C. ";
    cout << "Warmest: " << largest(temp, 4) << " C.\n";
    cout << tempResult(value, need) << ". ";
    if (value < need[1]) cout << "The difference is " << need[1] - value << " C below ideal. ";
    if (value > need[2]) cout << "The difference is " << value - need[2] << " C above ideal. ";
    cout << "\n" << temperatureAdvice(value, need) << "\n";
}

void showSoil(double soilPH, double need[]) {
    cout << "\nSOIL pH FEEDBACK\n";
    cout << "Your soil pH: " << soilPH << ". Crop ideal: "
        << need[1] << " to " << need[2] << ".\n";
    cout << phResult(soilPH, need) << ". ";
    if (soilPH < need[1]) cout << "The difference is " << need[1] - soilPH << " below ideal. ";
    if (soilPH > need[2]) cout << "The difference is " << soilPH - need[2] << " above ideal. ";
    cout << "\n" << soilAdvice(soilPH, need) << "\n";
}

void showWater(double rain[], double irrigation, double need[]) {
    double meanRain = average(rain, 4);
    double lowRain = smallest(rain, 4);
    double highRain = largest(rain, 4);
    cout << "\nWATER FEEDBACK\n";
    cout << "Average historical rainfall for the chosen three months: "
        << meanRain << " mm.\n";
    cout << "Driest past season: " << lowRain << " mm. Wettest: "
        << highRain << " mm. Difference: " << highRain - lowRain << " mm.\n";
    cout << "Planned irrigation for those three months: " << irrigation << " mm.\n";
    cout << "Historical average rainfall + planned irrigation: "
        << meanRain + irrigation << " mm (before losses).\n";
    cout << "Crop water reference for its growing period: "
        << need[0] << " to " << need[1] << " mm.\n";
    cout << "Rainfall varied across the previous seasons. If a coming season is as dry "
        << "as the driest one, the crop may need closer water management. "
        << "Rainfall timing and soil drainage also matter.\n";
    cout << waterAdvice(irrigation) << "\n";
}

// Save a short text report in the same Working Directory.
void saveReport(string farmer, string location, string season, string crop,
    int firstYear, double temp[], double rain[],
    double soilPH, double irrigation,
    double tempNeed[], double phNeed[], double waterNeed[]) {
    ofstream file("SmartCrop_Report.txt");
    if (!file.is_open()) {
        cout << "Could not save the report.\n";
        return;
    }
    file << fixed << setprecision(2);
    file << "SMARTCROP REPORT\n";
    file << "Farmer: " << farmer << "\n";
    file << "Location: " << location << ", Free State, South Africa\n";
    file << "Season: " << season << " | Crop: " << crop << "\n\n";

    file << "HISTORICAL WEATHER\n";
    for (int i = 0; i < 4; i++) {
        file << firstYear + i;
        if (season == "Summer") file << "/" << firstYear + i + 1;
        file << ": average temperature " << temp[i]
            << " C, total rainfall " << rain[i] << " mm\n";
    }

    double meanTemp = average(temp, 4);
    file << "\nTEMPERATURE FEEDBACK\n";
    file << "Crop ideal: " << tempNeed[1] << " to " << tempNeed[2]
        << " C. Historical average: " << meanTemp << " C.\n";
    file << "Coolest past seasonal average: " << smallest(temp, 4)
        << " C. Warmest: " << largest(temp, 4) << " C.\n";
    file << tempResult(meanTemp, tempNeed) << ". ";
    if (meanTemp < tempNeed[1]) file << "The difference is " << tempNeed[1] - meanTemp << " C below ideal. ";
    if (meanTemp > tempNeed[2]) file << "The difference is " << meanTemp - tempNeed[2] << " C above ideal. ";
    file << "\n" << temperatureAdvice(meanTemp, tempNeed) << "\n";

    file << "\nSOIL pH FEEDBACK\n";
    file << "Your soil pH: " << soilPH << ". Crop ideal: "
        << phNeed[1] << " to " << phNeed[2] << ".\n";
    file << phResult(soilPH, phNeed) << ". ";
    if (soilPH < phNeed[1]) file << "The difference is " << phNeed[1] - soilPH << " below ideal. ";
    if (soilPH > phNeed[2]) file << "The difference is " << soilPH - phNeed[2] << " above ideal. ";
    file << "\n" << soilAdvice(soilPH, phNeed) << "\n";

    double meanRain = average(rain, 4);
    double lowRain = smallest(rain, 4);
    double highRain = largest(rain, 4);
    file << "\nWATER FEEDBACK\n";
    file << "Average historical rainfall for the chosen three months: "
        << meanRain << " mm.\n";
    file << "Driest past season: " << lowRain << " mm. Wettest: "
        << highRain << " mm. Difference: " << highRain - lowRain << " mm.\n";
    file << "Planned irrigation for those three months: " << irrigation << " mm.\n";
    file << "Historical average rainfall + planned irrigation: "
        << meanRain + irrigation << " mm (before losses).\n";
    file << "Crop water reference for its growing period: "
        << waterNeed[0] << " to " << waterNeed[1] << " mm.\n";
    file << "Rainfall varied across the previous seasons. If a coming season is as dry "
        << "as the driest one, the crop may need closer water management. "
        << "Rainfall timing and soil drainage also matter.\n";
    file << waterAdvice(irrigation) << "\n";

    // Source websites for the explanations and crop ranges.
    //file << "\nSOURCES FOR THE ADVICE\n";
    //file << "FAO Ecocrop (crop limits): "
    //    << "https://www.fao.org/land-water/resources/tools/databases/ecocrop/en\n";
    //file << "FAO Agronomic Factors (temperature): "
    //    << "https://www.fao.org/4/x5648e/x5648e0e.htm\n";
    //file << "FAO Soil Chemical Properties (soil pH): "
    //    << "https://www.fao.org/soils-portal/data-hub/soil-classification/numerical-systems/chemical-properties/en/\n";
    //file << "FAO Crop Water Needs (water): "
    //    << "https://www.fao.org/4/s2022e/s2022e07.htm\n";
    //file << "FAO Waterlogging (drainage): "
    //    << "https://www.fao.org/4/x8234e/x8234e08.htm\n";

    file.close();
    cout << "\nReport saved as SmartCrop_Report.txt\n";
}


