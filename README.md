# Smart Emergency Response System

This is a Smart Emergency Response System that I developed as a project to manage emergency requests and assign the appropriate emergency units.

The system connects a web-based frontend with a Node.js backend and a C++ program that handles the main emergency management and routing logic.

## What the project does

The system can be used to:

* Login and register users with different roles
* Add new emergency requests
* View pending and completed emergencies
* Assign emergency units to requests
* Manage ambulances, fire trucks and police units
* Update the status of emergency units
* Find the shortest route between locations
* Display emergency locations and resource locations
* Manage different units such as hospitals, fire stations and police stations

## Technologies Used

* C++
* Node.js
* Express.js
* JavaScript
* HTML
* CSS
* REST APIs
* Dijkstra's Algorithm
* Hash tables using `unordered_map`
* File handling using text files

## Project Structure

```text
EmergencySystem/
│
├── backend/
│   ├── cpp/
│   │   ├── main.cpp
│   │   ├── graph.cpp
│   │   ├── graph.h
│   │   ├── emergency.cpp
│   │   ├── emergency.h
│   │   ├── resource.cpp
│   │   ├── resource.h
│   │   ├── dispatcher.cpp
│   │   ├── dispatcher.h
│   │   ├── user.cpp
│   │   ├── user.h
│   │   ├── file_handler.cpp
│   │   ├── file_handler.h
│   │   ├── sha1.cpp
│   │   ├── sha1.h
│   │   └── compile.bat
│   │
│   ├── data/
│   │   ├── emergencies.txt
│   │   ├── graph.txt
│   │   ├── locations.txt
│   │   └── resources.txt
│   │
│   ├── package.json
│   └── sever.js
│
└── frontend/
    ├── index.html
    ├── style.css
    └── app.js
```

## How it works

The frontend provides the interface where the user can login, register, view emergencies and manage emergency resources.

The Node.js server works as the connection between the frontend and the C++ program. It provides REST API endpoints for operations such as login, adding emergencies, processing emergencies and updating resource status.

The C++ program handles the main logic of the system.

The emergency locations are represented as nodes in a graph. The connections between locations have distances associated with them. Dijkstra's algorithm is used to find the shortest path between two locations.

The project also uses `unordered_map` for user management and file handling to store the system data.

## Emergency Types

The system currently supports emergency types such as:

* Medical
* Fire
* Crime

## Emergency Resources

The system manages different types of emergency units:

* Ambulances
* Fire Trucks
* Police Units

Each resource has information such as its ID, type, current location, status and owner.

## User Roles

The system supports different user roles:

* Dispatcher
* Hospital
* Fire Department
* Police

Different units can be associated with their respective hospitals, fire stations or police stations.

## Running the Project

### 1. Clone the repository

```bash
git clone https://github.com/YOUR_USERNAME/EmergencySystem.git
```

Go into the project folder:

```bash
cd EmergencySystem
```

### 2. Install Node.js dependencies

Go to the backend folder:

```bash
cd backend
```

Install the required packages:

```bash
npm install
```

### 3. Compile the C++ backend

Go to the C++ folder:

```bash
cd cpp
```

On Windows, run:

```text
compile.bat
```

This compiles the C++ source files and creates `emergency.exe`.

A C++ compiler such as MinGW/G++ is required.

### 4. Start the Node.js server

Go back to the backend folder:

```bash
cd ..
```

Run:

```bash
node sever.js
```

The server runs on:

```text
http://localhost:3000
```

### 5. Open the application

Open this in your browser:

```text
http://localhost:3000
```

The Node.js server serves the frontend automatically.

## Algorithms Used

### Dijkstra's Algorithm

Dijkstra's algorithm is used to find the shortest path between locations in the emergency response network.

The graph contains locations such as hospitals, fire stations, police headquarters and other city locations.

The implementation uses a priority queue to select the closest unprocessed node.

### Hash Table

The C++ backend uses `unordered_map` for storing and finding users.

This provides average `O(1)` lookup time for username searches.

## Data Storage

The project currently uses text files for storing data instead of a database.

Some of the files are:

* `graph.txt` - stores the graph connections
* `locations.txt` - stores location information
* `resources.txt` - stores emergency resource information
* `emergencies.txt` - stores emergency records

## API Examples

Some of the available API endpoints are:

```text
GET  /api/ping
POST /api/login
POST /api/register

POST /api/emergency/add
POST /api/emergency/process
GET  /api/emergencies

GET  /api/resources
POST /api/resource/status
POST /api/resource/add
```

## Future Improvements

Some things that can be added in the future are:

* Database integration
* Live GPS tracking
* Real-time emergency notifications
* Better authentication and security
* Mobile application
* Live maps and route visualization
* More advanced resource allocation
* Cloud deployment

## Author
Anam Shaikh

## Note

This project was created for learning and academic purposes. It is a prototype and should not be used as a real emergency management system.


Testing CICD
