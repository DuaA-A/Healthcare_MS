#include <iostream>
#include <fstream>
#include <string>
#include "sstream"
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
using namespace std;

template <typename T>
int binarySearch(const vector<pair<T, int>>& index, const T& key) {
    int low = 0, high = index.size() - 1;
    while (low <= high) {
        int mid = low + (high - low)/ 2;
        if (index[mid].first == key)
            return mid; // Key found
        else if (index[mid].first < key)
            low = mid + 1;
        else
            high = mid - 1;
    }
    return -1;
}

class SecondaryIndex {
public:
    map<string, vector<string>> index; // Secondary Key -> List of Primary Keys
    void insert(const string& secondaryKey, const string& primaryKey);
    vector<string>* find(const string& secondaryKey);
};

class HealthcareManagementSystem {

    vector<pair<string, int>> doctorPrimaryIndex;
    SecondaryIndex doctorSecondaryIndex;
    vector<int> doctorAvailList;

    const string DOCTOR_FILE = "doctors.txt";
    const string DOCTOR_INDEX_FILE = "doctor.index";

public:
    void displayMenu();
    void addDoctor(const string& doctorID, const string& name, const string& address);
    void updateDoctor();
    void deleteDoctor();
    void searchDoctorByID();
    void searchDoctorByName();
    void loadIndexes();
    void saveIndexes();

private:
    string readRecordFromFile(const string& fileName, int position);
    int findAvailableSlot(vector<int>& availList, const string& fileName);
    void markDeleted(vector<int>& availList, int position, const string& fileName);
};

void SecondaryIndex::insert(const string& secondaryKey, const string& primaryKey) {
    index[secondaryKey].push_back(primaryKey);
}

vector<string>* SecondaryIndex::find(const string& secondaryKey) {
    auto it = index.find(secondaryKey);
    return it != index.end() ? &(it->second) : nullptr;
}

void HealthcareManagementSystem::displayMenu() {
    cout << "\n--- Healthcare Management System ---\n";
    cout << "1. Add New Doctor\n";
    cout << "2. Update Doctor Name\n";
    cout << "3. Delete Doctor\n";
    cout << "4. Search Doctor by ID\n";
    cout << "5. Search Doctor by Name\n";
    cout << "6. Exit\n";
    cout << "Enter your choice: ";
}

string HealthcareManagementSystem::readRecordFromFile(const string& fileName, int position) {
    fstream file(fileName, ios::in);
    file.seekg(position, ios::beg);
    string record;
    getline(file, record);
    file.close();
    return record;
}

int HealthcareManagementSystem::findAvailableSlot(vector<int>& availList, const string& fileName) {
    if (!availList.empty()) {
        int position = availList.back();
        availList.pop_back();
        return position;
    }
    return -1; // No available slot
}

void HealthcareManagementSystem::markDeleted(vector<int>& availList, int position, const string& fileName) {
    availList.push_back(position);
    fstream file(fileName, ios::in | ios::out);
    file.seekp(position-1, ios::beg);
    file.put('*'); // Mark the record as deleted
    file.close();
}
void HealthcareManagementSystem::addDoctor(const string& doctorID, const string& name, const string& address) {
    // Check if the doctor already exists using binary search on primary index
    if (binarySearch(doctorPrimaryIndex, doctorID) != -1) {
        cout << "Doctor with this ID already exists.\n";
        return;
    }

    // Prepare the new record
    string fullRecord = doctorID + "|" + name + "|" + address;
    int recordLength = fullRecord.length();

    // Check if there's an available slot in the doctorAvailList
    int position = -1;

    // Loop through the availList to find a suitable slot
    while (!doctorAvailList.empty()) {
        int availablePosition = doctorAvailList.back();
        doctorAvailList.pop_back();

        // Read the existing record length from the file to compare
        string existingRecord = readRecordFromFile(DOCTOR_FILE, availablePosition);
        int existingRecordLength;
        if (existingRecord[0] == '*') {
           existingRecordLength = stoi(existingRecord.substr(1, 4));
            existingRecord = existingRecord.substr(1, existingRecordLength);
        }
        else {
            existingRecordLength = stoi(existingRecord.substr(0, 4));
        } // Assuming first 4 characters are the record length

        // If the deleted record is large enough to fit the new record, use this slot
        if (existingRecordLength >= recordLength) {
            position = availablePosition;
            break;
        }
    }
    // If no suitable slot is found, append the new record at the end of the file
    if (position == -1) {
        fstream doctorFile(DOCTOR_FILE, ios::in | ios::out| ios::app);
        if (!doctorFile) {
            cerr << "Error opening doctors.txt!" << endl;
            return;
        }

        doctorFile.seekp(0, ios::end);  // Move to the end of the file
        position = doctorFile.tellp();  // Get the position to write
        doctorFile.close();

    }


    // Write the new record with the fixed-length header
    stringstream ss;
    ss << setw(4) << setfill('0') << recordLength;
    string fixedLength = ss.str();

    fstream doctorFile(DOCTOR_FILE, ios::in | ios::out);
    doctorFile.seekp(position, ios::beg);

    if (readRecordFromFile(DOCTOR_FILE, position)[0] == '*') {
        doctorFile.seekp(position, ios::beg);
        doctorFile.put(' '); // Clear the '*' to indicate it is no longer deleted
    }

    doctorFile << fixedLength << fullRecord << "\n";
    doctorFile.close();

    // Update the primary index and secondary index
    doctorPrimaryIndex.push_back({doctorID, position});

    // Sort the primary index in memory after adding a new doctor
    sort(doctorPrimaryIndex.begin(), doctorPrimaryIndex.end());

    // Save the sorted index to the file
    saveIndexes();

    cout << "Doctor added successfully.\n";
}


