wget http://45.63.123.194/sort_dependencies/win/easy_profiler.zip -outfile ./easy_profiler.zip

Expand-Archive .\easy_profiler.zip -DestinationPath .\dependencies\

dir
md _out
cd _out
cmake ..