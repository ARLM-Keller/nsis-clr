OutFile "setup.exe"

Section
  InitPluginsDir
  SetOutPath $PLUGINSDIR
  
  File "SomeAssembly.dll"
  CLR::Call /NOUNLOAD "SomeAssembly.dll" "SomeNamespace.SomeClass" "SomeMethod" 5 "mystring1" "x" 10 15.8 false
  pop $0  
  MessageBox MB_OK $0
  CLR::Destroy
  
SectionEnd

