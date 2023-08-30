# GPIB-to-SQL-DLL
VISA Instrument Interface Library

This library provides a robust interface for VISA-compliant instruments, streamlining the data retrieval process and ensuring its secure storage in an SQLite database. While it is primarily optimized for LabVIEW, its flexible design ensures compatibility with various platforms.

Features

The library offers seamless communication, enabling smooth interaction with VISA-compliant instruments. It supports two primary modes of data retrieval: raw data retrieval and voltage-to-pressure conversion. An integrated SQLite functionality offers an efficient storage solution, capturing readings alongside vital metadata such as timestamps, channel details, and configuration data. Moreover, the library is equipped with directory management functions, ensuring the necessary directories for data storage exist and creating them if not. Users also have the flexibility to set custom paths for the SQLite database, with the system providing valuable feedback about the operation.

Integration with LabVIEW

For those looking to harness the library's capabilities within LabVIEW, the integration process is straightforward. Begin by importing the library into your LabVIEW project. Once imported, you can insert the commands and functions as required by your specific application.

Prerequisites

It's worth noting that the SQLite and VISA libraries are essential for the optimal functioning of this system. The good news is that both have been compiled directly into the DLL, ensuring efficient and hassle-free operations without the need for external dependencies.
