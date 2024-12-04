#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
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
    map<string, vector<string>> index;

    void insert(const string& secondaryKey, const string& primaryKey) {
        index[secondaryKey].push_back(primaryKey);
    }
    vector<string>* find(const string& secondaryKey) {
        auto it = index.find(secondaryKey);
        return it != index.end() ? &(it->second) : nullptr;
    }
};
class HealthcareManagementSystem {

    vector<pair<string, int>> doctorPrimaryIndex;
    vector<pair<string, int>> appointmentPrimaryIndex; 
    SecondaryIndex doctorSecondaryIndex;
    SecondaryIndex appointmentSecondaryIndex;
    vector<int> doctorAvailList;
    vector<int> appointmentAvailList;

    const string DOCTOR_FILE = "doctors.txt";
    const string DOCTOR_INDEX_FILE = "doctor.index";
    const string APPOINTMENT_FILE = "appointments.txt";
    const string APPOINTMENT_INDEX_FILE = "appointment.index";

public:
    void displayMenu();
    void addDoctor(const string& doctorID, const string& name, const string& address);
    void addAppointment(const string& appointmentID, const string& doctorID, const string& date);
    void updateDoctor();
    void updateAppointment();
    void deleteDoctor();
    void deleteAppointment();
    void searchDoctorByID();
    void searchDoctorByName();
    void searchAppointmentsByDoctorID();
    void loadIndexes();
    void saveIndexes();
    void loadAvailList(vector<int>& availList, const string& fileName);
    void saveAvailList(const vector<int>& availList, const string& fileName);



private:
    string readRecordFromFile(const string& fileName, int position);
    int findAvailableSlot(vector<int>& availList, const string& fileName);
    void markDeleted(vector<int>& availList, int position, const string& fileName);
};

void HealthcareManagementSystem::displayMenu() {
    cout << "\n--- Healthcare Management System ---\n";
    cout << "1. Add New Doctor\n";
    cout << "2. Add New Appointment\n";
    cout << "3. Update Doctor Name\n";
    cout << "4. Update Appointment Date\n";
    cout << "5. Delete Doctor\n";
    cout << "6. Delete Appointment\n";
    cout << "7. Search Doctor by ID\n";
    cout << "8. Search Doctor by Name\n";
    cout << "9. Search Appointments by Doctor ID\n";
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
    return -1; 
}

void HealthcareManagementSystem::markDeleted(vector<int>& availList, int position, const string& fileName) {
    availList.push_back(position);
    fstream file(fileName, ios::in | ios::out);
    file.seekp(position-1, ios::beg);
    file.put('*'); 
    availList.push_back(position); 
    file.close();
}

