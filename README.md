# GPIB-to-SQL-DLL
This automates the data retrieval process for VISA-compliant machines and stores it in an SQLite database. It's mainly for LabVIEW but the DLL should work for anything since VISA and SQL libraries are compiled into it. Since ExperiorLabs is restricted to LabVIEW, a DLL was needed to write code and automate the processes to save time. 

Data retrieval: raw data retrieval and voltage-to-pressure conversion. An SQLite functionality captures readings and metadata (timestamps, channel details, and configuration data.) The library also contains directory management functions. It checks if the necessary directories for data storage exist and creates them if not. Users also have the flexibility to set custom paths for the SQLite database, with the system providing feedback. 

Integration into LabVIEW: Start by importing the DLL into your LabVIEW project. Once imported, you can insert the commands and functions as required by your specific application. The DLL should not need to be tweaked since it automatically knows (based on the VISA machine's output) what type of data it's receiving. 

It's worth noting that the SQLite and VISA libraries are needed for the functioning of this system. But both are compiled into the DLL! So no hassle necessary.
