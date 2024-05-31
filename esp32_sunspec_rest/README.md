# SunSpec RESTFull API

[SUNSPEC DEVICE WEB SERVICE API SPECIFICATION](https://sunspec.org/sunspec-device-web-service-api-specification/) is a specification that describes a Representational State Transfer (RESTful) interface, compatible with HTTPS, to read and write device data that conforms to the [SunSpec Device Information Models](https://github.com/sunspec/models). It enables software developers to implement both the server (SunSpec device) and client (application) parts of a web service to communicate using the JSON-based Application Programming Interface (API) defined in this specification. This interface is fully interoperable with with SunSpec Modbus. 

This project contains an implementation of the SunSpec specification for an ESP32. It is an initial version, with the TmpAmb and Pres points from the 307 model (Base Meteorological Model). The temperature and atmospheric pressure are read from a BMP280 sensor connected to the I2C interface.


To test, load the project in 'esp32_sunspec_rest' onto an ESP32, connect to the “SunSpec Server” Wi-Fi network and Go to https://sunspec.local/v1/models for all models or https://sunspec.local/v1/models/307/instances/0?points=TmpAmb,Pres to read only temperature and pressure.

The example https certificate was generated locally, so you have to 'accept the risks and continue'

The user and password are 'admin':'admin'