void HealthcareManagementSystem::addDoctor(const string& doctorID, const string& name, const string& address) {
    if (doctorID.length() > 15 || name.length() > 30 || address.length() > 30) {
        cout << "Error: Input exceeds the maximum allowed length.\n";
        return;
    }
    if (binarySearch(doctorPrimaryIndex, doctorID) != -1) {
        cout << "Doctor with this ID already exists.\n";
        return;
    }
    int position = findAvailableSlot(doctorAvailList, DOCTOR_FILE);
    if (position == -1) {
        fstream doctorFile(DOCTOR_FILE, ios::in | ios::out);
        if (!doctorFile) {
            cerr << "Error opening doctors.txt!" << endl;
            return;
        }
        doctorFile.seekp(0, ios::end);
        position = doctorFile.tellp();
        doctorFile.close();
    }
    string fullRecord = doctorID + "|" + name + "|" + address;
    int recordLength = fullRecord.length();
    stringstream ss;
    ss << setw(4) << setfill('0') << fullRecord.length();
    string fixedLength = ss.str();
    fstream doctorFile(DOCTOR_FILE, ios::in | ios::out);
    doctorFile.seekp(position, ios::beg);
    doctorFile << fixedLength << fullRecord << "\n";
    doctorFile.close();
    doctorPrimaryIndex.push_back({doctorID, position});
    sort(doctorPrimaryIndex.begin(), doctorPrimaryIndex.end());
    doctorSecondaryIndex.insert(name, doctorID);
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
void HealthcareManagementSystem::addAppointment(const string& appointmentID, const string& doctorID, const string& date) {
    if (appointmentID.length() > 15 || doctorID.length() > 15 || date.length() > 30) {
        cout << "Error: Input exceeds the maximum allowed length.\n";
        return;
    }
    if (binarySearch(appointmentPrimaryIndex, appointmentID) != -1) {
        cout << "Appointment with this ID already exists.\n";
        return;
    }
    int position = findAvailableSlot(appointmentAvailList, APPOINTMENT_FILE);
    if (position == -1) {
        fstream appointmentFile(APPOINTMENT_FILE, ios::in | ios::out);
        if (!appointmentFile) {
            cerr << "Error opening appointments.txt!" << endl;
            return;
        }
        appointmentFile.seekp(0, ios::end);
        position = appointmentFile.tellp();
        appointmentFile.close();
    }
    string fullRecord = appointmentID + "|" + date + "|" + doctorID;
    int recordLength = fullRecord.length();
    stringstream ss;
    ss << setw(4) << setfill('0') << recordLength; 
    string fixedLength = ss.str();
    fstream appointmentFile(APPOINTMENT_FILE, ios::in | ios::out);
    if (!appointmentFile) {
        cerr << "Error: Unable to open " << APPOINTMENT_FILE << endl;
        return;
    }
    appointmentFile.seekp(position, ios::beg);
    appointmentFile << fixedLength << fullRecord << string(1, ' '); 
    appointmentFile.close();
    auto it = lower_bound(appointmentPrimaryIndex.begin(), appointmentPrimaryIndex.end(), make_pair(appointmentID, 0));
    appointmentPrimaryIndex.insert(it, {appointmentID, position});
    appointmentSecondaryIndex.insert(doctorID, appointmentID);
    saveIndexes();
    cout << "Appointment added successfully.\n";
}



void HealthcareManagementSystem::updateAppointment() {
    string appointmentID, newDate;
    cout << "Enter Appointment ID to update: ";
    cin >> appointmentID;

    int pos = binarySearch(appointmentPrimaryIndex, appointmentID);
    if (pos == -1) {
        cout << "Appointment not found.\n";
        return;
    }
    cout << "Enter new appointment date (leave blank to skip): ";
    cin.ignore();
    getline(cin, newDate);
    string record = readRecordFromFile(APPOINTMENT_FILE, appointmentPrimaryIndex[pos].second);
    size_t delim1 = record.find('|');
    size_t delim2 = record.find('|', delim1 + 1);
    string id = record.substr(0, delim1);  
    string date = record.substr(delim1 + 1, delim2 - delim1 - 1);  
    string doctorID = record.substr(delim2 + 1); 
    if (!newDate.empty()) {
        date = newDate; 
    }
    string updatedRecord = id + "|" + date + "|" + doctorID;
    fstream file(APPOINTMENT_FILE, ios::in | ios::out);
    file.seekp(appointmentPrimaryIndex[pos].second, ios::beg);
    file << setw(4) << setfill('0') << updatedRecord.length() << updatedRecord << "\n";
    file.close();
    cout << "Appointment updated successfully.\n";
}

void HealthcareManagementSystem::deleteAppointment() {
    string appointmentID;
    cout << "Enter Appointment ID to delete: ";
    cin >> appointmentID;

    int pos = binarySearch(appointmentPrimaryIndex, appointmentID);
    if (pos == -1) {
        cout << "Appointment not found.\n";
        return;
    }
    markDeleted(appointmentAvailList, appointmentPrimaryIndex[pos].second, APPOINTMENT_FILE);
    string doctorID = readRecordFromFile(APPOINTMENT_FILE, appointmentPrimaryIndex[pos].second);
    size_t delim1 = doctorID.find('|');
    size_t delim2 = doctorID.find('|', delim1 + 1);
    doctorID = doctorID.substr(delim2 + 1);
    appointmentPrimaryIndex.erase(appointmentPrimaryIndex.begin() + pos);
    auto* appointments = appointmentSecondaryIndex.find(doctorID);
    if (appointments) {
        appointments->erase(
            remove(appointments->begin(), appointments->end(), appointmentID), appointments->end());
    }
    cout << "Appointment deleted successfully.\n";
}

void HealthcareManagementSystem::searchAppointmentsByDoctorID() {
    string doctorID;
    cout << "Enter Doctor ID to search for appointments: ";
    cin >> doctorID;
    auto* appointmentIDs = appointmentSecondaryIndex.find(doctorID);
    if (!appointmentIDs || appointmentIDs->empty()) {
        cout << "No appointments found for Doctor ID: " << doctorID << endl;
        return;
    }
    cout << "Appointments for Doctor ID: " << doctorID << "\n";
    for (const string& appointmentID : *appointmentIDs) {
        int pos = binarySearch(appointmentPrimaryIndex, appointmentID);
        if (pos != -1) {
            string record = readRecordFromFile(APPOINTMENT_FILE, appointmentPrimaryIndex[pos].second);
            size_t delim1 = record.find('|');
            size_t delim2 = record.find('|', delim1 + 1);
            string id = record.substr(0, delim1); 
            string date = record.substr(delim1 + 1, delim2 - delim1 - 1);  
            string docID = record.substr(delim2 + 1);
            cout << "\n--- Appointment Details ---\n";
            cout << "Appointment ID: " << id << "\n";
            cout << "Date: " << date << "\n";
            cout << "Doctor ID: " << docID << "\n";
            cout << "---------------------------\n";
        }
    }
}
void HealthcareManagementSystem::loadAvailList(vector<int>& availList, const string& fileName) {
    ifstream file(fileName, ios::in);
    if (!file) {
        cerr << "Error: Unable to open " << fileName << " for reading." << endl;
        return;
    }
    int pos;
    while (file >> pos) {
        availList.push_back(pos);
    }
    file.close();
}

void HealthcareManagementSystem::saveAvailList(const vector<int>& availList, const string& fileName) {
    ofstream file(fileName, ios::out | ios::trunc);
    if (!file) {
        cerr << "Error: Unable to open " << fileName << " for writing." << endl;
        return;
    }
    for (int pos : availList) {
        file << pos << "\n";
    }
    file.close();
}


void HealthcareManagementSystem::loadIndexes() {
    fstream doctorIndexFile(DOCTOR_INDEX_FILE, ios::in);
    if (doctorIndexFile.is_open()) {
        string line;
        while (getline(doctorIndexFile, line)) {
            stringstream ss(line);
            string doctorID;
            int position;
            getline(ss, doctorID, '|');
            ss >> position;
            doctorPrimaryIndex.push_back({doctorID, position});
        }
        doctorIndexFile.close();
    }
    sort(doctorPrimaryIndex.begin(), doctorPrimaryIndex.end());

    fstream appointmentIndexFile(APPOINTMENT_INDEX_FILE, ios::in);
    if (appointmentIndexFile.is_open()) {
        string line;
        while (getline(appointmentIndexFile, line)) {
            stringstream ss(line);
            string appointmentID;
            int position;
            getline(ss, appointmentID, '|');
            ss >> position;
            appointmentPrimaryIndex.push_back({appointmentID, position});
        }
        appointmentIndexFile.close();
    }
    sort(appointmentPrimaryIndex.begin(), appointmentPrimaryIndex.end());
    loadAvailList(doctorAvailList, "doctor.avail");
    loadAvailList(appointmentAvailList, "appointment.avail");
}
void HealthcareManagementSystem::saveIndexes() {
    fstream doctorIndexFile(DOCTOR_INDEX_FILE, ios::out);
    if (!doctorIndexFile) {
        cerr << "Error: Unable to open " << DOCTOR_INDEX_FILE << " for writing." << endl;
        return;
    }
    for (const auto& entry : doctorPrimaryIndex) {
        doctorIndexFile << entry.first << "|" << entry.second << "\n";
    }
    doctorIndexFile.close();

    fstream appointmentIndexFile(APPOINTMENT_INDEX_FILE, ios::out);
    if (!appointmentIndexFile) {
        cerr << "Error: Unable to open " << APPOINTMENT_INDEX_FILE << " for writing." << endl;
        return;
    }
    for (const auto& entry : appointmentPrimaryIndex) {
        appointmentIndexFile << entry.first << "|" << entry.second << "\n";
    }
    appointmentIndexFile.close();

    saveAvailList(doctorAvailList, "doctor.avail");
    saveAvailList(appointmentAvailList, "appointment.avail");
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
    system.loadIndexes();
    int choice;
    
    do {
        system.displayMenu(); 
        cin >> choice; 
        
        switch (choice) {
            case 1: {  // Add New Doctor
                string doctorID, name, address;
                cout << "Enter Doctor ID: ";
                cin >> doctorID;
                cout << "Enter Doctor Name: ";
                cin.ignore(); 
                getline(cin, name);
                cout << "Enter Doctor Address: ";
                getline(cin, address);
                system.addDoctor(doctorID, name, address);  
                break;
            }
            case 2: {  // Add New Appointment
                string appointmentID, doctorID, date;
                cout << "Enter Appointment ID: ";
                cin >> appointmentID;
                cout << "Enter Doctor ID: ";
                cin >> doctorID;
                cout << "Enter Appointment Date: ";
                cin.ignore(); 
                getline(cin, date);
                system.addAppointment(appointmentID, doctorID, date);
                break;
            }
            case 3: {  // Update Doctor Name
                system.updateDoctor(); 
                break;
            }
            case 4: {  // Update Appointment Date
                system.updateAppointment(); 
                break;
            }
            case 5: {  // Delete Doctor
                system.deleteDoctor();  
                break;
            }
            case 6: {  // Delete Appointment
                system.deleteAppointment();  
                break;
            }
            case 7: {  // Search Doctor by ID
                system.searchDoctorByID(); 
                break;
            }
            case 8: {  // Search Doctor by Name
                system.searchDoctorByName();
                break;
            }
            case 9: {  // Search Appointments by Doctor ID
                system.searchAppointmentsByDoctorID();  
                break;
            }
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
