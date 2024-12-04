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
        int mid = low + (high - low) / 2;
        if (index[mid].first == key)
            return mid; 
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

     vector<pair<string, int>> appointmentPrimaryIndex;
    vector<int> appointmentAvailList;

    const string DOCTOR_FILE = "doctors.txt";
    const string DOCTOR_INDEX_FILE = "doctor.index";

    const string APPOINTMENT_FILE = "appointments.txt";

public:
    void displayMenu();
    void addDoctor(const string& doctorID, const string& name, const string& address);
    void updateDoctor();
    void deleteDoctor();
    void searchDoctorByID();
    void searchDoctorByName();
    void loadIndexes();
    void saveIndexes();
    void addAppointment(const string& appointmentID, const string& date, const string& doctorID);
    void deleteAppointment();
    void searchAppointmentByID();
    void updateAppointment();

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
    cout << "6. Add Appointment\n";
    cout << "7. Delete Appointment\n";
    cout << "8. Search Appointment by ID\n";
    cout << "9. Update Appointment\n";
    cout << "10. Exit\n";
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

    // Check if there's an available slot in the doctorAvailList
    int position = findAvailableSlot(doctorAvailList, DOCTOR_FILE);

    // If no available slot, append the doctor record at the end of the file
    if (position == -1) {
        fstream doctorFile(DOCTOR_FILE, ios::in | ios::out);
        if (!doctorFile) {
            cerr << "Error opening doctors.txt!" << endl;
            return;
        }

        doctorFile.seekp(0, ios::end);  // Move to the end of the file
        position = doctorFile.tellp();  // Get the position to write

        doctorFile.close();  // Close the file after getting the position
    }

    string fullRecord = doctorID + "|" + name + "|" + address;
    int recordLength = fullRecord.length();

    stringstream ss;
    ss << setw(4) << setfill('0') << recordLength;
    string fixedLength = ss.str();

    // Write the fixed-length followed by the actual record
    fstream doctorFile(DOCTOR_FILE, ios::in | ios::out);
    doctorFile.seekp(position, ios::beg);
    doctorFile << fixedLength << fullRecord << "\n";
    doctorFile.close();

    doctorPrimaryIndex.push_back({doctorID, position});
    sort(doctorPrimaryIndex.begin(), doctorPrimaryIndex.end());

    fstream doctorIndexFile(DOCTOR_INDEX_FILE, ios::in | ios::out | ios::app);
    if (!doctorIndexFile) {
        cerr << "Error opening doctor.index!" << endl;
        return;
    }
    doctorIndexFile << doctorID << "|" << position << "\n";
    doctorIndexFile.close();

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
void HealthcareManagementSystem::addAppointment(const string& appointmentID, const string& date, const string& doctorID) {
    // Validate Doctor ID
    if (binarySearch(doctorPrimaryIndex, doctorID) == -1) {
        cout << "Error: Doctor ID does not exist.\n";
        return;
    }

    // Check if Appointment already exists
    if (binarySearch(appointmentPrimaryIndex, appointmentID) != -1) {
        cout << "Error: Appointment ID already exists.\n";
        return;
    }

  
    int position = findAvailableSlot(appointmentAvailList, APPOINTMENT_FILE);

    if (position == -1) {
        fstream file(APPOINTMENT_FILE, ios::in | ios::out);
        file.seekp(0, ios::end);
        position = file.tellp();
        file.close();
    }

 
    string record = appointmentID + "|" + date + "|" + doctorID;
    fstream file(APPOINTMENT_FILE, ios::in | ios::out);
    file.seekp(position, ios::beg);
    file << record << "\n";
    file.close();

 
    appointmentPrimaryIndex.push_back({appointmentID, position});
    sort(appointmentPrimaryIndex.begin(), appointmentPrimaryIndex.end());

    cout << "Appointment added successfully.\n";
}


void HealthcareManagementSystem::searchAppointmentByID() {
    string appointmentID;
    cout << "Enter Appointment ID to search: ";
    cin >> appointmentID;

    int pos = binarySearch(appointmentPrimaryIndex, appointmentID);
    if (pos == -1) {
        cout << "Error: Appointment not found.\n";
        return;
    }

    string record = readRecordFromFile(APPOINTMENT_FILE, appointmentPrimaryIndex[pos].second);
    cout << "Appointment Record: " << record << "\n";
}

void HealthcareManagementSystem::deleteAppointment() {
    string appointmentID;
    cout << "Enter Appointment ID to delete: ";
    cin >> appointmentID;

    int pos = binarySearch(appointmentPrimaryIndex, appointmentID);
    if (pos == -1) {
        cout << "Error: Appointment not found.\n";
        return;
    }

    markDeleted(appointmentAvailList, appointmentPrimaryIndex[pos].second, APPOINTMENT_FILE);
    appointmentPrimaryIndex.erase(appointmentPrimaryIndex.begin() + pos);

    cout << "Appointment deleted successfully.\n";
}


