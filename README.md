# sonos-denon-controller

Code to allow a [Particle Argon](https://docs.particle.io/argon/) to monitor the playing state of a Sonos device (in my case a Sonos Port) and automatically switch on a Denon AVR-2300W receiver through its web interface. 

## Getting Started

These instructions will explain how to tailor the code for your environment and deploy to the Particle device.

### Prerequisites

1. Particle Argon
2. Sonos device. This should work against any device however it was tested against the Sonos Port
3. A Denon AV recevier with a web interface (again tested against the AVR-2300W)

### Installing

1. Find the IP address of your Sonos device (easiest method is to use the mobile app. Settings -> System ->About My System, then scroll to find the relevant device).

2. Find the IP address of your Denon receiver.

Nb. I would recommend fixing these ip addresses using your router to ensure they remain static.

3. Open the Web IDE for your Particle device [build.particle.io](https://build.particle.io/build) and create a new application. 

4. Copy the code from the attached .ino file, modify the two lines relating to the IP address of your devices.

5. Within the Web IDE Add the library (HttpClient).

6. Save and flash to your device.

## Versioning

We use [SemVer](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](https://github.com/your/project/tags). 

## Authors

* **Waldemar Sakalus** - *Initial code and inspiration* - [Sonos, Alexa, Denon Ultimate Integration](https://www.hackster.io/saka/sonos-alexa-denon-ultimate-integration-256740)

See also the list of [contributors](https://github.com/your/project/contributors) who participated in this project.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
