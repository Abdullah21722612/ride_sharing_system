#include <bits/stdc++.h>

using namespace std;

// User class to handle authentication and user data
class User {
private:
    string username;
    string loginTime;
    bool isLoggedIn;

public:
    User() : isLoggedIn(false) {}

    bool login(const string& username) {
        // In a real app, this would check against a database
        if (username.empty()) {
            cout << "Error: Username cannot be empty." << endl;
            return false;
        }
        
        this->username = username;
        
        // Get current time
        auto now = chrono::system_clock::now();
        time_t currentTime = chrono::system_clock::to_time_t(now);
        stringstream ss;
        ss << put_time(localtime(&currentTime), "%Y-%m-%d %H:%M:%S");
        this->loginTime = ss.str();
        
        isLoggedIn = true;
        return true;
    }

    string getUsername() const { return username; }
    string getLoginTime() const { return loginTime; }
    bool isAuthenticated() const { return isLoggedIn; }
};

// Location structure with improved reliability
class Location {
public:
    int x, y;
    string name;

    Location(int x = 0, int y = 0, string name = "") 
        : x(x), y(y), name(name) {}
    
    // Validation method
    bool isValid() const {
        // Assuming our map is within certain boundaries (e.g., 0-100)
        return x >= 0 && x <= 100 && y >= 0 && y <= 100;
    }
};

// Driver class to manage driver information
class Driver {
public:
    int id;
    Location location;
    bool available;
    double rating;
    
    Driver(int id, Location loc, bool avail = true, double rating = 4.5) 
        : id(id), location(loc), available(avail), rating(rating) {}
};

// Ride class to manage ride information
class Ride {
public:
    int id;
    User user;
    int driverId;
    Location pickup;
    Location dropoff;
    string requestTime;
    string status; // "requested", "accepted", "in-progress", "completed", "cancelled"
    double fare;
    
    Ride(int id, User user, Location pickup, Location dropoff)
        : id(id), user(user), pickup(pickup), dropoff(dropoff), 
          driverId(-1), status("requested"), fare(0.0) {
        // Get current time
        auto now = chrono::system_clock::now();
        time_t currentTime = chrono::system_clock::to_time_t(now);
        stringstream ss;
        ss << put_time(localtime(&currentTime), "%Y-%m-%d %H:%M:%S");
        this->requestTime = ss.str();
    }
};

// RideManager class to handle ride-related operations
class RideManager {
private:
    vector<Driver> drivers;
    vector<Location> predefinedLocations;
    vector<Ride> rides;
    int nextRideId;
    
    // Function to calculate Manhattan distance
    int calculateDistance(const Location& a, const Location& b) const {
        return abs(a.x - b.x) + abs(a.y - b.y);
    }
    
    // Function to estimate fare based on distance
    double calculateFare(const Location& pickup, const Location& dropoff) const {
        int distance = calculateDistance(pickup, dropoff);
        double baseFare = 50.0; // Base fare in some currency
        double distanceFare = distance * 5.0; // 5 per unit distance
        return baseFare + distanceFare;
    }
    
public:
    RideManager() : nextRideId(1) {
        // Initialize predefined locations
        predefinedLocations = {
            Location(0, 0, "Changa"),
            Location(5, 5, "Gate 2"),
            Location(10, 10, "Khagan")
        };
        
        // Initialize drivers with random positions
        initializeDrivers();
    }
    
    void initializeDrivers() {
        // Create 10 drivers with different locations
        drivers = {
            Driver(1, Location(1, 1)),
            Driver(2, Location(6, 5)),
            Driver(3, Location(4, 3)),
            Driver(4, Location(2, 8)),
            Driver(5, Location(9, 7)),
            Driver(6, Location(11, 12)),
            Driver(7, Location(0, 2)),
            Driver(8, Location(8, 8)),
            Driver(9, Location(3, 4)),
            Driver(10, Location(10, 0))
        };
    }
    
    const vector<Location>& getPredefinedLocations() const {
        return predefinedLocations;
    }
    
    // Find the closest predefined location to a given point
    int findClosestPredefinedLocation(const Location& point) const {
        int closestIndex = -1;
        int minDistance = INT_MAX;

        for (int i = 0; i < predefinedLocations.size(); i++) {
            int dist = calculateDistance(point, predefinedLocations[i]);
            if (dist < minDistance) {
                minDistance = dist;
                closestIndex = i;
            }
        }
        return closestIndex;
    }
    