void HealthcareManagementSystem::updateAppointment() {
    string appointmentID;
    cout << "Enter Appointment ID to update: ";
    cin >> appointmentID;

    // Search for the appointment in the primary index
    int pos = binarySearch(appointmentPrimaryIndex, appointmentID);
    if (pos == -1) {
        cout << "Error: Appointment not found.\n";
        return;
    }

    // Read the record from the file
    string record = readRecordFromFile(APPOINTMENT_FILE, appointmentPrimaryIndex[pos].second);

    // Parse the record fields
    size_t delim1 = record.find('|');
    size_t delim2 = record.find('|', delim1 + 1);

    string currentDate = record.substr(delim1 + 1, delim2 - delim1 - 1);
    string currentDoctorID = record.substr(delim2 + 1);

    cout << "Current Appointment Date: " << currentDate << "\n";
    cout << "Enter new Appointment Date (leave blank to keep current): ";
    string newDate;
    cin.ignore();
    getline(cin, newDate);

    cout << "Current Doctor ID: " << currentDoctorID << "\n";
    cout << "Enter new Doctor ID (leave blank to keep current): ";
    string newDoctorID;
    getline(cin, newDoctorID);

    // Validate the new Doctor ID if changed
    if (!newDoctorID.empty() && binarySearch(doctorPrimaryIndex, newDoctorID) == -1) {
        cout << "Error: New Doctor ID does not exist.\n";
        return;
    }

    // Update fields if input is provided
    if (!newDate.empty()) currentDate = newDate;
    if (!newDoctorID.empty()) currentDoctorID = newDoctorID;

    // Reconstruct the updated record
    string updatedRecord = appointmentID + "|" + currentDate + "|" + currentDoctorID;

    // Write the updated record back to the file
    fstream file(APPOINTMENT_FILE, ios::in | ios::out);
    file.seekp(appointmentPrimaryIndex[pos].second, ios::beg);
    file << updatedRecord << "\n";
    file.close();

    cout << "Appointment updated successfully.\n";
}


// void HealthcareManagementSystem::searchDoctorByID() {
//     string doctorID;
//     cout << "Enter Doctor ID to search: ";
//     cin >> doctorID;
//
//     // Search doctorID in the primary index
//     int pos = binarySearch(doctorPrimaryIndex, doctorID);
//     if (pos == -1) {
//         cout << "Doctor not found.\n";
//         return;
//     }
//
//     // Read the doctor record from the file
//     string record = readRecordFromFile(DOCTOR_FILE, doctorPrimaryIndex[pos].second);
//     cout << "Doctor Record: " << record << endl;
//
//     // Extract Doctor ID, Name, and Address in a single pass
//     size_t d1 = record.find('|');  // Find the first delimiter (after the doctor ID)
//     string id = record.substr(4, d1 - 4);  // Doctor ID is after the length field (first 6 chars)
//
//     size_t d2 = record.find('|', d1 + 1);  // Find the second delimiter (after the doctor ID)
//     string name = record.substr(d1 + 1, d2 - d1 - 1);  // Extract doctor name between the first and second '|'
//
//     // Extract the address (rest of the string after the second '|')
//     string address = record.substr(d2 + 1);
//
//     // Output the extracted information in a user-friendly way
//     cout << "\n--- Doctor Details ---\n";
//     cout << "Doctor ID: " << id << "\n";
//     cout << "Name: " << name << "\n";
//     cout << "Address: " << address << "\n";
//     cout << "-----------------------\n";
// }


//
// void HealthcareManagementSystem::searchDoctorByName() {
//     string name;
//     cout << "Enter Doctor Name to search: ";
//     cin.ignore();
//     getline(cin, name);
//
//     vector<string>* doctorIDs = doctorSecondaryIndex.find(name);
//     if (!doctorIDs) {
//         cout << "No doctors found with the name: " << name << endl;
//         return;
//     }
//
//     cout << "Doctors found:\n";
//     for (const string& doctorID : *doctorIDs) {
//         int pos = binarySearch(doctorPrimaryIndex, doctorID);
//         if (pos != -1) {
//             string record = readRecordFromFile(DOCTOR_FILE, doctorPrimaryIndex[pos].second);
//
//             size_t d1 = record.find('|');  // Find first delimiter (ID)
//             size_t d2 = record.find('|', d1 + 1);  // Find second delimiter (Name)
//             size_t d3 = record.find('|', d2 + 1);  // Find third delimiter (Address)
//
//             string id = record.substr(0, d1);  // Doctor ID
//             string name = record.substr(d1 + 1, d2 - d1 - 1);  // Doctor Name
//             string address = record.substr(d2 + 1, d3 - d2 - 1);  // Doctor Address
//
//             cout << "\n--- Doctor Details ---\n";
//             cout << "Doctor ID: " << id << "\n";
//             cout << "Name: " << name << "\n";
//             cout << "Address: " << address << "\n";
//             cout << "-----------------------\n";
//         }
//     }
// }
//
//


