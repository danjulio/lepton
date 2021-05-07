## tCam-Mini Python Driver
Todd LaWall (bitreaper) wrote a python driver to allow access to tCam and tCam-Mini from python programs.

### tcam.py
The ```tcam.py``` file contains an object ```TCam``` used to connect to and communicate with a camera.  It manages the socket connection and provides an API for sending commands to the camera and for receiving responses back from the camera.  It uses three queues to handle the asynchronous nature of the interface.  A command queue is used for outgoing commands to the camera.  Responses from the camera are pushed into two incoming queues.  Image responses are stored in a special image queue.  All other responses are stored in a response queue.  The API hides queue operation.  API operations are single threaded and do not return until a response is returned or a timeout expires.  All response packets are presented as a python dictionary built from the json response string.

#### Usage
Include the TCam object from ```tcam.py``` file in your program.

	from tcam import TCam

Create a reference.

	cam = TCam()
	
Open a connection to the camera.

	cam.connect(<ip_address>)

where ```<ip_address>``` is the camera's IP address as a string.  The connect method returns a response indicating of the connection was successful or not.

	{"status": "timeout"}
	{"status": "connected"}

Then call various access methods to send information to or get information from the camera.

	img = cam.get_image()

Finally, shut down the driver before finishing your program to close the socket connection and terminate an internal manager thread.

	cam.shutdown()

### Demos
The ```examples``` directory contains several example python programs using the TCam driver.

The example programs take command line arguments.  Look at the source code for example-specific arguments but all examples allow specifying the camera IP address as shown.  By default the TCam driver attempts to connect to the default tCam-as-AP address of ```192.168.4.1```.

	disp_image.py -i 10.0.1.71