#Installation

There are three ways to add the Windows Remote Arduino library to your solution, in order from easiest to most difficult.

1. [Install the NuGet package](#option-1-install-the-nuget-package)
2. [Manually add the Windows Remote Arduino project files to your solution](#option-2-add-the-windows-remote-arduino-projects-to-your-solution)
3. [Manually compile the Windows Remote Arduino solution and reference the WinMD files in your solution.](#option-3-manually-compile-and-add-references-to-your-solution)

Of these options, installing the NuGet package is by far the easiest.

##Option 1: Install the NuGet package

NuGet is a quick and easy way to automatically install the packages and setup dependencies. Unfortunately, we do not yet have support for NuGet in Windows 10.

##Option 2: Add the Windows Remote Arduino projects to your solution

###Step 1: Create a new project!

1. *File -> New Project*

 ![New Project](https://github.com/ms-iot/content/images/remote-wiring/create_00.png)

2. Select your language of choice. Windows Remote Arduino is a WinRT component, meaning it is compatable with C++, C#, or JavaScript.

3. You'll see I have chosen C# by expanding the "Visual C#" menu. Select the "Windows" option and choose "Blank App (Windows Universal)" or "Blank App (Windows 8.1 Universal)" if you are building for Windows 8.1.

 ![Windows Universal](https://github.com/ms-iot/content/images/remote-wiring/create_01.png)


###Step 2: Add Windows Remote Arduino projects to your solution

1. Clone the [Windows Remote Arduino GitHub repository](https://github.com/ms-iot/remote-wiring/).

2. Right-click on your solution in the Solution Explorer and select *Add -> Existing Project*

 ![Add existing project](https://github.com/ms-iot/content/images/remote-wiring/project_00.png)

3. Navigate to your local copy of the repository. You'll see here that I've cloned it to **C:\git\remote-wiring**, but you can choose a different directory. Then, open the appropriate solution folder for your build environment (either Windows 10 or Windows 8.1).

 ![Open the solution directory](https://github.com/ms-iot/content/images/remote-wiring/compile_00.png)

4. Let's start with the Serial project (Microsoft.Maker.Serial). Open this directory.

 ![Serial directory](https://github.com/ms-iot/content/images/remote-wiring/project_01.png)

5. Select the *.vcxproj* file. (If you are targeting Windows 8.1, you will first have to choose between Windows and Windows Phone platform directories. You do not have to do this for Windows 10, as it is Universal to all platforms!)

 ![Select vcxproj](https://github.com/ms-iot/content/images/remote-wiring/project_02.png)

6. Right-click on any project and select *Build Dependencies -> Project Dependencies*

 ![Project Dependencies](https://github.com/ms-iot/content/images/remote-wiring/project_03.png)

7. Project dependencies for Windows Remote Arduino should already be configured. However, it will not hurt to verify that the correct dependencies are selected for each project. Select each project in the drop down and verify it has the correct dependencies.

 * Serial has no dependencies.
 * Firmata depends on Serial.
 * RemoteWiring depends on Serial and Firmata.

 Last, select your project in the drop down, and select each of the Microsoft.Maker projects as dependencies to your project.

 ![Dependencies](https://github.com/ms-iot/content/images/remote-wiring/project_04.png)

8. Right-click on "References" in your project. Select *Add Reference*

 ![Add Reference](https://github.com/ms-iot/content/images/remote-wiring/project_05.png)
 
9. Under the "Windows Universal" tab, choose the "Extensions" sub-menu and select the *Microsoft Visual C++ AppLocal Runtime Package* version 14.0.

 ![AppLocal package](https://github.com/ms-iot/content/images/remote-wiring/applocal.png)

10. Under the "Projects" tab, select all three of the Microsoft.Maker projects

 ![Project References](https://github.com/ms-iot/content/images/remote-wiring/project_06.png)

11. Rebuild your solution by selecting *Build -> Rebuild All*

 ![Rebuild All](https://github.com/ms-iot/content/images/remote-wiring/compile_03.png)
 
12. Verify you have added the necessary [Device Capabilities](#device-capabilities) to your project manifest!

###Step 3: Have fun!!

You can now use the three projects directly in your source code! You will notice I have constructed a BluetoothSerial object and attached it to my RemoteDevice object, so I have included the two appropriate namespaces at the top of my .cs file.

 ![Have Fun!](https://github.com/ms-iot/content/images/remote-wiring/utilize_00.png)


##Option 3: Manually Compile and Add References to Your Solution

Manually compiling a WinRT component library produces .winmd and .dll files which you can reference in your project.

###Step 1: Compiling the Windows Remote Arduino solution!

1. Clone the [Windows Remote Arduino GitHub repository](https://github.com/ms-iot/remote-wiring/).
2. Open your local copy of the repository. You'll see here that I've cloned it to **C:\git\remote-wiring**, but you can choose a different directory. Then, open the appropriate solution folder for your build environment (either Windows 10 or Windows 8.1).

 ![Open the solution directory](https://github.com/ms-iot/content/images/remote-wiring/compile_00.png)

3. Open the solution file (.sln). If you do not have "File name extensions" enabled on the "View" tab, you can look for the type "Microsoft Visual Studio Solution". (If you are targeting Windows 8.1, you will first have to choose between Windows and Windows Phone platform directories. You do not have to do this for Windows 10, as it is Universal to all platforms!)

 ![Open the solution file](https://github.com/ms-iot/content/images/remote-wiring/compile_01.png)

4. Select the correct build target. If you are planning to deploy your application to Raspberry Pi2 or Windows Phone, you'll want to select ARM. Otherwise if you are targeting a PC platform or MinnowBoardMax, select either x86 or x64 (x86 for MBM).

 ![Select build target](https://github.com/ms-iot/content/images/remote-wiring/compile_02.png)

5. *Build -> Rebuild Solution*

 ![Rebuild solution](https://github.com/ms-iot/content/images/remote-wiring/compile_03.png)

6. The solution should build in about a minute.

 ![Successful build](https://github.com/ms-iot/content/images/remote-wiring/compile_04.png)

###Step 2: Create a new project!

1. *File -> New Project*

 ![New Project](https://github.com/ms-iot/content/images/remote-wiring/create_00.png)

2. Select your language of choice. Windows Remote Arduino is a WinRT component, meaning it is compatable with C++, C#, or JavaScript.

3. You'll see I have chosen C# by expanding the "Visual C#" menu. Select the "Windows" option and choose "Blank App (Windows Universal)" or "Blank App (Windows 8.1 Universal)" if you are building for Windows 8.1.

 ![Windows Universal](https://github.com/ms-iot/content/images/remote-wiring/create_01.png)

###Step 3: Reference the WinMD files!

1. Locate the "References" item in the Solution Explorer in your new project. Right-click and select "Add Reference..."

 ![Add Reference](https://github.com/ms-iot/content/images/remote-wiring/ref_00.png)
 
2. Under the "Windows Universal" tab, choose the "Extensions" sub-menu and select the *Microsoft Visual C++ AppLocal Runtime Package* version 14.0.

 ![AppLocal package](https://github.com/ms-iot/content/images/remote-wiring/applocal.png)

3. Select the "Browse" tab on the left, and then click the "Browse..." button at the bottom.

 ![Browse](https://github.com/ms-iot/content/images/remote-wiring/ref_01.png)

4. Locate the directory where you have stored the Windows Remote Arduino repository and open the appropriate project folder.
  * If you compiled the library as "ARM" you will use the "ARM" folder as I have below, and then "Debug" inside "ARM".
  * Similarly, x64 can be found in the "x64\Debug" folder.
  * x86 will be in simply "Debug" as you can see in the screenshot below.

 ![Open debug folder](https://github.com/ms-iot/content/images/remote-wiring/ref_02.png)

5. Notice that I am inside the "ARM\Debug" directory, as I am compiling for ARM platforms like Windows Phone 10. We will start by opening the "Microsoft.Maker.Serial" folder.

 ![Open the Serial folder](https://github.com/ms-iot/content/images/remote-wiring/ref_03.png)

6. Select the WinMD file and press "Add".

 ![Add the WinMD](https://github.com/ms-iot/content/images/remote-wiring/ref_04.png)

7. Repeat steps 3 - 5 for all three WinMD files located in their respective folders. *You may find additional WinMD files in other project folders, as they reference other projects. Only reference the correct WinMD for each directory.*

 ![Repeat previous steps for all three references](https://github.com/ms-iot/content/images/remote-wiring/ref_05.png)

8. **Steps 8 - 12 will be automated in the future, but are a necessary for the time being.**

 For now, we must manually 'connect' the WinMD file with it's respective .dll file. Right-click on the project file (not the solution) and select "Unload Project".

 ![Unload Project](https://github.com/ms-iot/content/images/remote-wiring/ref_06.png)

9. Select the edit .csproj option for your project name.

 ![Edit csproj](https://github.com/ms-iot/content/images/remote-wiring/ref_07.png)

10. Near the bottom of this XML file, locate the Reference section and specifically the three `<Reference>` tags for the items we just added in steps 3 - 6.

 ![Locate Reference tags](https://github.com/ms-iot/content/images/remote-wiring/ref_08.png)

11. You must add two tags to each of these `<Reference>` tags below the `<HintPath>` tag.<br/>
 `<IsWinMDFile>true</IsWinMDFile>`<br/>
 `<Implementation>%PROJECT%.dll</Implementation>`<br/>
 Where *%PROJECT%* is the name of the project appropriate for that `<Reference>` tag. I have added all three here (although only one is highlighted). So you can add the text to match exactly what I have in the screenshot below. It does not matter where you have installed your projects, the `<HintPath>` tag takes care of that for us!

 ![Additional Tags](https://github.com/ms-iot/content/images/remote-wiring/ref_09.png)

12. Right-click on the project again and select "Reload Project". If you are prompted to save, select yes!

 ![Reload Project](https://github.com/ms-iot/content/images/remote-wiring/ref_10.png)
 
13. Verify you have added the necessary [Device Capabilities](#device-capabilities) to your project manifest!


###Step 4: Have fun!

You can now use the three projects directly in your source code! You will notice I have constructed a BluetoothSerial object and attached it to my RemoteDevice object, so I have included the two appropriate namespaces at the top of my .cs file.

 ![Have Fun!](https://github.com/ms-iot/content/images/remote-wiring/utilize_00.png)
 
 
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
```xml
<m2:DeviceCapability Name="serialcommunication">
  <m2:Device Id="any">
    <m2:Function Type="name:serialPort"/>
  </m2:Device>
</m2:DeviceCapability>
```