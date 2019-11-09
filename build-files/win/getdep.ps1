powershell Remove-Item -path ./dependencies -recurse -ErrorAction Ignore
wget http://45.63.123.194/sort_dependencies/win/dependencies.zip -OutFile dependencies.zip
Expand-Archive .\dependencies.zip -DestinationPath .\
rm .\dependencies.zip