void HealthcareManagementSystem::deleteDoctor() {
    string doctorID;
    cout << "Enter Doctor ID to delete: ";
    cin >> doctorID;

    int pos = binarySearch(doctorPrimaryIndex, doctorID);
    if (pos == -1) {
        cout << "Doctor not found.\n";
        return;
    }

    markDeleted(doctorAvailList, doctorPrimaryIndex[pos].second, DOCTOR_FILE);
    doctorPrimaryIndex.erase(doctorPrimaryIndex.begin() + pos);
    saveIndexes();

    cout << "Doctor deleted successfully.\n";
}
void HealthcareManagementSystem::searchDoctorByID() {
    string doctorID;
    cout << "Enter Doctor ID to search: ";
    cin >> doctorID;

    // Search doctorID in the primary index
    int pos = binarySearch(doctorPrimaryIndex, doctorID);
    if (pos == -1) {
        cout << "Doctor not found.\n";
        return;
    }

    // Read the doctor record from the file
    string record = readRecordFromFile(DOCTOR_FILE, doctorPrimaryIndex[pos].second);
    cout << "Doctor Record: " << record << endl;

    // Extract Doctor ID, Name, and Address in a single pass
    size_t d1 = record.find('|');  // Find the first delimiter (after the doctor ID)
    string id = record.substr(4, d1 - 4);  // Doctor ID is after the length field (first 6 chars)

    size_t d2 = record.find('|', d1 + 1);  // Find the second delimiter (after the doctor ID)
    string name = record.substr(d1 + 1, d2 - d1 - 1);  // Extract doctor name between the first and second '|'

    // Extract the address (rest of the string after the second '|')
    string address = record.substr(d2 + 1);

    // Output the extracted information in a user-friendly way
    cout << "\n--- Doctor Details ---\n";
    cout << "Doctor ID: " << id << "\n";
    cout << "Name: " << name << "\n";
    cout << "Address: " << address << "\n";
    cout << "-----------------------\n";
}



void HealthcareManagementSystem::searchDoctorByName() {
    string name;
    cout << "Enter Doctor Name to search: ";
    cin.ignore();
    getline(cin, name);

    vector<string>* doctorIDs = doctorSecondaryIndex.find(name);
    if (!doctorIDs) {
        cout << "No doctors found with the name: " << name << endl;
        return;
    }

    cout << "Doctors found:\n";
    for (const string& doctorID : *doctorIDs) {
        int pos = binarySearch(doctorPrimaryIndex, doctorID);
        if (pos != -1) {
            string record = readRecordFromFile(DOCTOR_FILE, doctorPrimaryIndex[pos].second);

            size_t d1 = record.find('|');  // Find first delimiter (ID)
            size_t d2 = record.find('|', d1 + 1);  // Find second delimiter (Name)
            size_t d3 = record.find('|', d2 + 1);  // Find third delimiter (Address)

            string id = record.substr(0, d1);  // Doctor ID
            string name = record.substr(d1 + 1, d2 - d1 - 1);  // Doctor Name
            string address = record.substr(d2 + 1, d3 - d2 - 1);  // Doctor Address

            cout << "\n--- Doctor Details ---\n";
            cout << "Doctor ID: " << id << "\n";
            cout << "Name: " << name << "\n";
            cout << "Address: " << address << "\n";
            cout << "-----------------------\n";
        }
    }
}



void HealthcareManagementSystem::loadIndexes() {
    // Load doctor primary index from the file
    fstream doctorIndexFile(DOCTOR_INDEX_FILE, ios::in);
    if (doctorIndexFile.is_open()) {
        string record;
        int position = 0;
        while (getline(doctorIndexFile, record)) {
            stringstream ss(record);
            string doctorID;
            int doctorPosition;

            getline(ss, doctorID, '|');
            ss >> doctorPosition;

            doctorPrimaryIndex.push_back({doctorID, doctorPosition});
            position = doctorIndexFile.tellg();
        }
        doctorIndexFile.close();
    }

    // Sort the primary index in memory
    sort(doctorPrimaryIndex.begin(), doctorPrimaryIndex.end());
}

