#Installation

There are three ways to add the Windows Remote Arduino library to your solution, in order from easiest to most difficult.

1. [Install the NuGet package](#option-1-install-the-nuget-package)
2. [Manually add the Windows Remote Arduino project files to your solution](#option-2-add-the-windows-remote-arduino-projects-to-your-solution)

Of these options, installing the NuGet package is by far the easiest.

##Option 1: Install the NuGet package

NuGet is a quick and easy way to automatically install the packages and setup dependencies. Unfortunately, we do not yet have support for NuGet in Windows 10.

##Option 2: Add the Windows Remote Arduino projects to your solution

###Step 1: Create a new project!

1. *File -> New Project*

 ![New Project](https://ms-iot.github.io/content/images/remote-wiring/create_00.png)

2. Select your language of choice. Windows Remote Arduino is a WinRT component, meaning it is compatable with C++, C#, or JavaScript.

3. You'll see I have chosen C# by expanding the "Visual C#" menu. Select the "Windows" option and choose "Blank App (Windows Universal)" or "Blank App (Windows 8.1 Universal)" if you are building for Windows 8.1.

 ![Windows Universal](https://ms-iot.github.io/content/images/remote-wiring/create_01.png)


###Step 2: Add Windows Remote Arduino projects to your solution

1. Clone the [Windows Remote Arduino GitHub repository](https://github.com/ms-iot/remote-wiring/).

2. Right-click on your solution in the Solution Explorer and select *Add -> Existing Project*

 ![Add existing project](https://ms-iot.github.io/content/images/remote-wiring/project_00.png)

3. Navigate to your local copy of the repository. You'll see here that I've cloned it to **C:\git\remote-wiring**, but you can choose a different directory. Then, open the appropriate solution folder for your build environment (either Windows 10 or Windows 8.1).

 ![Open the solution directory](https://ms-iot.github.io/content/images/remote-wiring/compile_00.png)

4. Let's start with the Serial project (Microsoft.Maker.Serial). Open this directory.

 ![Serial directory](https://ms-iot.github.io/content/images/remote-wiring/project_01.png)

5. Select the *.vcxproj* file. (If you are targeting Windows 8.1, you will first have to choose between Windows and Windows Phone platform directories. You do not have to do this for Windows 10, as it is Universal to all platforms!)

 ![Select vcxproj](https://ms-iot.github.io/content/images/remote-wiring/project_02.png)

6. Right-click on "References" in your project. Select *Add Reference*

 ![Add Reference](https://ms-iot.github.io/content/images/remote-wiring/project_05.png)

7. Under the "Projects" tab, select all three of the Microsoft.Maker projects

 ![Project References](https://ms-iot.github.io/content/images/remote-wiring/project_06.png)

8. Rebuild your solution by selecting *Build -> Rebuild All*

 ![Rebuild All](https://ms-iot.github.io/content/images/remote-wiring/compile_03.png)
 
9. Verify you have added the necessary [Device Capabilities](#device-capabilities) to your project manifest!

###Step 3: Have fun!!

You can now use the three projects directly in your source code! You will notice I have constructed a BluetoothSerial object and attached it to my RemoteDevice object, so I have included the two appropriate namespaces at the top of my .cs file.

 ![Have Fun!](https://ms-iot.github.io/content/images/remote-wiring/utilize_00.png)

 
#Device Capabilities

Each Windows project will contain a manifest file that must be configured to allow certain permissions, such as Bluetooth and USB connectivity. Fortunately, it is fairly easy to configure these.

You will need to open the package.appxmanifest file of your project by right-clicking and selecting the "View Code" option. Then find the <Capabilities> tag and paste one or both of the following tag blocks as a child node.

####Note:
For **Windows 8.1**, you will need to add the following namespace to the top of the XML file, inside the `<Package>` tag.

`xmlns:m2="http://schemas.microsoft.com/appx/2013/manifest"`

##Enabling Bluetooth Capabilities
You will need to add one of the following XML blocks to your manifest file, inside the <Capabilities> tag, in order to invoke the Bluetooth/USB capabilities of a WinRT application, depending on which OS version you are targetting.

###Windows 10
```xml
<DeviceCapability Name="bluetooth.rfcomm">
  <Device Id="any">
    <Function Type="name:serialPort"/>
  </Device>
</DeviceCapability>
```

###Windows 8.1
```xml
<m2:DeviceCapability Name="bluetooth.rfcomm">
  <m2:Device Id="any">
    <m2:Function Type="name:serialPort"/>
  </m2:Device>
</m2:DeviceCapability>
```

##Enabling Network Capabilities
You will need to add one of the following XML blocks to your manifest file, inside the <Capabilities> tag, in order to invoke the network socket capabilities of a WinRT application.

###Windows 10 and Windows 8.1
```xml
<Capability Name="privateNetworkClientServer"/>
<Capability Name="internetClientServer"/>
```

##Enabling USB Capabilities
You will need to add one of the following XML blocks to your manifest file in order to invoke the USB capabilities of a WinRT application, depending on which OS version you are targetting.

###Windows 10
```xml
<DeviceCapability Name="serialcommunication">
  <Device Id="any">
    <Function Type="name:serialPort"/>
  </Device>
</DeviceCapability>
```

###Windows 8.1

Unfortunately, this library does not support USB on Windows 8.1.