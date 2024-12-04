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
            return mid; 
        else if (index[mid].first < key)
            low = mid + 1;
        else
            high = mid - 1;
    }
    return -1;
}
struct Node {
    string doctorID;
    Node* next;
};
class LinkedList {
public:
    Node* head;
    LinkedList() : head(nullptr) {}

    void insert(const string& doctorID) {
        Node* newNode = new Node{doctorID, head};
        head = newNode;
    }

    bool find(const string& doctorID) {
        Node* current = head;
        while (current) {
            if (current->doctorID == doctorID)
                return true;
            current = current->next;
        }
        return false;
    }

    void remove(const string& doctorID) {
        Node* current = head;
        Node* prev = nullptr;
        while (current) {
            if (current->doctorID == doctorID) {
                if (prev)
                    prev->next = current->next;
                else
                    head = current->next;
                delete current;
                return;
            }
            prev = current;
            current = current->next;
        }
    }
};

class SecondaryIndex {
public:

    map<string, LinkedList> index;
    const string SECONDARY_INDEX_FILE = "doctor_secondary.index";  // File to store the secondary index

    void insert(const string& secondaryKey, const string& doctorID);
    bool find(const string& secondaryKey, const string& doctorID);
    void remove(const string& secondaryKey, const string& doctorID);
    void load();  // Load the index from the file
    void save();  // Save the index to the file
};

void SecondaryIndex::insert(const string& secondaryKey, const string& doctorID) {
    index[secondaryKey].insert(doctorID);
}

bool SecondaryIndex::find(const string& secondaryKey, const string& doctorID) {
    return index[secondaryKey].find(doctorID);
}

void SecondaryIndex::remove(const string& secondaryKey, const string& doctorID) {
    LinkedList& doctorList = index[secondaryKey];

    doctorList.remove(doctorID);
    if (doctorList.head == nullptr) {
        index.erase(secondaryKey);
    }
}

void SecondaryIndex::load() {
    fstream file(SECONDARY_INDEX_FILE, ios::in);
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string name;
            string doctorID;
            getline(ss, name, '|');
            while (getline(ss, doctorID, '|')) {
                insert(name, doctorID);
            }
        }
        file.close();
    }
}

void SecondaryIndex::save() {
    fstream file(SECONDARY_INDEX_FILE, ios::out | ios::trunc);
    if (file.is_open()) {
        for (const auto& entry : index) {
            file << entry.first;
            Node* current = entry.second.head;
            while (current) {
                file << "|" << current->doctorID;
                current = current->next;
            }
            file << "\n";
        }
        file.close();
    }
}
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
    int static findAvailableSlot(vector<int>& availList, const string& fileName);
    void markDeleted(vector<int>& availList, int position, const string& fileName);
    string extractField(const string& record, int fieldIndex);
};



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

    // Check if the last character is '*', indicating deletion
    if (!record.empty() && record.back() == '*') {
        record = record.substr(0, record.length() - 1);
    }
    file.close();
    return record;
}


int HealthcareManagementSystem:: findAvailableSlot(vector<int>& availList, const string & fileName) {
    if (!availList.empty()) {
        int position = availList.back();
        availList.pop_back();
        return position;
    }
    return -1;
}
void HealthcareManagementSystem::markDeleted(vector<int>& availList, int position, const string& fileName) {
    availList.push_back(position);
    fstream file(fileName, ios::in | ios::out);
    file.seekp(position, ios::beg);
    string record = readRecordFromFile(fileName, position);
    if (!record.empty()) {
        // Mark the last character of the record with *
        file.seekp(position + record.length() - 1, ios::beg);
        file.put('*');
    }
    file.close();
}

void HealthcareManagementSystem::addDoctor(const string& doctorID, const string& name, const string& address) {
    if (binarySearch(doctorPrimaryIndex, doctorID) != -1) {
        cout << "Doctor with this ID already exists.\n";
        return;
    }

    string fullRecord = doctorID + "|" + name + "|" + address;
    int recordLength = fullRecord.length();

    int position = -1;

    if (position == -1 && doctorPrimaryIndex.empty()) {
        position = 0;  // ensure that if empty the p=0
    }

    while (!doctorAvailList.empty()) {
        int availablePosition = doctorAvailList.back();

        string existingRecord = readRecordFromFile(DOCTOR_FILE, availablePosition);
        int existingRecordLength = stoi(existingRecord.substr(0, 4)); // Assuming first 4 characters are record length
        if (existingRecordLength >= recordLength) {
            position = availablePosition;
            cout<<"avail list"<<doctorAvailList.back()<<endl;//for debug
            doctorAvailList.pop_back();
            break;
        }
    }

    if (position == -1) {
        fstream doctorFile(DOCTOR_FILE, ios::in | ios::out | ios::app);
        if (!doctorFile) {
            cerr << "Error opening doctors.txt!" << endl;
            return;
        }
        doctorFile.seekp(0, ios::end);
        position = doctorFile.tellp();
        doctorFile.close();
    }

    stringstream ss;
    ss << setw(4) << setfill('0') << recordLength;
    string fixedLength = ss.str();

    fstream doctorFile(DOCTOR_FILE, ios::in | ios::out);
    doctorFile.seekp(position, ios::beg);

    string existingRecord = readRecordFromFile(DOCTOR_FILE, position);
    if (existingRecord.back() == '*') {
        doctorFile.seekp(position + existingRecord.length() - 1, ios::beg); // Move to the end to clear the '*'
        doctorFile.put(' ');
    }

    doctorFile << fixedLength << fullRecord << "\n";
    doctorFile.close();
    // Update the primary index
    doctorPrimaryIndex.push_back({doctorID, position});
    // Update the secondary index (doctor's name)
    doctorSecondaryIndex.insert(name, doctorID);
    sort(doctorPrimaryIndex.begin(), doctorPrimaryIndex.end());
//save indexes
    saveIndexes();
    doctorSecondaryIndex.save();

    cout << "Doctor added successfully.\n";
}


