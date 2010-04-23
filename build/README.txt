README for eKalappai Build.

1. Build prerequisite tools.
	
	* VC 2008 Express Edition. 
	  Install and ensure that "VCexpress.exe" is in system path. Default location of 
	  VCexpress is "C:\Program Files\Microsoft Visual Studio 9.0\Common7\IDE" (depends on install path).


	* QT SDK.
	  Install and ensure that "qmake.exe" is in system path. Default location of "qmake.exe" is
	  "C:\Qt\2010.02.1\qt\bin".
	  Ensure that "mingw32-make.exe" is in system path. Default location of "mingw32-make.exe" is
	  "C:\Qt\2010.02.1\mingw\bin".

	* NSIS.
	  Install and ensure that "makensis.exe" is in system path. Default location of "makensis.exe"
	  is "C:\Program Files\NSIS".

	* ANT 1.8.0 and JAVA 6 SDK. 
	  Install and make sure "ant" and "java" is in system path.


2. Go to build directory and edit "build.xml" for the source.root location value.
   Edit the line for the value to your source checkedout location. Trunk is the top level directory of
   source code.
   <property name="source.root" value="E:\ekalapaai\trunk"/>


3. From "build" directory, issue command as below.
	
   Command below will build the source code and create installer executable under ./installer directory.
   > ant 

   Command below will build source code only.
   > ant build

   Command below will clean all the generated artifacts including installer.
   > ant clean