    // Assign closest available driver to the passenger
    int assignClosestDriver(const Location& passengerLocation) {
        int closestDriver = -1;
        int minDistance = INT_MAX;

        for (int i = 0; i < drivers.size(); i++) {
            if (drivers[i].available) {
                int dist = calculateDistance(drivers[i].location, passengerLocation);
                if (dist < minDistance) {
                    minDistance = dist;
                    closestDriver = i;
                }
            }
        }
        
        // If a driver is found, mark them as unavailable
        if (closestDriver != -1) {
            drivers[closestDriver].available = false;
        }
        
        return closestDriver;
    }
    
    // BFS to find shortest path between two locations
    int findShortestPath(const Location& start, const Location& end) const {
        if (start.x == end.x && start.y == end.y)
            return 0; // Already at destination

        queue<pair<Location, int>> q; // Queue stores location and steps taken
        q.push({start, 0});

        set<pair<int, int>> visited; // To avoid revisiting locations
        visited.insert({start.x, start.y});

        // Directions for up, down, left, right
        vector<pair<int, int>> directions = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};

        while (!q.empty()) {
            auto [current, steps] = q.front();
            q.pop();

            for (auto dir : directions) {
                int nx = current.x + dir.first;
                int ny = current.y + dir.second;
                
                // Check if we've reached the destination
                if (nx == end.x && ny == end.y)
                    return steps + 1;
                
                // Check if the new location is valid and hasn't been visited
                if (nx >= 0 && ny >= 0 && nx <= 100 && ny <= 100 && 
                    visited.find({nx, ny}) == visited.end()) {
                    q.push({{nx, ny}, steps + 1});
                    visited.insert({nx, ny});
                }
            }
        }
        return -1; // No path found
    }
    
    // Create a new ride request
    Ride requestRide(const User& user, const Location& pickup, const Location& dropoff) {
        Ride ride(nextRideId++, user, pickup, dropoff);
        
        // Calculate fare
        ride.fare = calculateFare(pickup, dropoff);
        
        // Add to rides list
        rides.push_back(ride);
        return ride;
    }
    
    // Confirm a ride by assigning a driver
    bool confirmRide(int rideId, int driverId) {
        for (auto& ride : rides) {
            if (ride.id == rideId) {
                ride.driverId = driverId;
                ride.status = "accepted";
                return true;
            }
        }
        return false;
    }
    
    // Log ride details to a file
    void logRide(const Ride& ride) {
        ofstream logFile("ride_logs.txt", ios::app);
        if (logFile.is_open()) {
            logFile << "Ride ID: " << ride.id << "\n"
                    << "User: " << ride.user.getUsername() << "\n"
                    << "Request Time: " << ride.requestTime << "\n"
                    << "Pickup: (" << ride.pickup.x << "," << ride.pickup.y << ") - " << ride.pickup.name << "\n"
                    << "Dropoff: (" << ride.dropoff.x << "," << ride.dropoff.y << ") - " << ride.dropoff.name << "\n"
                    << "Driver ID: " << ride.driverId << "\n"
                    << "Status: " << ride.status << "\n"
                    << "Fare: $" << fixed << setprecision(2) << ride.fare << "\n"
                    << "------------------------------\n";
            logFile.close();
        } else {
            cerr << "Error opening log file!" << endl;
        }
    }
};

// Main application class
class RideSharingApp {
private:
    User currentUser;
    RideManager rideManager;
    string currentDateTime;
    
public:
    RideSharingApp() {
        // Get current date and time
        updateCurrentDateTime();
    }
    
    void updateCurrentDateTime() {
        auto now = chrono::system_clock::now();
        time_t currentTime = chrono::system_clock::to_time_t(now);
        stringstream ss;
        ss << put_time(localtime(&currentTime), "%Y-%m-%d %H:%M:%S");
        currentDateTime = ss.str();
    }
    
    void setCurrentDateTime(const string& dateTime) {
        currentDateTime = dateTime;
    }
    
    bool loginUser(const string& username) {
        return currentUser.login(username);
    }
    