void HealthcareManagementSystem::deleteDoctor() {
    string doctorID;
    cout << "Enter Doctor ID to delete: ";
    cin >> doctorID;
    // Search doctorID in the primary index
    int pos = binarySearch(doctorPrimaryIndex, doctorID);
    if (pos == -1) {
        cout << "Doctor not found.\n";
        return;
    }
    // Find the doctor record in the primary index
    string doctorIDToDelete = doctorPrimaryIndex[pos].first;
    int recordPosition = doctorPrimaryIndex[pos].second;

    // Read the doctor record to get the doctor's name
    string record = readRecordFromFile(DOCTOR_FILE, recordPosition);
    size_t d1 = record.find('|');  // Find the first delimiter (after the doctor ID)
    size_t d2 = record.find('|', d1 + 1);  // Find the second delimiter (after the doctor name)
    string name = record.substr(d1 + 1, d2 - d1 - 1);  // Extract doctor name

    // Mark the end of the record with '*'
    markDeleted(doctorAvailList, recordPosition, DOCTOR_FILE);
    doctorPrimaryIndex.erase(doctorPrimaryIndex.begin() + pos);// Remove the doctor from the primary index
    saveIndexes();  // Save the updated primary index

    // remove the corresponding name from the secondary index
    doctorSecondaryIndex.remove(name, doctorID);
    doctorSecondaryIndex.save();

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

    string record = readRecordFromFile(DOCTOR_FILE, doctorPrimaryIndex[pos].second);// Read the doctor record from the file
    // Extract Doctor ID, Name, and Address in a single pass
    size_t d1 = record.find('|');  // Find the first delimiter (after the doctor ID)
    string id = record.substr(4, d1 - 4);  // Doctor ID is after the length field (first 6 chars)

    size_t d2 = record.find('|', d1 + 1);  // Find the second delimiter (after the doctor ID)
    string name = record.substr(d1 + 1, d2 - d1 - 1);  // Extract doctor name between the first and second '|'
    // Extract the address (rest of the string after the second '|')
    string address = record.substr(d2 + 1);

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

    Node* doctorIDs = doctorSecondaryIndex.index[name].head;
    if (!doctorIDs) {
        cout << "No doctors found with the name: " << name << endl;
        return;
    }

    while (doctorIDs) {
        int pos = binarySearch(doctorPrimaryIndex, doctorIDs->doctorID);
        if (pos != -1) {
            string record = readRecordFromFile(DOCTOR_FILE, doctorPrimaryIndex[pos].second);

            string doctorID = extractField(record.substr(4,record.length()), 0);
            string doctorName = extractField(record, 1);
            string doctorAddress = extractField(record, 2);

            cout << "\n--- Doctor Details ---\n";
            cout << "Doctor ID: " << doctorID << "\n";
            cout << "Name: " << doctorName << "\n";
            cout << "Address: " << doctorAddress << "\n";
            cout << "-----------------------\n";
        }
        doctorIDs = doctorIDs->next;
    }
}

string HealthcareManagementSystem::extractField(const string& record, int fieldIndex) {
    stringstream ss(record);
    string field;
    int currentIndex = 0;

    while (getline(ss, field, '|')) {
        if (currentIndex == fieldIndex) {
            return field;
        }
        currentIndex++;
    }
    return "";  // Return empty if the index is out of bounds
}

void HealthcareManagementSystem::loadIndexes() {
    // Load doctor primary index from the file
    fstream doctorIndexFile(DOCTOR_INDEX_FILE, ios::in);
    if (doctorIndexFile.is_open()) {
        string record;
        while (getline(doctorIndexFile, record)) {
            stringstream ss(record);
            string doctorID;
            int doctorPosition;

            getline(ss, doctorID, '|');
            ss >> doctorPosition;

            doctorPrimaryIndex.push_back({doctorID, doctorPosition});
        }
        doctorIndexFile.close();
    }

    // Load secondary index
    doctorSecondaryIndex.load();
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
    int recordLength = stoi(record.substr(0, 4));  

    
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

    // First, remove the old name from the secondary index if it exists
    doctorSecondaryIndex.remove(name, doctorID);

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
    if (readRecordFromFile(DOCTOR_FILE, doctorPrimaryIndex[pos].second).back() == '*') {
        doctorFile.seekp(doctorPrimaryIndex[pos].second + newRecord.length() - 1, ios::beg);
        doctorFile.put(' '); // Clear the '*' to indicate it is no longer deleted
    }

    // Write the fixed-length header and the new record
    stringstream ss;
    ss << setw(4) << setfill('0') << newRecordLength;
    string fixedLength = ss.str();

    doctorFile << fixedLength << newRecord;
    doctorFile.close();

    // Update the primary index with the new doctor record
    doctorPrimaryIndex[pos].first = doctorID;

    // Now, add the new name to the secondary index
    doctorSecondaryIndex.insert(newName, doctorID);

    // Save the updated indexes to file
    saveIndexes();
    doctorSecondaryIndex.save();  // Save the secondary index

    cout << "Doctor record updated successfully.\n";
}

int main() {
    HealthcareManagementSystem system;
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