void HealthcareManagementSystem::saveIndexes() {
    fstream doctorIndexFile(DOCTOR_INDEX_FILE, ios::out | ios::trunc);  // Use trunc to overwrite the file
    if (doctorIndexFile.is_open()) {
        for (const auto& entry : doctorPrimaryIndex) {
            doctorIndexFile << entry.first << "|" << entry.second << "\n";
        }
        doctorIndexFile.close();
    }
}
void HealthcareManagementSystem::updateDoctor() {
    string doctorID;
    cout << "Enter Doctor ID to update: ";
    cin >> doctorID;

    // Search doctorID in the primary index
    int pos = binarySearch(doctorPrimaryIndex, doctorID);
    if (pos == -1) {
        cout << "Doctor not found.\n";
        return;
    }

    // Read the doctor record from the file
    string record = readRecordFromFile(DOCTOR_FILE, doctorPrimaryIndex[pos].second);
    if (record.empty()) {
        cout << "Error reading the doctor record.\n";
        return;
    }

    // Extract the current record length
    int recordLength = stoi(record.substr(0, 4));  // The first 4 characters represent the record length

    // Extract the doctor's current name and address
    size_t d1 = record.find('|');  // Find the first delimiter (after the doctor ID)
    size_t d2 = record.find('|', d1 + 1);  // Find the second delimiter (after the doctor name)
    string name = record.substr(d1 + 1, d2 - d1 - 1);  // Doctor Name
    string address = record.substr(d2 + 1);  // Doctor Address

    // Prompt the user for new details
    string newName, newAddress;
    cout << "Enter new name (leave blank to keep current): ";
    cin.ignore();
    getline(cin, newName);
    cout << "Enter new address (leave blank to keep current): ";
    getline(cin, newAddress);

    // If the user didn't provide a new name, keep the old one
    if (newName.empty()) {
        newName = name;
    }
    // If the user didn't provide a new address, keep the old one
    if (newAddress.empty()) {
        newAddress = address;
    }

    // Prepare the new record with the new details
    string newRecord = doctorID + "|" + newName + "|" + newAddress;
    int newRecordLength = newRecord.length();

    // Check if the new record length matches the old one
    if (newRecordLength != recordLength) {
        cout << "Error: New record length should be " << recordLength << " characters.\n";
        return;
    }

    // Write the updated record to the file
    fstream doctorFile(DOCTOR_FILE, ios::in | ios::out);
    doctorFile.seekp(doctorPrimaryIndex[pos].second, ios::beg);

    // Clear the '*' if present (for deleted records)
    if (readRecordFromFile(DOCTOR_FILE, doctorPrimaryIndex[pos].second)[0] == '*') {
        doctorFile.seekp(doctorPrimaryIndex[pos].second, ios::beg);
        doctorFile.put(' '); // Clear the '*' to indicate it is no longer deleted
    }

    // Write the fixed-length header and the new record
    stringstream ss;
    ss << setw(4) << setfill('0') << newRecordLength;
    string fixedLength = ss.str();

    doctorFile << fixedLength << newRecord << "\n";
    doctorFile.close();

    // Update the primary index with the new doctor record
    doctorPrimaryIndex[pos].first = doctorID;

    // Save the updated indexes to file
    saveIndexes();

    cout << "Doctor record updated successfully.\n";
}


int main() {
    HealthcareManagementSystem system;

    // Load indexes and initialize availability lists
    system.loadIndexes();

    int choice;
    do {
        system.displayMenu();  // Display the menu
        cin >> choice;

        switch (choice) {
            case 1: {
                string doctorID, newName, newAddress;
                cout << "Enter Doctor ID: ";
                cin >> doctorID;
                cout << "Enter Doctor name: ";
                cin.ignore();
                getline(cin, newName);
                cout << "Enter Doctor address: ";
                getline(cin, newAddress);
                system.addDoctor(doctorID, newName, newAddress);
                break;
            }
            case 2: {
                system.updateDoctor();
                break;
            }
            case 3: {
                system.deleteDoctor();
                break;
            }
            case 4: {
                system.searchDoctorByID();
                break;
            }
            case 5: {
                system.searchDoctorByName();
                break;
            }
            case 6: {
                cout << "Exiting...\n";
                break;
            }
            default: {
                cout << "Invalid choice. Please try again.\n";
                break;
            }
        }
    } while (choice != 10);
    system.saveIndexes();
    return 0;
}