void HealthcareManagementSystem::loadIndexes() {
    // Load doctor primary index from the file
    fstream doctorFile(DOCTOR_FILE, ios::in);
    if (doctorFile.is_open()) {
        string line;
        int position = 0;
        while (getline(doctorFile, line)) {
            size_t delim1 = line.find('|');
            if (delim1 != string::npos) {
                string doctorID = line.substr(0, delim1);  // Extract DoctorID
                doctorPrimaryIndex.push_back({doctorID, position});  // Store position
            }
            position = doctorFile.tellg();  // Get the current file position
        }
        doctorFile.close();
        
        if (doctorPrimaryIndex.empty()) {
            cerr << "Warning: No doctor records found in " << DOCTOR_FILE << ".\n";
        }
        sort(doctorPrimaryIndex.begin(), doctorPrimaryIndex.end());
    } else {
        cerr << "Error: Could not open " << DOCTOR_FILE << " for reading.\n";
    }

    // Load appointment primary index from the file
    fstream appointmentFile(APPOINTMENT_FILE, ios::in);
    if (appointmentFile.is_open()) {
        string record;
        int position = 0;
        while (getline(appointmentFile, record)) {
            size_t delim1 = record.find('|');
            if (delim1 != string::npos) {
                string appointmentID = record.substr(0, delim1);  // Extract AppointmentID
                appointmentPrimaryIndex.push_back({appointmentID, position});  // Store position
            }
            position = appointmentFile.tellg();  // Get the current file position
        }
        appointmentFile.close();
        
        if (appointmentPrimaryIndex.empty()) {
            cerr << "Warning: No appointment records found in " << APPOINTMENT_FILE << ".\n";
        }
        sort(appointmentPrimaryIndex.begin(), appointmentPrimaryIndex.end());
    } else {
        cerr << "Error: Could not open " << APPOINTMENT_FILE << " for reading.\n";
    }
}


void HealthcareManagementSystem::saveIndexes() {
    fstream doctorIndexFile(DOCTOR_INDEX_FILE, ios::out);
    for (const auto& entry : doctorPrimaryIndex) {
        doctorIndexFile << entry.first << "|" << entry.second << "\n";
    }
    doctorIndexFile.close();

     fstream appointmentFile(APPOINTMENT_FILE, ios::out);
    for (const auto& entry : appointmentPrimaryIndex) {
        appointmentFile << entry.first << "|" << entry.second << "\n";
    }
    appointmentFile.close();
}

void HealthcareManagementSystem::updateDoctor() {
    string doctorID, newName, newAddress;
    cout << "Enter Doctor ID to update: ";
    cin >> doctorID;

    int pos = binarySearch(doctorPrimaryIndex, doctorID);
    if (pos == -1) {
        cout << "Doctor not found.\n";
        return;
    }

    cout << "Enter new name (leave blank to skip): ";
    cin.ignore();
    getline(cin, newName);
    cout << "Enter new address (leave blank to skip): ";
    getline(cin, newAddress);

    fstream file(DOCTOR_FILE, ios::in | ios::out);
    file.seekp(doctorPrimaryIndex[pos].second, ios::beg);

    string record = readRecordFromFile(DOCTOR_FILE, doctorPrimaryIndex[pos].second);
    size_t delim1 = record.find('|');
    size_t delim2 = record.find('|', delim1 + 1);

    string oldName = record.substr(delim1 + 1, delim2 - delim1 - 1);
    string address = record.substr(delim2 + 1);

    if (!newName.empty()) oldName = newName;
    if (!newAddress.empty()) address = newAddress;

    file.seekp(doctorPrimaryIndex[pos].second, ios::beg);
    file << doctorID << "|" << oldName << "|" << address << "\n";
    file.close();

    cout << "Doctor updated successfully.\n";
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
            // case 4: {
            //     system.searchDoctorByID();
            //     break;
            // }
            // case 5: {
            //     system.searchDoctorByName();
            //     break;
            // }
               
               case 6: {
                string appointmentID, date, doctorID;
                cout << "Enter Appointment ID: ";
                cin >> appointmentID;
                cout << "Enter Appointment Date: ";
                cin.ignore();
                getline(cin, date);
                cout << "Enter Doctor ID: ";
                cin >> doctorID;
                system.addAppointment(appointmentID, date, doctorID);
                break;
            }
            case 7:
                system.deleteAppointment();
                break;
            case 8:
                system.searchAppointmentByID();
                break;
            case 9:
            system.updateAppointment();
                break;
            case 10: {
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