    void displayWelcomeMessage() {
        cout << "===============================================" << endl;
        cout << "   Welcome to the Ride-Sharing Application" << endl;
        cout << "===============================================" << endl;
        cout << "Current Date & Time: " << currentDateTime << endl;
        if (currentUser.isAuthenticated()) {
            cout << "Logged in as: " << currentUser.getUsername() << endl;
            cout << "Login time: " << currentUser.getLoginTime() << endl;
        }
        cout << "-----------------------------------------------" << endl;
    }
    
    void run() {
        displayWelcomeMessage();
        
        if (!currentUser.isAuthenticated()) {
            string username;
            cout << "Please enter your username to login: ";
            getline(cin, username);
            
            if (!loginUser(username)) {
                cout << "Login failed. Exiting application." << endl;
                return;
            }
            displayWelcomeMessage();
        }
        
        // Main application flow
        Location passengerLocation;
        cout << "\nEnter your current location (x y): ";
        if (!(cin >> passengerLocation.x >> passengerLocation.y)) {
            cout << "Invalid input. Please enter numeric coordinates." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return;
        }
        
        if (!passengerLocation.isValid()) {
            cout << "Location coordinates out of bounds. Please enter valid coordinates." << endl;
            return;
        }
        
        // Find closest predefined pickup point
        const vector<Location>& predefinedPoints = rideManager.getPredefinedLocations();
        int pickupIndex = rideManager.findClosestPredefinedLocation(passengerLocation);
        
        if (pickupIndex == -1) {
            cout << "Error: Could not find a suitable pickup point." << endl;
            return;
        }
        
        cout << "Closest pickup point: " << predefinedPoints[pickupIndex].name 
             << " at (" << predefinedPoints[pickupIndex].x << ", " 
             << predefinedPoints[pickupIndex].y << ")\n";
        
        // Get drop-off location
        cout << "Choose drop-off location:" << endl;
        for (int i = 0; i < predefinedPoints.size(); i++) {
            cout << i << ": " << predefinedPoints[i].name << " at ("
                 << predefinedPoints[i].x << ", " << predefinedPoints[i].y << ")" << endl;
        }
        
        int dropIndex;
        cout << "Enter your choice (0-" << predefinedPoints.size() - 1 << "): ";
        cin >> dropIndex;
        
        if (dropIndex < 0 || dropIndex >= predefinedPoints.size()) {
            cout << "Invalid drop-off point selected." << endl;
            return;
        }
        
        Location pickup = predefinedPoints[pickupIndex];
        pickup.name = predefinedPoints[pickupIndex].name;
        
        Location dropoff = predefinedPoints[dropIndex];
        dropoff.name = predefinedPoints[dropIndex].name;
        
        cout << "\nDrop-off point: " << dropoff.name << " at ("
             << dropoff.x << ", " << dropoff.y << ")\n";
        
        // Create ride request
        Ride ride = rideManager.requestRide(currentUser, pickup, dropoff);
        
        // Find and assign closest driver
        int driverIndex = rideManager.assignClosestDriver(passengerLocation);
        
        if (driverIndex == -1) {
            cout << "Sorry, no drivers are available at the moment." << endl;
            ride.status = "cancelled";
        } else {
            rideManager.confirmRide(ride.id, driverIndex + 1);
            
            cout << "Assigned Driver: Driver " << driverIndex + 1 << endl;
            
            // Get path information
            int distance = rideManager.findShortestPath(pickup, dropoff);
            
            cout << "Estimated travel distance: " << distance << " units" << endl;
            cout << "Estimated fare: $" << fixed << setprecision(2) << ride.fare << endl;
            
            // Confirm ride
            char confirm;
            cout << "\nConfirm ride? (y/n): ";
            cin >> confirm;
            
            if (tolower(confirm) == 'y') {
                cout << "\nRide confirmed!" << endl;
                cout << "Driver is on the way to pick you up." << endl;
                ride.status = "in-progress";
            } else {
                cout << "\nRide cancelled by user." << endl;
                ride.status = "cancelled";
            }
        }
        
        // Log ride details
        rideManager.logRide(ride);
        
        cout << "\nThank you for using our Ride-Sharing Service!" << endl;
    }
};

int main() {
    RideSharingApp app;
    
    // Set the provided date and time
    app.setCurrentDateTime("2025-03-07 09:11:37");
    
    // Login with the provided username
    app.loginUser("abdullah241-16-008");
    
    // Run the application
    app.run();
    
    return 0;
}