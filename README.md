# GPIB-to-SQL-DLL
This automates the data retrieval process for VISA-compliant machines and stores it in an SQLite database. While it is primarily optimized for LabVIEW, its flexible design ensures compatibility with various platforms. Since ExperiorLabs is restricted to LabVIEW, a DLL was needed to write code and automate the processes to save time. 

This supports two primary modes of data retrieval: raw data retrieval and voltage-to-pressure conversion. An integrated SQLite functionality captures readings and vital metadata (timestamps, channel details, and configuration data.) The library also contains directory management functions. It checks if the necessary directories for data storage exist and creates them if not. Users also have the flexibility to set custom paths for the SQLite database, with the system providing feedback. 


For integration into LabVIEW: The integration process is straightforward. Start by importing the library into your LabVIEW project. Once imported, you can insert the commands and functions as required by your specific application.

It's worth noting that the SQLite and VISA libraries are essential for the optimal functioning of this system. But both are compiled into the DLL! Making it hassle-free for operations without the need for external dependencies.
