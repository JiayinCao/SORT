case "$(uname -s)" in	

Darwin)	
rm -rf dependencies	
mkdir dependencies	
cd dependencies	
curl -o easy_profiler.zip http://45.63.123.194/sort_dependencies/mac/easy_profiler.zip	
unzip easy_profiler.zip	
rm -rf __MACOSX	
rm easy_profiler.zip	
cd ..	
;;	

Linux)	
rm -rf dependencies	
mkdir dependencies	
cd dependencies	
apt-get install unzip	
wget http://45.63.123.194/sort_dependencies/linux/easy_profiler.zip	
unzip easy_profiler.zip	
rm easy_profiler.zip	
cd ..	
dir	
;;	

CYGWIN*|MINGW32*|MSYS*)	
echo 'MS Windows'	
;;	

# Add here more strings to compare	
# See correspondence table at the bottom of this answer	

*)	
echo 'other OS'	
;;	
esac